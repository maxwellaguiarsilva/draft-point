//	
//	SPDX-FileCopyrightText: 2026 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	SPDX-License-Identifier: GPL-3.0-or-later
//	


//	using	config/sdl3.json


#define GLAD_GL_IMPLEMENTATION


#include <format>
#include <map>
#include <regex>
#include <sak/fso/text_file.hpp>
#include <sak/opengl/program.hpp>
#include <sak/pattern/to_number.hpp>
#include <sak/pattern/value_or.hpp>
#include <sak/ranges/contains.hpp>
#include <sak/sdl3/application.hpp>
#include <sak/sdl3/opengl/context.hpp>
#include <game/fps.hpp>


namespace gl {

	using	direction	=	::sak::g3f::point;
	__using_alias( ::sak::g3f::, color, position, size )
	__using( ::std::, array, size_t, string, vector )
	__using( ::std::, define_static_array, make_unique, map, unique_ptr )
	__using( ::std::, regex, sregex_iterator )
	__using( ::std::meta::, enumerators_of, identifier_of )
	__using( ::std::regex_constants::, ECMAScript, multiline )
	__using( ::std::views::, transform, zip )
	__using( ::sak::, ensure )
	__using( ::sak::fso::, text_file )
	__using( ::sak::math::, cosine, min, rotate, sine )
	__using( ::sak::opengl::, shader )
	__using( ::sak::ranges::, contains, count_to, to )
	__using( ::sak::ranges::views::, rotated )
	__using( ::sak::sdl3::, application, window )


	//	reflection stays in constant evaluation, runtime only joins identifiers
	consteval auto shader_type_enumerators( )
	{
		return	define_static_array( enumerators_of( ^^shader::type ) );
	}


	//	combined glsl files are split on marker lines and compiled once per name
	class shader_loader
	{
	public:
		explicit shader_loader( const string& base_path = "data/shader" )
			:m_base_path( base_path )
		{ }

		auto operator[ ]( const string& name ) -> const map< shader::type, shader >&
		{
			if( not m_cache.contains( name ) )
				m_cache.emplace( name, load( name ) );
			return	*m_cache.at( name );
		}

	private:
		auto load( const string& name ) const -> unique_ptr< map< shader::type, shader > >
		{
			text_file file( m_base_path + "/" + name + ".glsl" );
			ensure( file.exists( ), "shader file not found: " + name );
			ensure( file.content( ).has_value( ), "unable to read shader file: " + name );
			const string& content = file.content( ).value( );
			auto table = make_unique< map< shader::type, shader > >( );
			map< string, shader::type > kind_by_name;
			string alternation;
			template for( constexpr auto enumerator : shader_type_enumerators( ) )
			{
				const string identifier( identifier_of( enumerator ) );
				kind_by_name.emplace( identifier, [: enumerator :] );
				if( not alternation.empty( ) )
					alternation += "|";
				alternation += identifier;
			}
			const string expression = "^//\\t(" + alternation + ")-shader\\r?\\n([\\s\\S]*?)(?=^//\\t(?:" + alternation + ")-shader\\r?\\n|$)";
			const regex section_pattern( expression, ECMAScript | multiline );
			const sregex_iterator first( content.begin( ), content.end( ), section_pattern );
			const sregex_iterator last;
			for( auto iterator = first; iterator not_eq last; ++iterator )
			{
				const string section_name = ( *iterator )[ 1 ].str( );
				const string body = ( *iterator )[ 2 ].str( );
				if( body.find_first_not_of( " \t\r\n" ) == string::npos )
					continue;
				const shader::type kind = kind_by_name.at( section_name );
				ensure( not contains( *table, kind ), "duplicate shader section: " + section_name + " in " + name );
				table->try_emplace( kind, body, kind );
			}
			ensure( not table->empty( ), "no shader section found in shader file: " + name );
			return	table;
		}

		string m_base_path;
		map< string, unique_ptr< map< shader::type, shader > > > m_cache;
	};


	//	the eight basic ansi colors, indexed by the rgb channel bits of the index
	inline constexpr array< color, 8 > palette = { {
		 {	0.0f	,0.0f	,0.0f	,1.0f	}
		,{	1.0f	,0.0f	,0.0f	,1.0f	}
		,{	0.0f	,1.0f	,0.0f	,1.0f	}
		,{	1.0f	,1.0f	,0.0f	,1.0f	}
		,{	0.0f	,0.0f	,1.0f	,1.0f	}
		,{	1.0f	,0.0f	,1.0f	,1.0f	}
		,{	0.0f	,1.0f	,1.0f	,1.0f	}
		,{	1.0f	,1.0f	,1.0f	,1.0f	}
	} };


	struct sphere
	{
		position	m_position;
		float		m_radius;
		color		m_color;
	};	//	total 8 floats


