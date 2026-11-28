//	
//	SPDX-FileCopyrightText: 2026 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	SPDX-License-Identifier: GPL-3.0-or-later
//	


//	using	config/sdl3.json


#define GLAD_GL_IMPLEMENTATION


#include <sak/fso/text_file.hpp>
#include <sak/opengl/program.hpp>
#include <sak/opengl/vertex_array.hpp>
#include <sak/ranges/fold_left_first.hpp>
#include <sak/ranges/views/enumerate.hpp>
#include <sak/sdl3/application.hpp>
#include <sak/sdl3/opengl/context.hpp>
#include <game/fps.hpp>


namespace demo {

	__using( ::std::
		,apply
		,array
		,bind_back
		,convertible_to
		,enable_shared_from_this
		,map
		,pair
		,remove_cvref_t
		,size_t
		,string
		,string_view
		,to_string
		,tuple
	)
	__using( ::sak::, ensure )
	__using( ::sak::fso::, text_file )
	__using( ::sak::math::, absolute, arccosine, clamp, cross, dot, min, minus, multiplies, normalize, plus, rotate )
	__using( ::sak::opengl::, program, shader, vertex_array )
	__using( ::sak::ranges::, contains, fold_left_first, index_of, lazy_transform, to )
	__using( ::sak::sdl3::, application, window )
	__using( ::std::ranges::, copy, count, find )
	__using( ::std::views::, concat, filter, join, join_with, single, transform, values, zip_transform )
	__using_alias( ::sak::g3f::, color, position, velocity )
	__using_static( ::sak::g3f::, zero )
	using	direction		=	::sak::g3f::point;
	using	planar_point	=	::sak::g2f::point;


	class environment
	{
	public:
		environment(
			 const GLuint binding
			,const color& background_color
			,const float ambient
			,const float volume
		)
			:m_buffer( 1, binding )
		{
			m_buffer[ 0 ] = { background_color, ambient, volume, { } };
		}

		//	half of the screen size in pixels
		auto screen_size( const planar_point& value ) noexcept -> void { m_buffer[ 0 ].m_half_screen_size = value / 2.0f; }

		auto flush( ) const noexcept -> void { m_buffer.flush( ); }

	private:
		//	scene-wide values, transferred as flat floats and assembled manually on the gpu
		struct payload
		{
			color	m_background_color;
			float	m_ambient;
			float	m_volume;
			planar_point	m_half_screen_size;
		};	//	total 8 floats

		using	environment_ssbo	=	shader::storage_buffer< payload >;
		environment_ssbo	m_buffer;
	};


	class camera
	{
	public:
		camera( const GLuint binding, const float focal, const float speed )
			:m_buffer( 1, binding )
			,m_focal( focal )
			,m_speed( speed )
		{ }

		//	world frame from the eye, the sight direction and an up hint
		auto view( const position& eye, const direction& sight, const direction& up_hint ) noexcept -> void
		{
			auto& slot = m_buffer[ 0 ];
			slot.m_position = eye;
			slot.m_forward = normalize( sight ) | to;
			slot.m_right = normalize( cross( up_hint, slot.m_forward ) ) | to;
			slot.m_up = cross( slot.m_forward, slot.m_right ) | to;
			update_pixel_vectors( );
		}

		//	world-space size of one pixel, derived from the screen and the projection
		auto viewport( const planar_point& screen_size ) noexcept -> void
		{
			pixel_scale( 2.0f / ( min( screen_size ) * m_focal ) );
			update_pixel_vectors( );
		}

		//	local movement expressed in the camera frame: right, forward and up
		auto move( const velocity& local, const float delta_seconds ) noexcept -> void
		{
			auto& slot = m_buffer[ 0 ];
			slot.m_position += *fold_left_first(
				 zip_transform(
					 multiplies
					,array{ slot.m_right, slot.m_forward, slot.m_up }
					,local
				)
				,plus
			) * ( m_speed * delta_seconds );
		}

		auto flush( ) const noexcept -> void { m_buffer.flush( ); }

	private:
		auto pixel_scale( ) const noexcept -> float { return m_pixel_scale; }
		auto pixel_scale( const float value ) noexcept -> void { m_pixel_scale = value; }

		//	world-space pixel vectors derived from the orientation basis and the pixel scale
		auto update_pixel_vectors( ) noexcept -> void
		{
			auto& slot = m_buffer[ 0 ];
			slot.m_pixel_right = slot.m_right * m_pixel_scale;
			slot.m_pixel_up = slot.m_up * m_pixel_scale;
		}

