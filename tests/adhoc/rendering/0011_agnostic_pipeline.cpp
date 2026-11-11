//	
//	SPDX-FileCopyrightText: 2026 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	SPDX-License-Identifier: GPL-3.0-or-later
//	


//	using	config/sdl3.json


#define GLAD_GL_IMPLEMENTATION


#include <format>
#include <sak/opengl/program.hpp>
#include <sak/opengl/vertex_array.hpp>
#include <sak/sdl3/application.hpp>
#include <sak/sdl3/opengl/context.hpp>
#include <game/fps.hpp>


namespace gl {

	__using( ::std::
		,apply
		,array
		,convertible_to
		,enable_shared_from_this
		,size_t
		,string
		,string_view
		,to_string
		,tuple
		,vector
	)
	__using( ::std::chrono::, duration, high_resolution_clock )
	__using( ::std::ranges::, count )
	__using( ::std::views::, join_with )
	__using( ::sak::, ensure )
	__using( ::sak::fso::, text_file )
	__using( ::sak::math::, min )
	__using( ::sak::opengl::, program, shader, vertex_array )
	__using( ::sak::ranges::, contains, to )
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


	inline constexpr size_t parameter_capacity = 1uz;
	using	figure = figure_payload< parameter_capacity >;


	//	compile-time figure descriptor pairing a gpu type id with its static glsl evaluator
	template< typename t_type >
	concept is_figure_trait = requires
	{
		{ t_type::m_type_id } -> convertible_to< GLuint >;
		{ t_type::m_parameter_count } -> convertible_to< size_t >;
		{ t_type::m_evaluator_name } -> convertible_to< string_view >;
		{ t_type::m_evaluator_source } -> convertible_to< string_view >;
	};


