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
#include <sak/ranges/contains.hpp>
#include <sak/sdl3/application.hpp>
#include <sak/sdl3/opengl/context.hpp>
#include <SDL3/SDL.h>
#include <game/fps.hpp>


namespace gl {

	using	geometry	=	::sak::g3f;
	using	point		=	geometry::point;

	__using( ::std::
		,array
		,size_t
		,string
	)
	__using( ::std::views::, transform, zip )
	__using( ::sak::math::, rotate )
	__using( ::sak::ranges::, to )
	__using( ::sak::ranges::views::, rotated )

	const string vertex_shader_source = R"glsl(
#version 460 core

layout( location = 0 ) in vec3 position;
layout( location = 1 ) in vec3 color;

out vec3 fragment_color;

void main( )
{
	gl_Position = vec4( position, 1.0 );
	fragment_color = color;
}
)glsl";

	const string fragment_shader_source = R"glsl(
#version 460 core

in vec3 fragment_color;
out vec4 final_color;

void main( )
{
	final_color = vec4( fragment_color, 1.0 );
}
)glsl";


	//	cpu-owned triangle, the authoritative geometry mirrored into the gpu vertex buffer
	class triangle
	{
	public:
		struct vertex
		{
			point position;
			point color;
		};

		auto turn( const float angle ) -> void
		{
			const point axis{ 0.0f, 0.0f, 1.0f };
			for( vertex& current : m_vertices )
				current.position = rotate( current.position, axis, angle ) | to;
			m_changed = true;
		}

		auto cycle_colors( const bool forward ) -> void
		{
			const array< point, 3 > colors = m_vertices | transform( &vertex::color ) | rotated( forward ? 1 : 2 ) | to;
			for( auto [ current, color ] : zip( m_vertices, colors ) )
				current.color = color;
			m_changed = true;
		}

		auto consume_changed( ) noexcept -> bool
		{
			const bool result = m_changed;
			m_changed = false;
			return	result;
		}

		auto data( ) const noexcept -> const vertex* { return m_vertices.data( ); }
		auto byte_size( ) const noexcept -> size_t { return sizeof( m_vertices ); }

	private:
		array< vertex, 3 > m_vertices{ {
			 {	{	0.0f	,0.5f	,0.0f	}	,{	1.0f	,0.0f	,0.0f	}	}
			,{	{	-0.5f	,-0.5f	,0.0f	}	,{	0.0f	,1.0f	,0.0f	}	}
			,{	{	0.5f	,-0.5f	,0.0f	}	,{	0.0f	,0.0f	,1.0f	}	}
		} };
		bool m_changed{ false };
	};


} 


auto main( const int argument_count, const char* argument_values[ ] ) -> int
{
	__using( ::sak::, exit_success, exit_failure, ensure )
	__using( ::sak::opengl::, program, shader )
	__using( ::sak::sdl3::, application, window )
	__using( ::sak::sdl3::opengl::, context )
	__using( ::gl::, triangle, vertex_shader_source, fragment_shader_source )
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
	__using( ::sak::ranges::, contains )
	__using( ::game::, fps )

	const vector< string > arguments( argument_values, argument_values + argument_count );
	if( contains( arguments, { "-h", "--help" } ) )
		return	println( "this executable is a modern opengl rgb triangle demo" ), exit_success;

	try
	{
		println( "starting modern opengl rgb triangle example" );

		application app;

		//	create a raii window and opengl context, declared before gpu resources so they outlive them on destruction
		using enum window::flag;
		window application_window( "modern opengl rgb triangle", { opengl, resizable } );
		context gl_context( application_window );

		//	cpu-owned geometry, rewritten by input and mirrored to the gpu when it changes
		triangle mesh;

		struct keyboard_listener final : public window::listener
		{
			keyboard_listener( window& target_window, application& target_application, triangle& target_mesh )
				: m_window( target_window ), m_application( target_application ), m_mesh( target_mesh )
			{ }

			void key_down( const SDL_KeyboardEvent& event ) override
			{
				constexpr float rotation_step = 3.14159265f / 36.0f;

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
			triangle&		m_mesh;
		};

		const auto key_listener = make_shared< keyboard_listener >( application_window, app, mesh );
		application_window.listeners( ) += key_listener;

		//	allocate immutable storage with dynamic updates enabled, the modern replacement for the usage hint
		GLuint vertex_array = 0;
		gl_create_vertex_arrays( 1, &vertex_array );
		GLuint vertex_buffer = 0;
		gl_create_buffers( 1, &vertex_buffer );
		gl_named_buffer_storage( vertex_buffer, mesh.byte_size( ), mesh.data( ), GL_DYNAMIC_STORAGE_BIT );

		//	bind the buffer to the vertex array and describe the vertex layout
		gl_vertex_array_vertex_buffer( vertex_array, 0, vertex_buffer, 0, sizeof( triangle::vertex ) );
		gl_vertex_array_attrib_format( vertex_array, 0, 3, GL_FLOAT, GL_FALSE, offsetof( triangle::vertex, position ) );
		gl_vertex_array_attrib_binding( vertex_array, 0, 0 );
		gl_enable_vertex_array_attrib( vertex_array, 0 );
		gl_vertex_array_attrib_format( vertex_array, 1, 3, GL_FLOAT, GL_FALSE, offsetof( triangle::vertex, color ) );
		gl_vertex_array_attrib_binding( vertex_array, 1, 0 );
		gl_enable_vertex_array_attrib( vertex_array, 1 );

		//	shader program
		map< shader::type, shader > shader_map;
		using enum shader::type;
		shader_map.try_emplace( vertex		,vertex_shader_source	,vertex		);
		shader_map.try_emplace( fragment	,fragment_shader_source	,fragment	);
		const program shader_program( shader_map | values );
		shader_program.use( );

		fps frame_limiter( 60 );
		frame_limiter.compute( );

		app.run( [ & ]( )
		{
			//	the cpu is the source of truth, so upload the mesh only when input rewrote it
			if( mesh.consume_changed( ) )
				gl_named_buffer_sub_data( vertex_buffer, 0, mesh.byte_size( ), mesh.data( ) );

			gl_clear_color( 0.0f, 0.0f, 0.0f, 1.0f );
			gl_clear( GL_COLOR_BUFFER_BIT );

			gl_bind_vertex_array( vertex_array );
			gl_draw_arrays( GL_TRIANGLES, 0, 3 );

			application_window.swap( );
			frame_limiter.compute( );
		} );

		//	clean up raw opengl objects while the context is still current;
		gl_delete_vertex_arrays( 1, &vertex_array );
		gl_delete_buffers( 1, &vertex_buffer );

		println( "modern opengl rgb triangle finished successfully" );
	}
	catch( const runtime_error& error )
	{
		println( "test failed: {}", error.what( ) );
		return	exit_failure;
	}

	return	exit_success;
}


