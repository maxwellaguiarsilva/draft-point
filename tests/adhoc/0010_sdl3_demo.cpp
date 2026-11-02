//	
//	SPDX-FileCopyrightText: 2026 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	SPDX-License-Identifier: GPL-3.0-or-later
//	


//	using	config/sdl3.json


#define GLAD_GL_IMPLEMENTATION


#include <format>
#include <span>
#include <sak/opengl/program.hpp>
#include <sak/opengl/vertex_array.hpp>
#include <sak/pattern/parse.hpp>
#include <sak/pattern/value_or.hpp>
#include <sak/sdl3/application.hpp>
#include <sak/sdl3/opengl/context.hpp>
#include <game/fps.hpp>


namespace gl {

	__using( ::std::
		,array
		,declval
		,enable_shared_from_this
		,size_t
		,span
		,string
		,vector
	)
	__using( ::std::chrono::, duration, high_resolution_clock )
	__using( ::std::numbers::, pi_v )
	__using( ::std::views::, transform, values, zip )
	__using( ::sak::math::, between, bit_of, cosine, min, rotate, sine )
	__using( ::sak::opengl::, program, shader, vertex_array )
	__using( ::sak::ranges::, contains, count_to, to )
	__using( ::sak::ranges::views::, rotated )
	__using( ::sak::sdl3::, application, window )
	__using( ::sak::sdl3::opengl::, context )
	__using( ::game::, fps )
	__using_alias( ::sak::g3f::, color, position )
	using	direction		=	::sak::g3f::point;
	using	planar_point	=	::sak::g2f::point;


	//	the eight basic ansi colors, indexed by the rgb channel bits of the index
	inline constexpr array< color, 8 > palette = count_to( 8uz )
		|	transform( [ ]( const size_t index ) { return color{ bit_of( index, 0 ), bit_of( index, 1 ), bit_of( index, 2 ), 1.0f }; } )
		|	to;


	struct sphere
	{
		position	m_position;
		float		m_radius;
		color		m_color;
	};	//	total 8 floats


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


	//	temporal pulsation applied to the radius of each element around the origin
	class pulsation
	{
	public:
		pulsation(
			 const float radius		=	1.0f
			,const float speed		=	1.0f
			,const float amplitude	=	1.0f
			,const float slices		=	1.0f
		)
			:m_radius( radius )
			,m_speed( speed )
			,m_amplitude( amplitude )
			,m_slices( slices )
		{ pulsation::slices( m_slices ); }

		auto radius( ) const noexcept -> float { return m_radius; }
		auto radius( const float value ) noexcept -> void { m_radius = value; }
		auto speed( ) const noexcept -> float { return m_speed; }
		auto speed( const float value ) noexcept -> void { m_speed = value; }
		auto amplitude( ) const noexcept -> float { return m_amplitude; }
		auto amplitude( const float value ) noexcept -> void { m_amplitude = value; }

		auto slices( ) const noexcept -> float { return m_slices; }
		auto slices( const float value ) noexcept -> void
		{
			m_slices = value;
			m_phase_step = 2.0f * pi_v< float > / value;
		}

		auto radius_at( const float time, const float index ) const noexcept -> float
		{ return m_radius + m_amplitude * sine( m_speed * time + index * m_phase_step ); }

	private:
		float	m_radius;
		float	m_speed;
		float	m_amplitude;
		float	m_slices;
		float	m_phase_step;
	};


	//	regular polygon centered at the origin, the authoritative cpu geometry
	class polygon
	{
	public:
		static constexpr float polygon_radius = 0.7f;

		shader::storage_buffer< sphere >& m_storage;

		explicit polygon( shader::storage_buffer< sphere >& storage )
			:m_storage( storage )
			,m_pulsation( 0.8f * polygon_radius * sine( pi_v< float > / m_storage.count( ) ), 3.0f, 0.05f, 4.0f )
		{
			const float step = 2.0f * pi_v< float > / m_storage.count( );
			for( const size_t index : count_to( m_storage.count( ) ) )
				m_storage[ index ] = sphere{
					 position{ polygon_radius * cosine( step * index ), polygon_radius * sine( step * index ), 1.0f }
					,m_pulsation.radius( )
					,palette[ index % palette.size( ) ]
				};
			m_storage.flush( );
		}