	//	shared environment, transferred as flat floats and assembled manually on the gpu like spheres
	struct environment
	{
		position	m_cam_position;
		direction	m_cam_forward;
		direction	m_cam_right;
		direction	m_cam_up;
		color		m_background_color;
		float		m_focal;
		float		m_ambient;
		float		m_volume;
	};	//	total 19 floats


	//	regular polygon centered at the origin, the authoritative cpu geometry
	class polygon
	{
	public:
		static constexpr float polygon_radius = 0.7f;
		static constexpr float pulsation_speed = 3.0f;
		static constexpr float pulsation_amplitude = 0.05f;
		static constexpr float pulsation_phase_step = 3.14159265f / 2.0f;

		explicit polygon( const size_t total )
			: m_spheres( ), m_base_radius( 0.8f * polygon_radius * sine( 3.14159265f / total ) )
		{
			const float step = 2.0f * 3.14159265f / total;
			m_spheres.reserve( total );
			for( const size_t index : count_to( total ) )
				m_spheres.push_back( {
					 position{ polygon_radius * cosine( step * index ), polygon_radius * sine( step * index ), 1.0f }
					,m_base_radius
					,palette[ index % palette.size( ) ]
				} );
		}

		auto turn( const float angle ) -> void
		{
			for( sphere& current : m_spheres )
				current.m_position = rotate( current.m_position, position{ 0.0f, 0.0f, 1.0f }, angle ) | to;
			m_changed = true;
		}

		auto cycle_colors( const bool forward ) -> void
		{
			const vector< color > colors = m_spheres
				|	transform( &sphere::m_color )
				|	rotated( forward ? 1 : m_spheres.size( ) - 1 )
				|	to;
			for( auto [ current, color_value ] : zip( m_spheres, colors ) )
				current.m_color = color_value;
			m_changed = true;
		}

		auto consume_changed( ) noexcept -> bool { return ::std::exchange( m_changed, false ); }

		auto update( const float delta_seconds ) -> void
		{
			m_time += delta_seconds;
			for( const size_t index : count_to( m_spheres.size( ) ) )
				m_spheres[ index ].m_radius = m_base_radius + pulsation_amplitude * sine( pulsation_speed * m_time + index * pulsation_phase_step );
			m_changed = true;
		}

		auto data( ) const noexcept -> const sphere* { return m_spheres.data( ); }
		auto byte_size( ) const noexcept -> size_t { return m_spheres.size( ) * sizeof( sphere ); }
		auto count( ) const noexcept -> size_t { return m_spheres.size( ); }

	private:
		vector< sphere > m_spheres;
		float m_base_radius;
		float m_time{ 0.0f };
		bool m_changed{ false };
	};


	class window_listener final : public window::listener
	{
	public:
		using	geometry	=	::sak::g2i;
		__using_static( geometry::, width, height )

		static constexpr float rotation_speed = 1.5f;

		window_listener( window& target_window, application& target_application, polygon& target_mesh, const GLuint target_program_id )
			:m_window( target_window )
			,m_application( target_application )
			,m_mesh( target_mesh )
			,m_program_id( target_program_id )
		{ pixel_resize( target_window.pixel_size( ) ); }

		void pixel_resize( const geometry::size& new_size ) override { gl_program_uniform_2f( m_program_id, 1, width( new_size ), height( new_size ) ); }

		auto turn_direction( ) const noexcept -> float { return m_turn_direction; }
		auto turn_direction( const float value ) noexcept -> void { m_turn_direction = value; }

		void key_down( const SDL_KeyboardEvent& event ) override
		{
			if( event.key == SDLK_ESCAPE )
				m_application.quit( );
			else if( event.key == SDLK_F11 and not event.repeat )
				m_window.toggle_fullscreen( );
			else if( event.key == SDLK_LEFT )
				turn_direction( -1.0f );
			else if( event.key == SDLK_RIGHT )
				turn_direction( 1.0f );
			else if( event.key == SDLK_UP )
				m_mesh.cycle_colors( true );
			else if( event.key == SDLK_DOWN )
				m_mesh.cycle_colors( false );
		}

		void key_up( const SDL_KeyboardEvent& event ) override
		{
			if( event.key == SDLK_LEFT and turn_direction( ) < 0.0f )
				turn_direction( 0.0f );
			else if( event.key == SDLK_RIGHT and turn_direction( ) > 0.0f )
				turn_direction( 0.0f );
		}

		void focus_lost( ) override { turn_direction( 0.0f ); }

		auto update( const float delta_seconds ) -> void
		{
			if( turn_direction( ) not_eq 0.0f )
				m_mesh.turn( turn_direction( ) * rotation_speed * delta_seconds );
			m_mesh.update( delta_seconds );
		}