		//	orientation basis plus the world-space contribution of one pixel of screen offset
		struct payload
		{
			position	m_position;
			direction	m_forward;
			direction	m_right;
			direction	m_up;
			direction	m_pixel_right;
			direction	m_pixel_up;
		};	//	total 18 floats

		using	camera_ssbo	=	shader::storage_buffer< payload >;
		camera_ssbo	m_buffer;
		const float	m_focal;
		const float	m_speed;
		float		m_pixel_scale{ 0.0f };
	};


	inline constexpr string_view shader_directory{ "data/shader" };


	//	compile-time constraint requiring a figure trait to declare its parameter count
	template< typename t_type >
	concept is_figure_trait = requires
	{
		{ t_type::m_parameter_count } -> convertible_to< size_t >;
	};


	//	parameter budget shared by every figure payload
	struct parameter
	{
		static constexpr size_t capacity = 12uz;

		//	true when every registered trait fits the shared slot budget
		static constexpr auto fits = [ ]( const is_figure_trait auto&... descriptor )
		{
			return	( true and ... and ( descriptor.m_parameter_count <= capacity ) );
		};
	};


	//	file predicate selecting the glsl evaluator files of a figure directory
	inline constexpr auto is_glsl_file = [ ]( const auto& file_entry )
	{
		return	file_entry.is_regular_file( ) and file_entry.path( ).extension( ) == ".glsl";
	};

	//	file stem paired with its text content
	inline constexpr auto read_stem_text = [ ]( const auto& file_entry ) -> pair< string, string >
	{
		const text_file file( file_entry.path( ) );
		ensure( file.exists( ), "unable to read figure file: " + file_entry.path( ).string( ) );
		return	pair{ file.name( ), file.content( ) };
	};


	//	identifier of a figure trait read from its reflection token instead of a repeated string literal
	template< typename t_type >
	consteval auto evaluator_identifier( ) noexcept -> string_view
	{
		__using( ::std::meta::, identifier_of )
		return	identifier_of( ^^t_type );
	}


	//	close an injected region by mapping the next line back onto the template
	auto restored_diagnostics( const string& content, const size_t marker_line ) -> string
	{
		return	content + "\n#line " + to_string( marker_line + 1uz );
	}


	//	the figure concept: flat payload record, compile-time traits and the loader of its glsl evaluators
	struct figure
	{
		//	fixed-stride flat payload shared across all registered figure types
		template< size_t t_parameter_count = parameter::capacity >
		struct payload
		{
			position	m_position;
			float		m_type_id;
			color		m_color;
			array< float, t_parameter_count >	m_parameters;
		};	//	total 8 + t_parameter_count floats

		//	compile-time descriptor carrying only the parameter count, its name is its identifier
		struct trait
		{
			struct sphere { static constexpr size_t m_parameter_count = 1uz; };
			struct triangle { static constexpr size_t m_parameter_count = 12uz; };
		};

		//	every registered trait, each one named by its own identifier
		static constexpr tuple traits{ trait::sphere{ }, trait::triangle{ } };

		//	apply one per-trait mapping across the whole registered set
		template< typename t_mapping >
		static constexpr auto map_traits( const t_mapping& mapping )
		{
			return	apply( mapping, traits );
		}

		//	glsl evaluators loaded from individual files, the map key supplies the reflected name while the loader supplies the source and assigns deterministic type ids
		class loader
		{
		public:
			struct entry
			{
				GLuint	m_type_id;
				string	m_source;
			};
			using	table	=	map< string, entry >;

			explicit loader( const string& base_path = string( shader_directory ) + "/figures" )
				:m_figures( load( base_path ) )
			{ }

			auto operator[ ]( const string_view name ) const -> const entry&
			{
				const string key( name );
				ensure( contains( m_figures, key ), "unknown figure evaluator: " + key );
				return	m_figures.at( key );
			}

			auto figures( ) const noexcept -> const table& { return m_figures; }

		private:
			//	load every glsl evaluator from one directory, keyed by file stem
			//	a directory cannot hold two files with the same stem and the same extension
			static auto load( const string& base_path ) -> table
			{
				__using( ::sak::ranges::views::, enumerate )
				__using( ::std::filesystem::, directory_iterator, is_directory )
				ensure( is_directory( base_path ), "unable to open figure directory: " + base_path );

				map< string, string > text_by_name = directory_iterator( base_path )
					|	filter( is_glsl_file )
					|	transform( read_stem_text )
					|	to;

				ensure( not text_by_name.empty( ), "no figure file found in: " + base_path );

				return	text_by_name | enumerate
					|	transform( [ ]( auto&& indexed_text )
						{
							auto&& [ index, text_entry ] = indexed_text;
							return	pair{
								 text_entry.first
								,entry{ static_cast< GLuint >( index ), ::std::move( text_entry.second ) }
							};
						} )
					|	to;
			}

