//	
//	Copyright (C) 2026 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	
//	This program is free software: you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//	
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//	
//	You should have received a copy of the GNU General Public License
//	along with this program.  If not, see <http://www.gnu.org/licenses/>.
//	
//	
//	File:   source/sdl3_demo.cpp
//	Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	
//	Created on 2026-08-09 21:51:36
//	


//	using	config/sdl3.json


#define GLAD_GL_IMPLEMENTATION


#include <array>
#include <cstddef>
#include <format>
#include <map>
#include <ranges>
#include <string>
#include <vector>
#include <sak/opengl/program.hpp>
#include <sak/opengl/shader.hpp>
#include <sak/ranges/contains.hpp>
#include <sak/sdl3/application.hpp>
#include <SDL3/SDL.h>
#include <game/fps.hpp>


namespace gl {

	__using( ::std::, string )

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


} 


namespace sdl3 {

using	::std::string;
using	::sak::ensure;


class window
{
public:
	window( const string& title, const int width, const int height )
	{
		try
		{
			ensure( SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 ), "failed to set opengl major version" );
			ensure( SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 6 ), "failed to set opengl minor version" );
			ensure( SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE ), "failed to set opengl core profile" );

			m_id = SDL_CreateWindow( title.c_str( ), width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE );
			ensure( m_id not_eq nullptr, "failed to create sdl window" );

			m_context = SDL_GL_CreateContext( m_id );
			ensure( m_context not_eq nullptr, "failed to create opengl context" );
			ensure( SDL_GL_MakeCurrent( m_id, m_context ), "failed to make opengl context current" );
			ensure( gladLoadGL( SDL_GL_GetProcAddress ) not_eq 0, "failed to load opengl functions with glad" );
		}
		catch( ... )
		{
			SDL_GL_DestroyContext( m_context );
			SDL_DestroyWindow( m_id );
			throw;
		}
	}

	~window( ) noexcept
	{
		SDL_GL_DestroyContext( m_context );
		SDL_DestroyWindow( m_id );
	}

	delete_copy_move_ctc( window )

	auto id( ) const noexcept -> SDL_Window* { return m_id; }
	auto context( ) const noexcept -> SDL_GLContext { return m_context; }
	auto swap( ) const noexcept -> void { SDL_GL_SwapWindow( m_id ); }
	auto title( const string& title ) -> void { SDL_SetWindowTitle( m_id, title.c_str( ) ); }

private:
	SDL_Window*		m_id{ nullptr };
	SDL_GLContext	m_context{ nullptr };
};

} 


auto main( const int argument_count, const char* argument_values[ ] ) -> int
{
	__using( ::sak::, exit_success, exit_failure, ensure )
	__using( ::sak::opengl::, program, shader )
	using	::sak::sdl3::application;
	__using( ::gl::, vertex_shader_source, fragment_shader_source )
	__using( ::sdl3::, window )
	__using( ::std::
		,array
		,format
		,map
		,println
		,runtime_error
		,string
		,vector
		,views::values
	)
	__using( ::sak::ranges::, contains )
	using	::game::fps;

	const vector< string > arguments( argument_values, argument_values + argument_count );
	if( contains( arguments, { "-h", "--help" } ) )
		return	println( "this executable is a modern opengl rgb triangle demo" ), exit_success;

	try
	{
		println( "starting modern opengl rgb triangle example" );

		application app( application::flag::video );

		//	create a raii window with an opengl 4.6 core context,
		//	declared before gpu resources so it outlives them on destruction
		window application_window( "modern opengl rgb triangle", 800, 600 );

		map< shader::type, shader > shader_map;
		shader_map.try_emplace( shader::type::vertex, vertex_shader_source, shader::type::vertex );
		shader_map.try_emplace( shader::type::fragment, fragment_shader_source, shader::type::fragment );
		const program shader_program( shader_map | values );

		//	triangle vertices stored as a compact array of structs
		struct vertex
		{
			float position[ 3 ];
			float color[ 3 ];
		};

		constexpr array< vertex, 3 > vertices = { {
			{ { 0.0f,  0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
			{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
			{ { 0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
		} };

		//	create and initialize objects with the modern direct state access api,
		//	which edits objects by name instead of the legacy bind-to-edit model
		GLuint vertex_array = 0;
		gl_create_vertex_arrays( 1, &vertex_array );
		GLuint vertex_buffer = 0;
		gl_create_buffers( 1, &vertex_buffer );
		gl_named_buffer_data( vertex_buffer, sizeof( vertices ), vertices.data( ), GL_STATIC_DRAW );

		//	bind the buffer to the vertex array and describe the vertex layout
		gl_vertex_array_vertex_buffer( vertex_array, 0, vertex_buffer, 0, sizeof( vertex ) );
		gl_vertex_array_attrib_format( vertex_array, 0, 3, GL_FLOAT, GL_FALSE, offsetof( vertex, position ) );
		gl_vertex_array_attrib_binding( vertex_array, 0, 0 );
		gl_enable_vertex_array_attrib( vertex_array, 0 );
		gl_vertex_array_attrib_format( vertex_array, 1, 3, GL_FLOAT, GL_FALSE, offsetof( vertex, color ) );
		gl_vertex_array_attrib_binding( vertex_array, 1, 0 );
		gl_enable_vertex_array_attrib( vertex_array, 1 );

		//	initialize fps controller for 60 frames per second
		fps frame_limiter( 60 );
		frame_limiter.compute( );

		bool is_running = true;
		SDL_Event event;

		println( "entering main loop. close window to exit." );

		//	simple event processing and rendering loop
		while( is_running )
		{
			while( SDL_PollEvent( &event ) )
			{
				if( event.type == SDL_EVENT_QUIT )
					is_running = false;
				else if( event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED )
					gl_viewport( 0, 0, event.window.data1, event.window.data2 );
			}

			gl_clear_color( 0.0f, 0.0f, 0.0f, 1.0f );
			gl_clear( GL_COLOR_BUFFER_BIT );

			shader_program.use( );
			gl_bind_vertex_array( vertex_array );
			gl_draw_arrays( GL_TRIANGLES, 0, 3 );

			application_window.swap( );

			//	control frame rate using game::fps instead of fixed sdl_delay
			const int current_fps = frame_limiter.compute( );

			//	update window title with the current fps
			const string title = format( "modern opengl rgb triangle | fps: {}", current_fps );
			application_window.title( title );
		}

		//	clean up raw opengl objects while the context is still current;
		//	the window raii destroys the context and sdl on its own scope exit
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


