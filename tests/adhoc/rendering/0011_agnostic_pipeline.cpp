//	
//	SPDX-FileCopyrightText: 2026 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	SPDX-License-Identifier: GPL-3.0-or-later
//	


//	using	config/sdl3.json


#define GLAD_GL_IMPLEMENTATION


#include <sak/opengl/program.hpp>
#include <sak/opengl/vertex_array.hpp>
#include <sak/ranges/fold_left_first.hpp>
#include <sak/ranges/views/enumerate.hpp>
#include <sak/sdl3/application.hpp>
#include <sak/sdl3/opengl/context.hpp>
#include <game/fps.hpp>


namespace gl {

	__using( ::std::
		,apply
		,array
		,bind_back
		,convertible_to
		,enable_shared_from_this
		,map
		,remove_cvref_t
		,size_t
		,string
		,string_view
		,to_string
		,tuple
	)
	__using( ::std::chrono::, duration, high_resolution_clock )
	__using( ::std::filesystem::, directory_iterator, is_directory )
	__using( ::std::meta::, identifier_of )
	__using( ::std::numbers::, pi_v )
	__using( ::std::ranges::, copy, count, find )
	__using( ::std::views::
		,concat
		,join
		,join_with
		,single
		,transform
		,values
		,zip_transform
	)
	__using( ::sak::, ensure )
	__using( ::sak::fso::, text_file )
	__using( ::sak::math::
		,absolute
		,arccosine
		,clamp
		,cross
		,dot
		,length
		,min
		,minus
		,multiplies
		,normalize
		,plus
		,rotate
	)
	__using( ::sak::opengl::, program, shader, vertex_array )
	__using( ::sak::ranges::
		,contains
		,fold_left_first
		,index_of
		,lazy_transform
		,to
	)
	__using( ::sak::ranges::views::, enumerate )
	__using( ::sak::sdl3::, application, window )
	__using( ::sak::sdl3::opengl::, context )
	__using( ::game::, fps )
	__using_alias( ::sak::g3f::, color, position, velocity )
	__using_static( ::sak::g3f::, zero )
	using	direction		=	::sak::g3f::point;
	using	planar_point	=	::sak::g2f::point;


	//	shared environment, transferred as flat floats and assembled manually on the gpu
	struct environment
	{
		color	m_background_color;
		float	m_focal;
		float	m_ambient;
		float	m_volume;
		planar_point	m_half_screen_size;
		float	m_coordinate_scale;
	};	//	total 10 floats


	//	shared camera, transferred as flat floats and assembled manually on the gpu
	struct camera
	{
		position	m_position;
		direction	m_forward;
		direction	m_right;
		direction	m_up;
	};	//	total 12 floats


	//	fixed-stride flat figure payload shared across all registered figure types
	template< size_t t_parameter_count >
	struct figure_payload
	{
		position	m_position;
		float		m_type_id;
		color		m_color;
		array< float, t_parameter_count >	m_parameters;
	};	//	total 8 + t_parameter_count floats


	inline constexpr size_t parameter_capacity = 12uz;
	using	figure = figure_payload< parameter_capacity >;


	//	compile-time figure descriptor carrying only the parameter count, its name is its identifier
	struct figure_trait
	{
		struct sphere
		{
			static constexpr size_t	m_parameter_count = 1uz;
		};

		struct triangle
		{
			static constexpr size_t	m_parameter_count = 12uz;
		};

		struct capsule
		{
			static constexpr size_t	m_parameter_count = 5uz;
		};
	};


	//	identifier of a figure trait read from its reflection token instead of a repeated string literal
	template< typename t_trait >
	consteval auto evaluator_identifier( ) noexcept -> string_view
	{
		return	identifier_of( ^^t_trait );
	}


	//	compile-time constraint requiring a figure trait to declare its parameter count
	template< typename t_type >
	concept is_figure_trait = requires
	{
		{ t_type::m_parameter_count } -> convertible_to< size_t >;
	};


	//	shared vertex storage and centroid accessors for polygon and segment shapes
	template< size_t t_vertex_count >
	class vertex_shape
	{
	public:
		using	data	=	array< ::gl::position, t_vertex_count >;

		explicit vertex_shape( const data& vertices ) noexcept
			:m_vertices( vertices )
		{ }

		auto vertices( ) const noexcept -> const data& { return m_vertices; }
		auto vertices( const data& value ) noexcept -> void { m_vertices = value; }