			table	m_figures;
		};

		//	figure block, taken from the braces that open and close its file
		static auto block( const string& source ) -> string
		{
			const size_t opening = source.find( '{' );
			const size_t closing = source.rfind( '}' );
			ensure( opening < closing and closing < source.size( ), "figure file without a block" );
			return	source.substr( opening, closing - opening + 1uz );
		}

		//	one dispatch case inlining a loaded figure block on the shared dispatcher scope
		static auto dispatch_case( const loader::table::value_type& source_entry ) -> string
		{
			return	"\t\tcase " + to_string( source_entry.second.m_type_id )
				+ "u: " + block( source_entry.second.m_source )
				+ "\n\t\tbreak;";
		}

		//	integer switch dispatching on the type id assigned by the loader
		static auto dispatch( const loader& figure_loader, const size_t marker_line ) -> string
		{
			return	restored_diagnostics(
				 "\tswitch( uint( figure_item.type_id ) )\n\t{\n"
				+ string( figure_loader.figures( )
					|	transform( dispatch_case )
					|	join_with( string_view{ "\n" } )
					|	to )
				+ "\n\t}"
				,marker_line
			);
		}

		//	assert every registered trait has a loaded evaluator
		static auto ensure_sources( const loader& figure_loader ) -> void
		{
			map_traits( [ & ]( const is_figure_trait auto&... descriptor )
			{
				for( const string_view name : array{ evaluator_identifier< remove_cvref_t< decltype( descriptor ) > >( )... } )
					ensure( contains( figure_loader.figures( ), string( name ) ), "missing figure evaluator: " + string( name ) );
			} );
		}

		//	write one payload inside its shared parameter slot budget
		template< is_figure_trait t_trait >
		static auto write(
			 payload<>& item
			,const loader& figure_loader
			,const position& world_position
			,const color& tint
			,const array< float, t_trait::m_parameter_count >& parameters
		) -> void
		{
			item.m_position = world_position;
			item.m_type_id = figure_loader[ evaluator_identifier< t_trait >( ) ].m_type_id;
			item.m_color = tint;
			item.m_parameters.fill( 0.0f );
			copy( parameters, item.m_parameters.begin( ) );
		}

		//	write one payload from a cpu shape carrying a trait
		template< typename t_shape >
			requires is_figure_trait< typename t_shape::descriptor >
		static auto write(
			 payload<>& item
			,const loader& figure_loader
			,const t_shape& shape
			,const color& tint
		) -> void
		{
			write< typename t_shape::descriptor >(
				 item
				,figure_loader
				,shape.position( )
				,tint
				,shape.parameters( )
			);
		}
	};


	//	one glsl template placeholder pairing its marker line with its replacement producer
	struct glsl_placeholder
	{
		//	position of a placeholder marker inside a glsl text while asserting the text still carries it
		static auto find( const string& source, const string_view marker ) -> size_t
		{
			const size_t position = source.find( marker );
			ensure( position < source.size( ), "missing glsl placeholder: " + string( marker ) );
			return	position;
		}

		//	replace one glsl placeholder while asserting the template still carries it
		static auto replace(
			 string& source
			,const string_view marker
			,const string_view content
		) -> void
		{
			source.replace( find( source, marker ), marker.size( ), content );
		}

		//	capacity define replacing the parameter capacity marker
		static auto define( const figure::loader&, const size_t ) -> string
		{
			return	"#define figure_parameter_capacity " + to_string( parameter::capacity );
		}

		static const array< glsl_placeholder, 2 > all;

		string_view	m_marker;
		string( *m_produce )( const figure::loader& loader, const size_t marker_line );
	};


	//	one-based line number of a placeholder marker inside its source
	auto line_number_at( const string& source, const string_view marker ) -> size_t
	{
		return	1uz + count(
			 string_view{ source.data( ), glsl_placeholder::find( source, marker ) }
			,'\n'
		);
	}


	//	shared vertex storage and centroid accessors for fixed-vertex-count shapes
	template< size_t t_vertex_count >
	class vertex_shape
	{
	public:
		using	data	=	array< ::demo::position, t_vertex_count >;

		explicit vertex_shape( const data& vertices ) noexcept
			:m_vertices( vertices )
		{ }

		auto vertices( ) const noexcept -> const data& { return m_vertices; }
		auto vertices( const data& value ) noexcept -> void { m_vertices = value; }