	private:
		window&			m_window;
		application&	m_application;
		polygon&		m_mesh;
		GLuint			m_program_id;
		float			m_turn_direction{ 0.0f };
	};


} 


auto main( const int argument_count, const char* argument_values[ ] ) -> int
{
	__using( ::std::
		,format
		,make_shared
		,map
		,println
		,runtime_error
		,string
		,vector
		,views::values
	)
	__using( ::sak::, exit_success, exit_failure, ensure )
	__using( ::sak::math::, between )
	__using( ::sak::opengl::, program, shader )
	__using( ::sak::pattern::, to_number, value_or )
	__using( ::sak::ranges::, contains )
	__using( ::sak::sdl3::, application, window )
	__using( ::sak::sdl3::opengl::, context )
	__using( ::std::chrono::, duration, high_resolution_clock )
	__using( ::game::, fps )
	__using( ::gl::, environment, polygon, shader_loader, window_listener )

	const vector< string > arguments( argument_values, argument_values + argument_count );
	if( contains( arguments, { "-h", "--help" } ) )
		return	println( "this executable is a modern opengl rgb shadertoy demo" ), exit_success;

	const int parsed_total = to_number( value_or( arguments, 1uz, string{ "8" } ), 0 );

	try
	{
		println( "starting modern opengl rgb shadertoy example" );

		application app;

		//	create a raii window and opengl context, declared before gpu resources so they outlive them on destruction
		using enum window::flag;
		window application_window( "modern opengl rgb shadertoy", { opengl, resizable } );
		context gl_context( application_window );

		//	cpu-owned geometry, rewritten by input and mirrored to the gpu when it changes
		polygon mesh( between( parsed_total, 3, 16 ) ? parsed_total : 8 );

		//	allocate dummy vao and storage for spheres as shader storage buffer
		GLuint vertex_array = 0;
		gl_create_vertex_arrays( 1, &vertex_array );
		GLuint sphere_buffer = 0;
		gl_create_buffers( 1, &sphere_buffer );
		gl_named_buffer_storage( sphere_buffer, mesh.byte_size( ), mesh.data( ), GL_DYNAMIC_STORAGE_BIT );
		gl_bind_buffer_base( GL_SHADER_STORAGE_BUFFER, 0, sphere_buffer );

		//	shader program loaded on demand from data/shader
		shader_loader loader;
		const program shader_program( loader[ "shadertoy" ] | values );
		shader_program.use( );

		gl_program_uniform_1i( shader_program.id( ), 0, static_cast< GLint >( mesh.count( ) ) );

		//	shared environment, uploaded once because it rarely changes
		static_assert( sizeof( environment ) == 19 * sizeof( float ), "environment must stay tightly packed" );
		const float ambient = 0.3f;
		const environment environment_data = {
			 {	0.0f	,0.0f	,-2.0f	}
			,{	0.0f	,0.0f	,1.0f	}
			,{	1.0f	,0.0f	,0.0f	}
			,{	0.0f	,1.0f	,0.0f	}
			,{	0.0f	,0.0f	,0.0f	,1.0f	}
			,2.0f
			,ambient
			,1.0f - ambient
		};
		GLuint environment_buffer = 0;
		gl_create_buffers( 1, &environment_buffer );
		gl_named_buffer_storage( environment_buffer, sizeof( environment ), &environment_data, 0 );
		gl_bind_buffer_base( GL_SHADER_STORAGE_BUFFER, 1, environment_buffer );

		const auto listener = make_shared< window_listener >( application_window, app, mesh, shader_program.id( ) );
		application_window.listeners( ) += listener;

		fps frame_limiter( 60 );
		frame_limiter.compute( );

		auto last_time = high_resolution_clock::now( );

		app.run( [ & ]( )
		{
			const auto current_time = high_resolution_clock::now( );
			listener->update( duration< float >( current_time - last_time ).count( ) );
			last_time = current_time;

			//	the cpu is the source of truth, so upload the mesh only when input rewrote it
			if( mesh.consume_changed( ) )
				gl_named_buffer_sub_data( sphere_buffer, 0, mesh.byte_size( ), mesh.data( ) );

			gl_bind_vertex_array( vertex_array );
			gl_draw_arrays( GL_TRIANGLE_STRIP, 0, 4 );

			application_window.swap( );
			frame_limiter.compute( );
		} );

		//	clean up raw opengl objects while the context is still current;
		gl_delete_vertex_arrays( 1, &vertex_array );
		gl_delete_buffers( 1, &sphere_buffer );
		gl_delete_buffers( 1, &environment_buffer );

		println( "modern opengl rgb shadertoy finished successfully" );
	}
	catch( const runtime_error& error )
	{
		println( "test failed: {}", error.what( ) );
		return	exit_failure;
	}

	return	exit_success;
}