		auto turn( const float angle ) -> void
		{
			for( sphere& current : spheres( ) )
				current.m_position = rotate( current.m_position, position{ 0.0f, 0.0f, 1.0f }, angle ) | to;
			m_storage.flush( );
		}

		auto cycle_colors( const bool forward ) -> void
		{
			const auto objects = spheres( );
			const vector< color > reordered = objects | transform( &sphere::m_color ) | rotated( forward ? 1 : objects.size( ) - 1 ) | to;
			for( auto [ current, color_value ] : zip( objects, reordered ) )
				current.m_color = color_value;
			m_storage.flush( );
		}

		auto update( const float delta_seconds ) -> void
		{
			m_time += delta_seconds;
			for( const size_t index : count_to( m_storage.count( ) ) )
				m_storage[ index ].m_radius = m_pulsation.radius_at( m_time, index );
			m_storage.flush( );
		}

		auto count( ) const noexcept -> size_t { return m_storage.count( ); }

	private:
		auto spheres( ) noexcept -> span< sphere > { return { m_storage.data( ), m_storage.count( ) }; }

		pulsation m_pulsation;
		float m_time{ 0.0f };
	};


	class demo_logic final : public window::listener, public enable_shared_from_this< demo_logic >
	{
	public:
		using	geometry	=	::sak::g2i;
		using	enum	window::flag;

		static constexpr float rotation_speed = 1.5f;

		explicit demo_logic( const size_t sphere_count )
			:m_application( )
			,m_window( "modern opengl rgb shadertoy", { opengl, resizable } )
			,m_context( m_window )
			,m_scene_environment( 1 )
			,m_scene_camera( 1 )
			,m_scene_spheres( between( sphere_count, 3uz, 16uz ) ? sphere_count : 8uz )
			,m_polygon( m_scene_spheres )
			,m_loader( )
			,m_shader_program( m_loader[ "sdl3-demo" ] | values )
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
			m_scene_environment.flush( );
			m_scene_camera.flush( );

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
				m_polygon.cycle_colors( true );
			else if( event.key == SDLK_DOWN )
				m_polygon.cycle_colors( false );
		}

		void key_up( const SDL_KeyboardEvent& event ) override
		{
			if( event.key == SDLK_LEFT and turn_direction( ) < 0.0f )
				turn_direction( 0.0f );
			else if( event.key == SDLK_RIGHT and turn_direction( ) > 0.0f )
				turn_direction( 0.0f );
		}

		void focus_lost( ) override { turn_direction( 0.0f ); }

	private:
		using	shader_range	=	decltype( declval< const shader::table& >( ) | values );

		auto update( const float delta_seconds ) -> void
		{
			if( turn_direction( ) not_eq 0.0f )
				m_polygon.turn( turn_direction( ) * rotation_speed * delta_seconds );
			m_polygon.update( delta_seconds );
		}

		application	m_application;
		window		m_window;
		context		m_context;
		shader::storage_buffer< environment >	m_scene_environment;
		shader::storage_buffer< camera >	m_scene_camera;
		shader::storage_buffer< sphere >	m_scene_spheres;
		polygon		m_polygon;
		vertex_array	m_vertex_array;
		shader::loader	m_loader;
		program< shader_range >	m_shader_program;
		fps		m_frame_limiter{ 60 };
		float		m_turn_direction{ 0.0f };
	};


} 


auto main( const int argument_count, const char* argument_values[ ] ) -> int
{
	__using( ::std::, make_shared, println, runtime_error, string, vector )
	__using( ::sak::, exit_success, exit_failure )
	__using( ::sak::pattern::, parse, value_or )
	__using( ::sak::ranges::, contains )
	__using( ::gl::, demo_logic )

	const vector< string > arguments( argument_values, argument_values + argument_count );
	if( contains( arguments, { "-h", "--help" } ) )
		return	println( "this executable is a modern opengl rgb shadertoy demo" ), exit_success;

	const int parsed_total = parse( value_or( arguments, 1uz, string{ "8" } ), 0 );

	try
	{
		println( "starting modern opengl rgb shadertoy example" );

		const auto demo = make_shared< demo_logic >( parsed_total );
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