		auto position( ) const noexcept -> ::demo::position { return *fold_left_first( m_vertices, plus ) / t_vertex_count; }
		auto position( const ::demo::position& value ) noexcept -> void
		{
			const ::demo::position offset = value - position( );
			for( auto& vertex : m_vertices )
				vertex += offset;
		}

	protected:
		data	m_vertices;
	};


	//	cpu triangle defined by three world vertices, deriving centroid, normal and gpu parameters
	class triangle : public vertex_shape< 3 >
	{
	public:
		using	descriptor	=	::demo::figure::trait::triangle;
		using	vertex_shape< 3 >::vertex_shape;

		auto normal( ) const noexcept -> direction
		{
			return	normalize( cross(
				 m_vertices[ 1 ] - m_vertices[ 0 ]
				,m_vertices[ 2 ] - m_vertices[ 0 ]
			) ) | to;
		}
		auto normal( const direction& value ) noexcept -> void
		{
			__using( ::std::numbers::, pi_v )
			const direction current = normal( );
			const direction target = normalize( value ) | to;
			const float cosine_angle = clamp( dot( current, target ), -1.0f, 1.0f );
			if( 1.0f - cosine_angle <= 1e-6f )
				return;
			const bool is_opposite = cosine_angle <= -1.0f + 1e-6f;
			const direction axis = is_opposite ? orthogonal_axis( current ) : normalize( cross( current, target ) ) | to;
			const float angle = is_opposite ? pi_v< float > : arccosine( cosine_angle );
			const ::demo::position center = position( );
			for( auto& vertex : m_vertices )
				vertex = ( center + rotate( vertex - center, axis, angle ) ) | to;
		}

		auto parameters( ) const noexcept -> array< float, descriptor::m_parameter_count >
		{
			return	concat(
					 m_vertices | lazy_transform( bind_back( minus, position( ) ) )
					,single( normal( ) )
				) | join | to;
		}

	private:
		static auto orthogonal_axis( const direction& unit ) noexcept -> direction
		{
			const auto magnitudes = unit | absolute;
			direction candidate{ };
			candidate[ *index_of( magnitudes, min( magnitudes ) ) ] = 1.0f;
			return	normalize( cross( unit, candidate ) ) | to;
		}
	};


	//	compile-time validation of the shared payload capacity for every registered figure
	static_assert(
		 figure::map_traits( parameter::fits )
		,"figure parameter count exceeds the shared payload capacity"
	);


	//	every placeholder of the fragment template in replacement order
	inline const array< glsl_placeholder, 2 > glsl_placeholder::all{ {
		 { "//	@figure_parameter_capacity", glsl_placeholder::define }
		,{ "//	@figure_dispatch", figure::dispatch }
	} };


	//	fragment template copy with every placeholder replaced by its generated content
	auto assemble_fragment( const string& fragment_template, const figure::loader& loader ) -> string
	{
		string source = fragment_template;
		for( const auto& entry : glsl_placeholder::all )
			glsl_placeholder::replace(
				 source
				,entry.m_marker
				,entry.m_produce( loader, line_number_at( fragment_template, entry.m_marker ) )
			);
		return	source;
	}


	//	every glsl section compiled in memory with figure evaluators from the figure loader
	auto compile_shaders( const figure::loader& loader ) -> shader::table
	{
		__using( ::sak::meta::, name_of )
		figure::ensure_sources( loader );

		const text_file template_file( string( shader_directory ) + "/figures.glsl" );
		ensure( template_file.exists( ), "unable to read shader file: " + template_file.path( ).string( ) );

		const string fragment_name( name_of( shader::type::fragment ) );
		auto sections = shader::split_sections( template_file.content( ) );
		sections.at( fragment_name ) = assemble_fragment( sections.at( fragment_name ), loader );
		return	shader::compile( sections );
	}


	class demo_logic final : public window::listener, public enable_shared_from_this< demo_logic >
	{
	public:
		using	geometry	=	::sak::g2i;
		using	enum	window::flag;

