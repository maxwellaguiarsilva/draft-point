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
		,array
		,declval
		,enable_shared_from_this
		,size_t
		,string
		,vector
	)
	__using( ::std::chrono::, duration, high_resolution_clock )
	__using( ::std::views::, values )
	__using( ::sak::math::, min )
	__using( ::sak::opengl::, program, shader, vertex_array )
	__using( ::sak::ranges::, contains )
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

	struct sphere
	{
		position	m_position;
		float		m_radius;
		color		m_color;
	};	//	total 8 floats


	class demo_logic final : public window::listener, public enable_shared_from_this< demo_logic >
	{
	public:
		using	geometry	=	::sak::g2i;
		using	enum	window::flag;

		static constexpr float camera_speed = 1.5f;

		demo_logic( )
			:m_application( )
			,m_window( "modern opengl rgb shadertoy", { opengl, resizable } )
			,m_context( m_window )
			,m_scene_environment( 1 )
			,m_scene_camera( 1 )
			,m_scene_spheres( 2 )
			,m_loader( )
			,m_shader_program( m_loader[ "sdl3-shadertoy" ] | values )
		{
			m_scene_environment.bind( 0 );
			m_scene_camera.bind( 1 );
			m_scene_spheres.bind( 2 );

			m_vertex_array.bind( );
			m_shader_program.use( );

			m_scene_environment[ 0 ] = environment{
				 { 0.0f, 0.0f, 0.0f, 1.0f }
				,2.0f
				,0.3f
				,0.7f
				,{ 0.0f, 0.0f }
				,0.0f
			};
			m_scene_camera[ 0 ] = camera{ { 0.0f, 0.0f, -2.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } };
			m_scene_spheres[ 0 ] = sphere{
				 { -0.55f, 0.0f, 1.0f }
				,0.45f
				,{ 1.0f, 0.0f, 0.0f, 1.0f }
			};
			m_scene_spheres[ 1 ] = sphere{
				 { 0.55f, 0.0f, 1.0f }
				,0.45f
				,{ 0.0f, 0.0f, 1.0f, 1.0f }
			};
			m_scene_environment.flush( );
			m_scene_camera.flush( );
			m_scene_spheres.flush( );

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
			m_scene_environment[ 0 ].m_half_screen_size = screen_size / 2.0f;
			m_scene_environment[ 0 ].m_coordinate_scale = 2.0f / min( screen_size );
			m_scene_environment.flush( );
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
		using	shader_range	=	decltype( declval< const shader::table& >( ) | values );

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
			camera& current = m_scene_camera[ 0 ];
			const position world_direction = current.m_right * m_movement[ 0 ]
				+ current.m_forward * m_movement[ 1 ]
				+ current.m_up * m_movement[ 2 ];
			current.m_position += world_direction * ( camera_speed * delta_seconds );
			m_scene_camera.flush( );
		}

		application	m_application;
		window		m_window;
		context		m_context;
		shader::storage_buffer< environment >	m_scene_environment;
		shader::storage_buffer< camera >	m_scene_camera;
		shader::storage_buffer< sphere >	m_scene_spheres;
		vertex_array	m_vertex_array;
		shader::loader	m_loader;
		program< shader_range >	m_shader_program;
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
		return	println( "this executable renders two static spheres; use w a s d to move the camera, f11 toggles fullscreen and escape quits" ), exit_success;

	try
	{
		println( "starting modern opengl rgb shadertoy example" );

		const auto demo = make_shared< demo_logic >( );
		demo->run( );

		println( "modern opengl rgb shadertoy finished successfully" );
	}
	catch( const runtime_error& error )
	{
		println( "test failed: {}", error.what( ) );
		return	exit_failure;
	}

	return	exit_success;
}