		auto position( ) const noexcept -> ::gl::position { return *fold_left_first( m_vertices, plus ) / t_vertex_count; }
		auto position( const ::gl::position& value ) noexcept -> void
		{
			const ::gl::position offset = value - position( );
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
		using	figure_trait	=	::gl::figure_trait::triangle;
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
			const direction current = normal( );
			const direction target = normalize( value ) | to;
			const float cosine_angle = clamp( dot( current, target ), -1.0f, 1.0f );
			if( 1.0f - cosine_angle <= 1e-6f )
				return;
			const bool is_opposite = cosine_angle <= -1.0f + 1e-6f;
			const direction axis = is_opposite
				? orthogonal_axis( current )
				: normalize( cross( current, target ) ) | to;
			const float angle = is_opposite ? pi_v< float > : arccosine( cosine_angle );
			const ::gl::position center = position( );
			for( auto& vertex : m_vertices )
				vertex = ( center + rotate( vertex - center, axis, angle ) ) | to;
		}

		auto parameters( ) const noexcept -> array< float, figure_trait::m_parameter_count >
		{
			return	concat(
					 m_vertices | lazy_transform( bind_back( minus, position( ) ) )
					,single( normal( ) )
				)
				| join
				| to;
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


	//	cpu capsule defined by two world vertices and a radius, deriving midpoint, unit axis and gpu parameters
	class capsule : public vertex_shape< 2 >
	{
	public:
		using	figure_trait	=	::gl::figure_trait::capsule;

		explicit capsule( const data& vertices, const float radius ) noexcept
			:vertex_shape< 2 >( vertices )
			,m_radius( radius )
		{ }

		auto radius( ) const noexcept -> float { return m_radius; }
		auto radius( const float value ) noexcept -> void { m_radius = value; }

		auto parameters( ) const noexcept -> array< float, figure_trait::m_parameter_count >
		{
			const direction segment = ( m_vertices[ 1 ] - m_vertices[ 0 ] ) | to;
			const float segment_length = length( segment );
			direction axis{ 0.0f, 1.0f, 0.0f };
			if( segment_length > 1e-6f )
				axis = ( segment / segment_length ) | to;
			return	concat( axis, array{ segment_length * 0.5f, m_radius } ) | to;
		}

	private:
		float	m_radius;
	};


	//	every registered figure trait, each one named by its own identifier
	inline constexpr tuple figure_traits{ figure_trait::sphere{ }, figure_trait::triangle{ }, figure_trait::capsule{ } };


	//	apply one per-trait mapping across the whole registered figure set
	template< typename t_mapping >
	constexpr auto map_figure_traits( const t_mapping& mapping )
	{
		return	apply( mapping, figure_traits );
	}


	//	per-trait mapping that validates the shared parameter slot budget
	inline constexpr auto payload_within_capacity = [ ]( const is_figure_trait auto&... trait )
	{
		return	( true and ... and ( trait.m_parameter_count <= parameter_capacity ) );
	};


	//	compile-time validation of the shared payload capacity for every registered figure
	static_assert(
		 map_figure_traits( payload_within_capacity )
		,"figure parameter count exceeds the shared payload capacity"
	);


	//	figure glsl evaluators loaded from individual files, the map key supplies the reflected name while the loader supplies the source and assigns deterministic type ids
	class figure_loader
	{
	public:
		struct entry
		{
			GLuint	m_type_id;
			string	m_source;
		};
		using	table	=	map< string, entry >;

		explicit figure_loader( const string& base_path = "data/shader/figures" )
			:m_base_path( base_path )
		{
			ensure( is_directory( m_base_path ), "unable to open figure directory: " + m_base_path );

			map< string, string > sources;
			for( const auto& item : directory_iterator( m_base_path ) )
			{
				if( not item.is_regular_file( ) or item.path( ).extension( ) not_eq ".glsl" )
					continue;
				const text_file file( item.path( ) );
				ensure( file.exists( ), "unable to read figure file: " + item.path( ).string( ) );
				ensure( not contains( sources, file.name( ) ), "duplicate figure name: " + file.name( ) );
				sources.emplace( file.name( ), file.content( ) );
			}

			ensure( not sources.empty( ), "no figure file found in: " + m_base_path );

			for( auto&& [ index, item ] : sources | enumerate )
				m_figures.emplace( item.first, entry{ static_cast< GLuint >( index ), ::std::move( item.second ) } );
		}

		auto operator[ ]( const string_view name ) const -> const entry&
		{
			const string key( name );
			ensure( contains( m_figures, key ), "unknown figure evaluator: " + key );
			return	m_figures.at( key );
		}

		auto figures( ) const noexcept -> const table& { return m_figures; }

	private:
		string	m_base_path;
		table	m_figures;
	};


	//	one dispatch case generated per loaded figure evaluator
	auto figure_case( const string_view name, const figure_loader::entry& figure_entry ) -> string
	{
		return	"\t\tcase " + to_string( figure_entry.m_type_id )
			+ "u: return evaluate_" + string( name )
			+ "( ray_origin, unit_direction, figure_item );";
	}


	//	marker lines delimiting the two shader bodies of the template
	inline constexpr string_view vertex_shader_marker = "//	vertex-shader";
	inline constexpr string_view fragment_shader_marker = "//	fragment-shader";


	//	position of a placeholder marker inside a glsl text while asserting the text still carries it
	auto find_placeholder(
		 const string& source
		,const string_view marker
		,const string_view label = "missing glsl placeholder: "
	) -> size_t
	{
		const size_t position = source.find( marker );
		ensure( position < source.size( ), string( label ) + string( marker ) );
		return	position;
	}


	//	replace one glsl placeholder while asserting the template still carries it
	auto replace_placeholder(
		 string& source
		,const string_view marker
		,const string_view content
	) -> void
	{
		source.replace( find_placeholder( source, marker ), marker.size( ), content );
	}


	//	body between the first and the second occurrence of a section marker line
	auto section( const string& source, const string_view marker ) -> string
	{
		const size_t marker_end = source.find( '\n', find_placeholder( source, marker, "missing glsl section marker: " ) );
		ensure(
			 marker_end < source.size( )
			,"unterminated glsl section marker line: " + string( marker )
		);
		const size_t second = source.find( marker, marker_end );
		ensure(
			 second < source.size( )
			,"unterminated glsl section marker: " + string( marker )
		);
		return	source.substr( marker_end + 1uz, second - marker_end - 1uz );
	}


	//	one-based line number of a placeholder marker inside its source
	auto line_number_at( const string& source, const string_view marker ) -> size_t
	{
		return	1uz + count(
			 string_view{ source.data( ), find_placeholder( source, marker ) }
			,'\n'
		);
	}


	//	close an injected region by mapping the next line back onto the template
	auto restored_diagnostics( const string& content, const size_t marker_line ) -> string
	{
		return	content + "\n#line " + to_string( marker_line + 1uz );
	}


	//	capacity define replacing the parameter capacity marker
	auto parameter_capacity_define( const figure_loader&, const size_t ) -> string
	{
		return	"#define figure_parameter_capacity " + to_string( parameter_capacity );
	}


	//	evaluator definitions concatenated from the loaded figure files
	auto figure_functions( const figure_loader& loader, const size_t marker_line ) -> string
	{
		return	restored_diagnostics(
			 loader.figures( )
				| values
				| transform( &figure_loader::entry::m_source )
				| join_with( string_view{ "\n\n" } )
				| to
			,marker_line
		);
	}


	//	integer switch dispatching on the type id assigned by the figure loader
	auto figure_dispatch( const figure_loader& loader, const size_t marker_line ) -> string
	{
		const string cases = loader.figures( )
			|	transform( [ ]( const auto& figure_entry ) { return figure_case( figure_entry.first, figure_entry.second ); } )
			|	join_with( string_view{ "\n" } )
			|	to;
		return	restored_diagnostics(
			 "\tswitch( uint( figure_item.type_id ) )\n\t{\n"
			+ cases
			+ "\n\t}"
			,marker_line
		);
	}


	//	one glsl template placeholder pairing its marker line with its replacement producer
	struct glsl_placeholder
	{
		string_view	m_marker;
		string( *m_produce )( const figure_loader& loader, const size_t marker_line );
	};


	//	every placeholder of the fragment template in replacement order
	inline constexpr array< glsl_placeholder, 3 > glsl_placeholders{ {
		 { "//	@figure_parameter_capacity", parameter_capacity_define }
		,{ "//	@figure_functions", figure_functions }
		,{ "//	@figure_dispatch", figure_dispatch }
	} };


	//	fragment template copy with every placeholder replaced by its generated content
	auto assemble_fragment( const string& fragment_template, const figure_loader& loader ) -> string
	{
		string source = fragment_template;
		for( const auto& placeholder : glsl_placeholders )
			replace_placeholder(
				 source
				,placeholder.m_marker
				,placeholder.m_produce( loader, line_number_at( fragment_template, placeholder.m_marker ) )
			);
		return	source;
	}


	//	name of the prototype assembled in memory instead of loaded by the file based shader loader
	inline constexpr string_view shader_name = "agnostic-pipeline";


	//	both template sections compiled in memory with figure evaluators from the figure loader
	auto compiled_shaders( const figure_loader& loader ) -> array< shader, 2 >
	{
		using	enum	shader::type;

		map_figure_traits( [ & ]( const is_figure_trait auto&... trait )
		{
			for( const string_view name : array{ evaluator_identifier< remove_cvref_t< decltype( trait ) > >( )... } )
				ensure(
					 contains( loader.figures( ), string( name ) )
					,"missing figure evaluator: " + string( name )
				);
		} );

		const text_file file( "data/shader/" + string( shader_name ) + ".glsl" );
		ensure( file.exists( ), "unable to read shader file: " + string( shader_name ) );

		const string source = file.content( );
		return	{
			 shader{ section( source, vertex_shader_marker ), vertex }
			,shader{ assemble_fragment( section( source, fragment_shader_marker ), loader ), fragment }
		};
	}


	//	write one figure payload inside its shared parameter slot budget
	template< is_figure_trait t_figure >
	auto register_figure(
		 figure& figure_item
		,const figure_loader& loader
		,const position& figure_position
		,const color& figure_color
		,const array< float, t_figure::m_parameter_count >& parameters
	) -> void
	{
		figure_item.m_position = figure_position;
		figure_item.m_type_id = loader[ evaluator_identifier< t_figure >( ) ].m_type_id;
		figure_item.m_color = figure_color;
		figure_item.m_parameters.fill( 0.0f );
		copy( parameters, figure_item.m_parameters.begin( ) );
	}


	//	write one figure payload from a cpu shape carrying a figure trait
	template< typename t_shape >
		requires is_figure_trait< typename t_shape::figure_trait >
	auto register_figure(
		 figure& figure_item
		,const figure_loader& loader
		,const t_shape& shape
		,const color& figure_color
	) -> void
	{
		register_figure< typename t_shape::figure_trait >(
			 figure_item
			,loader
			,shape.position( )
			,figure_color
			,shape.parameters( )
		);
	}


	class demo_logic final : public window::listener, public enable_shared_from_this< demo_logic >
	{
	public:
		using	geometry	=	::sak::g2i;
		using	enum	window::flag;

		static constexpr float m_camera_speed = 1.5f;

		demo_logic( )
			:m_application( )
			,m_window( "agnostic pipeline", { opengl, resizable } )
			,m_context( m_window )
			,m_environment( 1 )
			,m_camera( 1 )
			,m_figures( 3 )
			,m_vertex_array( )
			,m_figure_loader( )
			,m_shader_program( compiled_shaders( m_figure_loader ) )
		{
			m_environment.bind( 0 );
			m_camera.bind( 1 );
			m_figures.bind( 2 );

			m_vertex_array.bind( );
			m_shader_program.use( );

			m_environment[ 0 ] = environment{
				 { 0.0f, 0.0f, 0.0f, 1.0f }
				,2.0f
				,0.3f
				,0.7f
				,{ 0.0f, 0.0f }
				,0.0f
			};
			m_camera[ 0 ] = camera{
				 { 0.0f, 0.0f, -2.0f }
				,{ 0.0f, 0.0f,  1.0f }
				,{ 1.0f, 0.0f,  0.0f }
				,{ 0.0f, 1.0f,  0.0f }
			};

			register_figure< figure_trait::sphere >(
				 m_figures[ 0 ]
				,m_figure_loader
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
			register_figure( m_figures[ 1 ], m_figure_loader, blue_triangle, { 0.0f, 0.0f, 1.0f, 1.0f } );
			register_figure(
				 m_figures[ 2 ]
				,m_figure_loader
				,capsule{
					 capsule::data{
						 position{ 0.80f, -0.40f, 1.25f }
						,position{ 1.20f,  0.40f, 0.75f }
					}
					,0.22f
				}
				,{ 0.0f, 1.0f, 0.0f, 1.0f }
			);

			m_environment.flush( );
			m_camera.flush( );
			m_figures.flush( );

			pixel_resize( m_window.pixel_size( ) );
		}

		auto run( ) -> void
		{
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
			m_environment[ 0 ].m_half_screen_size = screen_size / 2.0f;
			m_environment[ 0 ].m_coordinate_scale = 2.0f / min( screen_size );
			m_environment.flush( );
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
			camera& current = m_camera[ 0 ];
			current.m_position += *fold_left_first(
				 zip_transform(
					 multiplies
					,array{ current.m_right, current.m_forward, current.m_up }
					,m_movement
				)
				,plus
			) * ( m_camera_speed * delta_seconds );
			m_camera.flush( );
		}

		application	m_application;
		window		m_window;
		context		m_context;
		shader::storage_buffer< environment >	m_environment;
		shader::storage_buffer< camera >	m_camera;
		shader::storage_buffer< figure >	m_figures;
		vertex_array	m_vertex_array;
		figure_loader	m_figure_loader;
		program< array< shader, 2 > >	m_shader_program;
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
	__using( ::gl::, demo_logic )

	const vector< string > arguments( argument_values, argument_values + argument_count );
	if( contains( arguments, { "-h", "--help" } ) )
		return	println( "this executable renders a sphere, a triangle and a capsule through the agnostic pipeline; use w a s d to move the camera, f11 toggles fullscreen and escape quits" ), exit_success;

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