		demo_logic( )
			:m_application( )
			,m_window( "glsl figures", { opengl, resizable } )
			,m_context( m_window )
			,m_environment( 0, { 0.0f, 0.0f, 0.0f, 1.0f }, 0.3f, 0.7f )
			,m_camera( 1, 2.0f, 1.5f )
			,m_instances( 2, 2 )
			,m_vertex_array( )
			,m_loader( )
			,m_shaders( compile_shaders( m_loader ) )
			,m_shader_program( m_shaders | values )
		{
			m_vertex_array.bind( );
			m_shader_program.use( );

			m_camera.view( { 0.0f, 0.0f, -2.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } );

			figure::write< figure::trait::sphere >(
				 m_instances[ 0 ]
				,m_loader
				,{ -1.00f, 0.0f, 1.0f }
				,{ 1.0f, 0.0f, 0.0f, 1.0f }
				,{ 0.40f }
			);
			triangle blue_triangle{ triangle::data{
				 position{ -0.45f, -0.40f, 0.0f }
				,position{  0.45f, -0.40f, 0.0f }
				,position{  0.00f,  0.50f, 0.0f }
			} };
			blue_triangle.position( { 0.00f, 0.0f, 1.0f } );
			blue_triangle.normal( { 0.35f, 0.25f, -1.0f } );
			figure::write( m_instances[ 1 ], m_loader, blue_triangle, { 0.0f, 0.0f, 1.0f, 1.0f } );

			m_environment.flush( );
			m_camera.flush( );
			m_instances.flush( );

			pixel_resize( m_window.pixel_size( ) );
		}

		auto run( ) -> void
		{
			__using( ::std::chrono::, duration, high_resolution_clock )
			m_window.listeners( ) += shared_from_this( );

			m_frame_limiter.compute( );
			auto last_time = high_resolution_clock::now( );

			m_application.run( [ & ]( )
			{
				const auto current_time = high_resolution_clock::now( );
				update( duration< float >( current_time - last_time ).count( ) );
				last_time = current_time;

				gl_draw_arrays( GL_TRIANGLE_STRIP, 0, 4 );

				m_window.swap( );
				m_frame_limiter.compute( );
			} );
		}

		void pixel_resize( const geometry::size& new_size ) override
		{
			const auto screen_size = planar_point{ new_size };
			m_environment.screen_size( screen_size );
			m_environment.flush( );

			m_camera.viewport( screen_size );
			m_camera.flush( );
		}

		void key_down( const SDL_KeyboardEvent& event ) override
		{
			if( event.key == SDLK_ESCAPE )
				m_application.quit( );
			else if( event.key == SDLK_F11 and not event.repeat )
				m_window.toggle_fullscreen( );
			else if( const auto found = find( m_key_movements, event.key, &key_movement::m_key ); found not_eq m_key_movements.end( ) )
				m_movement[ found->m_axis ] = found->m_value;
		}

		void key_up( const SDL_KeyboardEvent& event ) override
		{
			const auto found = find( m_key_movements, event.key, &key_movement::m_key );
			if( found not_eq m_key_movements.end( ) and m_movement[ found->m_axis ] == found->m_value )
				m_movement[ found->m_axis ] = 0.0f;
		}

		void focus_lost( ) override { m_movement = { }; }

	private:
		__using_alias( ::game::, fps )
		__using_alias( ::sak::sdl3::opengl::, context )
		using	instances_ssbo	=	shader::storage_buffer< figure::payload<> >;

		struct key_movement
		{
			SDL_Keycode	m_key;
			size_t		m_axis;
			float		m_value;
		};

		static constexpr array< key_movement, 4 > m_key_movements{ {
			 { SDLK_W, 1, 1.0f }
			,{ SDLK_S, 1, -1.0f }
			,{ SDLK_A, 0, -1.0f }
			,{ SDLK_D, 0, 1.0f }
		} };

		auto update( const float delta_seconds ) -> void
		{
			if( m_movement == zero )
				return;
			m_camera.move( m_movement, delta_seconds );
			m_camera.flush( );
		}

		application	m_application;
		window		m_window;
		context		m_context;
		environment	m_environment;
		camera		m_camera;
		instances_ssbo	m_instances;
		vertex_array	m_vertex_array;
		figure::loader	m_loader;
		shader::table	m_shaders;
		program	m_shader_program;
		fps		m_frame_limiter{ 60 };
		velocity	m_movement{ };
	};


} 


auto main( const int argument_count, const char* argument_values[ ] ) -> int
{
	__using( ::std::
		,make_shared
		,println
		,runtime_error
		,string
		,vector
	)
	__using( ::sak::, exit_success, exit_failure )
	__using( ::sak::ranges::, contains )
	__using( ::demo::, demo_logic )

	const vector< string > arguments( argument_values, argument_values + argument_count );
	if( contains( arguments, { "-h", "--help" } ) )
		return	println(
			"this executable renders figures through the agnostic pipeline;\n"
			"use w a s d to move the camera, f11 toggles fullscreen and escape quits"
		), exit_success;

	try
	{
		println( "starting agnostic pipeline example" );

		const auto demo = make_shared< demo_logic >( );
		demo->run( );

		println( "agnostic pipeline finished successfully" );
	}
	catch( const runtime_error& error )
	{
		println( "test failed: {}", error.what( ) );
		return	exit_failure;
	}

	return	exit_success;
}


