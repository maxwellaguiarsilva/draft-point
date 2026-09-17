//	
//	SPDX-FileCopyrightText: 2026 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	SPDX-License-Identifier: GPL-3.0-or-later
//	


//	using	config/sdl3.json


#define GLAD_GL_IMPLEMENTATION


#include <array>
#include <cstddef>
#include <format>
#include <map>
#include <ranges>
#include <string>
#include <utility>
#include <vector>
#include <sak/geometry/geometry.hpp>
#include <sak/opengl/program.hpp>
#include <sak/pattern/to_number.hpp>
#include <sak/pattern/value_or.hpp>
#include <sak/ranges/contains.hpp>
#include <sak/sdl3/application.hpp>
#include <sak/sdl3/opengl/context.hpp>
#include <game/fps.hpp>


namespace gl {

	using	geometry	=	::sak::g3f;
	using	point		=	geometry::point;

	__using( ::std::
		,array
		,size_t
		,string
		,vector
	)
	__using( ::std::views::, drop, take, transform, zip )
	__using( ::sak::math::, cosine, min, rotate, sine )
	__using( ::sak::ranges::, count_to, to )
	__using( ::sak::ranges::views::, rotated )
	__using( ::sak::sdl3::, application, window )

	const string vertex_shader_source = R"glsl(
#version 460 core

const vec2 vertices[ 4 ] = vec2[ 4 ](
	vec2( -1.0, -1.0 ),
	vec2(  1.0, -1.0 ),
	vec2( -1.0,  1.0 ),
	vec2(  1.0,  1.0 )
);

void main( )
{
	gl_Position = vec4( vertices[ gl_VertexID ], 0.0, 1.0 );
}
)glsl";

	const string fragment_shader_source = R"glsl(
#version 460 core

layout( location = 0 ) uniform int sphere_count;
layout( location = 1 ) uniform vec2 resolution;

layout( std430, binding = 0 ) readonly buffer sphere_buffer
{
	float raw_spheres[];
};

out vec4 final_color;

struct sphere
{
	vec3 position;
	float radius;
	vec3 color;
};

sphere get_sphere( int index )
{
	int base_index = index * 7;
	return sphere(
		vec3( raw_spheres[ base_index + 0 ], raw_spheres[ base_index + 1 ], raw_spheres[ base_index + 2 ] ),
		raw_spheres[ base_index + 3 ],
		vec3( raw_spheres[ base_index + 4 ], raw_spheres[ base_index + 5 ], raw_spheres[ base_index + 6 ] )
	);
}