	struct sphere_figure
	{
		static constexpr GLuint	m_type_id = 0u;
		static constexpr size_t	m_parameter_count = 1uz;
		static constexpr string_view	m_evaluator_name = "sphere";
		static constexpr string_view	m_evaluator_source = R"glsl(
//	unit-ray sphere evaluator returning world-metric hit distance and inclination cosine
hit_result evaluate_sphere( vec3 ray_origin, vec3 unit_direction, figure figure_item )
{
	float	radius = figure_item.parameters[ 0 ];
	vec3	local_center = figure_item.position - ray_origin;
	float	along = dot( local_center, unit_direction );
	float	opposite_leg_squared = dot( local_center, local_center ) - square( along );
	float	radius_squared = square( radius );
	if( opposite_leg_squared > radius_squared )
		return	hit_result( false, -1.0, 0.0 );
	float	ratio = opposite_leg_squared / radius_squared;
	float	cos_incidence = sqrt( 1.0 - clamp( ratio, 0.0, 1.0 ) );
	float	half_chord = radius * cos_incidence;
	float	near_distance = along - half_chord;
	float	far_distance = along + half_chord;
	if( far_distance <= 0.0 )
		return	hit_result( false, -1.0, 0.0 );
	float	hit_distance = near_distance > 0.0 ? near_distance : far_distance;
	return	hit_result( true, hit_distance, cos_incidence );
}
)glsl";
	};


	struct cube_figure
	{
		static constexpr GLuint	m_type_id = 1u;
		static constexpr size_t	m_parameter_count = 1uz;
		static constexpr string_view	m_evaluator_name = "cube";
		static constexpr string_view	m_evaluator_source = R"glsl(
//	dimension-symmetric axis-aligned slab box evaluator
hit_result evaluate_cube( vec3 ray_origin, vec3 unit_direction, figure figure_item )
{
	vec3	half_size = vec3( figure_item.parameters[ 0 ] );
	vec3	local_origin = ray_origin - figure_item.position;
	vec3	inverse_direction = 1.0 / unit_direction;
	vec3	first_slab = ( -half_size - local_origin ) * inverse_direction;
	vec3	second_slab = ( half_size - local_origin ) * inverse_direction;
	vec3	near_slab = min( first_slab, second_slab );
	vec3	far_slab = max( first_slab, second_slab );
	float	enter_distance = max_axis( near_slab );
	float	exit_distance = min_axis( far_slab );
	if( exit_distance < max( enter_distance, 0.0 ) )
		return	hit_result( false, -1.0, 0.0 );
	if( exit_distance <= 0.0 )
		return	hit_result( false, -1.0, 0.0 );
	bool	is_exterior = enter_distance > 0.0;
	float	hit_distance = is_exterior ? enter_distance : exit_distance;
	vec3	face_mask = is_exterior ? step( vec3( enter_distance ), near_slab ) : step( far_slab, vec3( exit_distance ) );
	float	cos_incidence = dot( abs( unit_direction ), face_mask ) / length( face_mask );
	return	hit_result( true, hit_distance, cos_incidence );
}
)glsl";
	};


	//	the registered figure set is the single source of truth for the generated glsl
	inline constexpr tuple figure_traits{ sphere_figure{ }, cube_figure{ } };


	//	apply one per-trait mapping across the whole registered figure set
	template< typename t_mapping >
	constexpr auto map_figure_traits( const t_mapping& mapping )
	{
		return	apply( mapping, figure_traits );
	}


	//	one dispatch case generated per registered figure trait
	auto figure_case( const is_figure_trait auto& figure_trait ) -> string
	{
		return	"\t\tcase " + to_string( figure_trait.m_type_id )
			+ "u: return evaluate_" + string( figure_trait.m_evaluator_name )
			+ "( ray_origin, unit_direction, figure_item );";
	}


	//	per-trait mapping that validates the shared parameter slot budget
	inline constexpr auto payload_within_capacity = [ ]( const is_figure_trait auto&... figure_trait )
	{
		return	( true and ... and ( figure_trait.m_parameter_count <= parameter_capacity ) );
	};


	//	per-trait mapping that concatenates the evaluator definitions
	inline constexpr auto evaluator_sources = [ ]( const is_figure_trait auto&... figure_trait )
		-> string
	{
		return	array{ figure_trait.m_evaluator_source... }
			| join_with( string_view{ "\n\n" } )
			| to;
	};


	//	per-trait mapping that concatenates the generated dispatch cases
	inline constexpr auto dispatch_cases = [ ]( const is_figure_trait auto&... figure_trait )
		-> string
	{
		return	array{ figure_case( figure_trait )... }
			| join_with( string_view{ "\n" } )
			| to;
	};


	//	compile-time validation of the shared payload capacity for every registered figure
	static_assert(
		 map_figure_traits( payload_within_capacity )
		,"figure parameter count exceeds the shared payload capacity"
	);


	//	marker lines delimiting the two shader bodies of the template
	inline constexpr string_view vertex_shader_marker = "//	vertex-shader";
	inline constexpr string_view fragment_shader_marker = "//	fragment-shader";


	//	position of a placeholder marker inside a glsl text while asserting the text still carries it
	auto find_placeholder( const string& source, const string_view marker ) -> size_t
	{
		const size_t position = source.find( marker );
		ensure( position < source.size( ), "missing glsl placeholder: " + string( marker ) );
		return	position;
	}


	//	replace one glsl placeholder while asserting the template still carries it
	auto replace_placeholder(
		 string& source
		,const string_view marker
		,const string_view content
	) -> void
	{
		const size_t position = find_placeholder( source, marker );
		source.replace( position, marker.size( ), content );
	}


	//	body between the first and the second occurrence of a section marker line
	auto section( const string& source, const string_view marker ) -> string
	{
		const size_t first = source.find( marker );
		ensure( first < source.size( ), "missing glsl section marker: " + string( marker ) );
		const size_t marker_end = source.find( '\n', first );
		ensure( marker_end < source.size( ), "unterminated glsl section marker line: " + string( marker ) );
		const size_t second = source.find( marker, marker_end );
		ensure( second < source.size( ), "unterminated glsl section marker: " + string( marker ) );
		return	source.substr( marker_end + 1uz, second - marker_end - 1uz );
	}


	//	one-based line number of a placeholder marker inside its source
	auto line_number_at( const string& source, const string_view marker ) -> size_t
	{
		return	1uz + static_cast< size_t >( count(
			 string_view{ source.data( ), find_placeholder( source, marker ) }
			,'\n'
		) );
	}


	//	close an injected region by mapping the next line back onto the template
	auto restored_diagnostics( const string_view content, const size_t marker_line ) -> string
	{
		return	string( content ) + "\n#line " + to_string( marker_line + 1uz );
	}


	//	capacity define replacing the parameter capacity marker
	auto parameter_capacity_define( const size_t ) -> string
	{
		return	"#define figure_parameter_capacity " + to_string( parameter_capacity );
	}


	//	evaluator definitions concatenated from the registered figure traits
	auto figure_functions( const size_t marker_line ) -> string
	{
		return	restored_diagnostics( map_figure_traits( evaluator_sources ), marker_line );
	}


	//	integer switch dispatching on the type id stored inside the figure payload
	auto figure_dispatch( const size_t marker_line ) -> string
	{
		return	restored_diagnostics(
			 "\tswitch( uint( figure_item.type_id ) )\n\t{\n"
			+ map_figure_traits( dispatch_cases )
			+ "\n\t}"
			,marker_line
		);
	}


	//	one glsl template placeholder pairing its marker line with its replacement producer
	struct glsl_placeholder
	{
		string_view	m_marker;
		string( *m_produce )( const size_t marker_line );
	};


	//	every placeholder of the fragment template in replacement order
	inline constexpr array< glsl_placeholder, 3 > glsl_placeholders{ {
		 { "//	@figure_parameter_capacity", parameter_capacity_define }
		,{ "//	@figure_functions", figure_functions }
		,{ "//	@figure_dispatch", figure_dispatch }
	} };


	//	fragment template copy with every placeholder replaced by its generated content
	auto assemble_fragment( const string& fragment_template ) -> string
	{
		string source = fragment_template;
		for( const auto& placeholder : glsl_placeholders )
			replace_placeholder(
				 source
				,placeholder.m_marker
				,placeholder.m_produce( line_number_at( fragment_template, placeholder.m_marker ) )
			);
		return	source;
	}


	//	name of the prototype assembled in memory instead of loaded by the file based shader loader
	inline constexpr string_view shader_name = "agnostic-pipeline";


	//	both template sections compiled in memory without the file based shader loader
	auto compiled_shaders( ) -> array< shader, 2 >
	{
		using	enum	shader::type;

		const text_file file( "data/shader/" + string( shader_name ) + ".glsl" );
		ensure( file.exists( ), "unable to read shader file: " + string( shader_name ) );

		const string source = file.content( );
		const string vertex_source = section( source, vertex_shader_marker );
		const string fragment_source = assemble_fragment( section( source, fragment_shader_marker ) );
		return	{ shader{ vertex_source, vertex }, shader{ fragment_source, fragment } };
	}


	//	write one figure payload inside its shared parameter slot budget
	template< is_figure_trait t_figure >
	auto register_figure(
		 figure& figure_item
		,const position& figure_position
		,const color& figure_color
		,const array< float, t_figure::m_parameter_count >& parameters
	) -> void
	{
		figure_item.m_position = figure_position;
		figure_item.m_type_id = static_cast< float >( t_figure::m_type_id );
		figure_item.m_color = figure_color;
		figure_item.m_parameters = parameters;
	}


	class demo_logic final : public window::listener, public enable_shared_from_this< demo_logic >
	{
	public:
		using	geometry	=	::sak::g2i;
		using	enum	window::flag;

		static constexpr float camera_speed = 1.5f;

		demo_logic( )
			:m_application( )
			,m_window( "agnostic pipeline", { opengl, resizable } )
			,m_context( m_window )
			,m_environment( 1 )
			,m_camera( 1 )
			,m_figures( 2 )
			,m_vertex_array( )
			,m_shader_program( compiled_shaders( ) )
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
			m_camera[ 0 ] = camera{ { 0.0f, 0.0f, -2.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } };

			register_figure< sphere_figure >(
				 m_figures[ 0 ]
				,{ -0.55f, 0.0f, 1.0f }
				,{ 1.0f, 0.0f, 0.0f, 1.0f }
				,{ 0.45f }
			);
			register_figure< cube_figure >(
				 m_figures[ 1 ]
				,{ 0.55f, 0.0f, 1.0f }
				,{ 0.0f, 0.0f, 1.0f, 1.0f }
				,{ 0.45f }
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
			else
				for( const auto& entry : key_movements )
					if( event.key == entry.m_key )
						m_movement[ entry.m_axis ] = entry.m_value;
		}

		void key_up( const SDL_KeyboardEvent& event ) override
		{
			for( const auto& entry : key_movements )
				if( event.key == entry.m_key and m_movement[ entry.m_axis ] == entry.m_value )
					m_movement[ entry.m_axis ] = 0.0f;
		}

		void focus_lost( ) override { m_movement = { }; }

	private:
		struct key_movement
		{
			SDL_Keycode	m_key;
			size_t		m_axis;
			float		m_value;
		};

		static constexpr array< key_movement, 4 > key_movements{ {
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
			const position world_direction = current.m_right * m_movement[ 0 ]
				+ current.m_forward * m_movement[ 1 ]
				+ current.m_up * m_movement[ 2 ];
			current.m_position += world_direction * ( camera_speed * delta_seconds );
			m_camera.flush( );
		}

		application	m_application;
		window		m_window;
		context		m_context;
		shader::storage_buffer< environment >	m_environment;
		shader::storage_buffer< camera >	m_camera;
		shader::storage_buffer< figure >	m_figures;
		vertex_array	m_vertex_array;
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
		return	println( "this executable renders a sphere and a cube through the agnostic pipeline; use w a s d to move the camera, f11 toggles fullscreen and escape quits" ), exit_success;

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