void main( )
{
	vec2 input_coord = ( gl_FragCoord.xy - 0.5 * resolution ) * ( 2.0 / min( resolution.x, resolution.y ) );

	const float focal = 2.0;
	const vec3 cam_position = vec3( 0.0, 0.0, -2.0 );
	const vec3 cam_forward = vec3( 0.0, 0.0, 1.0 );
	const vec3 cam_right = vec3( 1.0, 0.0, 0.0 );
	const vec3 cam_up = vec3( 0.0, 1.0, 0.0 );

	vec3 direction = cam_forward * focal + cam_right * input_coord.x + cam_up * input_coord.y;

	float best_distance = 1e20;
	const float edge_shade = 0.7;
	const vec3 background_color = vec3( 0.0, 0.0, 0.0 );
	vec3 color = background_color;

	for( int index = 0; index < sphere_count; ++index )
	{
		sphere object = get_sphere( index );
		vec3 hypotenuse = object.position - cam_position;
		float dot_direction = dot( direction, direction );
		float dot_hypotenuse_direction = dot( hypotenuse, direction );
		float opposite_leg_squared = dot( hypotenuse, hypotenuse ) - ( dot_hypotenuse_direction * dot_hypotenuse_direction ) / dot_direction;
		float radius_squared = object.radius * object.radius;
		float ratio = opposite_leg_squared / radius_squared;
		float distance = length( hypotenuse );

		if( dot_hypotenuse_direction > 0.0 && distance < best_distance && ratio <= 1.0 )
		{
			best_distance = distance;
			color = object.color * ( 1.0 - edge_shade * clamp( ratio, 0.0, 1.0 ) );
		}
	}

	final_color = vec4( color, 1.0 );
}
)glsl";


	//	the eight basic ansi colors, indexed by the rgb channel bits of the index
	inline constexpr array< point, 8 > palette = { {
		 {	0.0f	,0.0f	,0.0f	}
		,{	1.0f	,0.0f	,0.0f	}
		,{	0.0f	,1.0f	,0.0f	}
		,{	1.0f	,1.0f	,0.0f	}
		,{	0.0f	,0.0f	,1.0f	}
		,{	1.0f	,0.0f	,1.0f	}
		,{	0.0f	,1.0f	,1.0f	}
		,{	1.0f	,1.0f	,1.0f	}
	} };


	struct sphere
	{
		using	geometry	=	::sak::g3f;
		using	color		=	geometry::point;
		using	position	=	geometry::position;

		position	m_position;
		float		radius;
		color		m_color;
	};	//	total 7 floats


	//	regular polygon centered at the origin, the authoritative cpu geometry
	class polygon
	{
	public:
		static constexpr float polygon_radius = 0.7f;

		explicit polygon( const size_t total )
			: m_spheres( )
		{
			const float step = 2.0f * 3.14159265f / total;
			const float sphere_radius = ( 0.8f * polygon_radius * sine( 3.14159265f / total ) );
			m_spheres.reserve( total + 1 );
			m_spheres.push_back( { sphere::position{ 0.0f, 0.0f, 1.0f }, sphere_radius, palette[ 7 ] } );
			for( const size_t index : count_to( total ) )
				m_spheres.push_back( {
					 sphere::position{ polygon_radius * cosine( step * index ), polygon_radius * sine( step * index ), 1.0f }
					,sphere_radius * cosine( step * index )
					,palette[ ( index % 7 ) + 1 ]
				} );
		}

		auto turn( const float angle ) -> void
		{
			const sphere::position axis{ 0.0f, 0.0f, 1.0f };
			for( sphere& current : m_spheres )
				current.m_position = rotate( current.m_position, axis, angle ) | to;
			m_changed = true;
		}

		auto cycle_colors( const bool forward ) -> void
		{
			const size_t perimeter_count = m_spheres.size( ) - 1;
			const vector< sphere::color > colors = m_spheres
				|	drop( 1 )
				|	take( perimeter_count )
				|	transform( &sphere::m_color )
				|	rotated( forward ? 1 : perimeter_count - 1 )
				|	to;
			for( auto [ current, color_value ] : zip( m_spheres | drop( 1 ), colors ) )
				current.m_color = color_value;
			m_changed = true;
		}

		auto consume_changed( ) noexcept -> bool
		{
			const bool result = m_changed;
			m_changed = false;
			return	result;
		}

		auto data( ) const noexcept -> const sphere* { return m_spheres.data( ); }
		auto byte_size( ) const noexcept -> size_t { return m_spheres.size( ) * sizeof( sphere ); }
		auto count( ) const noexcept -> size_t { return m_spheres.size( ); }

	private:
		vector< sphere > m_spheres;
		bool m_changed{ false };
	};


	class window_listener final : public window::listener
	{
	public:
		using	geometry	=	::sak::g2i;
		__using_static( geometry::, width, height )

		window_listener( window& target_window, application& target_application, polygon& target_mesh, const GLuint target_program_id )
			: m_window( target_window ), m_application( target_application ), m_mesh( target_mesh ), m_program_id( target_program_id )
		{
			pixel_resize( target_window.pixel_size( ) );
		}

		void pixel_resize( const geometry::size& new_size ) override
		{
			gl_program_uniform_2f( m_program_id, 1, static_cast< float >( width( new_size ) ), static_cast< float >( height( new_size ) ) );
		}

		void key_down( const SDL_KeyboardEvent& event ) override
		{
			float rotation_step = 3.14159265f / static_cast< float >( m_mesh.count( ) * 2 );

			if( event.key == SDLK_ESCAPE )
				m_application.quit( );
			else if( event.key == SDLK_F11 and not event.repeat )
				m_window.toggle_fullscreen( );
			else if( event.key == SDLK_LEFT )
				m_mesh.turn( -rotation_step );
			else if( event.key == SDLK_RIGHT )
				m_mesh.turn( rotation_step );
			else if( event.key == SDLK_UP )
				m_mesh.cycle_colors( true );
			else if( event.key == SDLK_DOWN )
				m_mesh.cycle_colors( false );
		}

	private:
		window&			m_window;
		application&	m_application;
		polygon&		m_mesh;
		GLuint			m_program_id;
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
	__using( ::game::, fps )
	__using( ::gl::, polygon, window_listener, vertex_shader_source, fragment_shader_source )

	const vector< string > arguments( argument_values, argument_values + argument_count );
	if( contains( arguments, { "-h", "--help" } ) )
		return	println( "this executable is a modern opengl rgb shadertoy demo" ), exit_success;

	const int parsed_total = to_number( value_or( arguments, 1uz, string{ "8" } ), 0 );
	const size_t total = between( parsed_total, 3, 16 ) ? parsed_total : 8;

	try
	{
		println( "starting modern opengl rgb shadertoy example" );

		application app;

		//	create a raii window and opengl context, declared before gpu resources so they outlive them on destruction
		using enum window::flag;
		window application_window( "modern opengl rgb shadertoy", { opengl, resizable } );
		context gl_context( application_window );

		//	cpu-owned geometry, rewritten by input and mirrored to the gpu when it changes
		polygon mesh( total );

		//	allocate dummy vao and storage for spheres as shader storage buffer
		GLuint vertex_array = 0;
		gl_create_vertex_arrays( 1, &vertex_array );
		GLuint sphere_buffer = 0;
		gl_create_buffers( 1, &sphere_buffer );
		gl_named_buffer_storage( sphere_buffer, mesh.byte_size( ), mesh.data( ), GL_DYNAMIC_STORAGE_BIT );
		gl_bind_buffer_base( GL_SHADER_STORAGE_BUFFER, 0, sphere_buffer );

		//	shader program
		map< shader::type, shader > shader_map;
		using enum shader::type;
		shader_map.try_emplace( vertex		,vertex_shader_source	,vertex		);
		shader_map.try_emplace( fragment	,fragment_shader_source	,fragment	);
		const program shader_program( shader_map | values );
		shader_program.use( );

		gl_program_uniform_1i( shader_program.id( ), 0, static_cast< GLint >( mesh.count( ) ) );

		const auto listener = make_shared< window_listener >( application_window, app, mesh, shader_program.id( ) );
		application_window.listeners( ) += listener;

		fps frame_limiter( 60 );
		frame_limiter.compute( );

		app.run( [ & ]( )
		{
			//	the cpu is the source of truth, so upload the mesh only when input rewrote it
			if( mesh.consume_changed( ) )
				gl_named_buffer_sub_data( sphere_buffer, 0, mesh.byte_size( ), mesh.data( ) );

			gl_clear_color( 0.0f, 0.0f, 0.0f, 1.0f );
			gl_clear( GL_COLOR_BUFFER_BIT );

			gl_bind_vertex_array( vertex_array );
			gl_draw_arrays( GL_TRIANGLE_STRIP, 0, 4 );

			application_window.swap( );
			frame_limiter.compute( );
		} );

		//	clean up raw opengl objects while the context is still current;
		gl_delete_vertex_arrays( 1, &vertex_array );
		gl_delete_buffers( 1, &sphere_buffer );

		println( "modern opengl rgb shadertoy finished successfully" );
	}
	catch( const runtime_error& error )
	{
		println( "test failed: {}", error.what( ) );
		return	exit_failure;
	}

	return	exit_success;
}


