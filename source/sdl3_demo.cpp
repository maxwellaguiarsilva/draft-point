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
#include <utility>
#include <vector>
#include <sak/sak.hpp>
#include <sak/opengl/program.hpp>
#include <sak/pattern/bitmask.hpp>
#include <sak/ranges/contains.hpp>
#include <sak/sdl3/application.hpp>
#include <sak/sdl3/opengl/attributes.hpp>
#include <sak/sdl3/opengl/context.hpp>
#include <sak/sdl3/window.hpp>
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


auto main( const int argument_count, const char* argument_values[ ] ) -> int
{
	__using( ::sak::, exit_success, exit_failure, ensure )
	__using( ::sak::opengl::, program, shader )
	__using( ::sak::sdl3::, application, window )
	__using( ::sak::sdl3::opengl::, attributes, context )
	__using( ::gl::, vertex_shader_source, fragment_shader_source )
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

		application app;

		//	create a raii window and opengl context, declared before gpu resources so they outlive them on destruction
		attributes gl_attributes;
		using	flag		=	window::flag;
		window application_window( "modern opengl rgb triangle", 800, 600, window::window_flags{ flag::opengl, flag::resizable } );
		context gl_context( application_window );
		ensure( gladLoadGL( gl_context.function_pointer( ) ) not_eq 0, "failed to load opengl functions with glad" );

		map< shader::type, shader > shader_map;
		shader_map.try_emplace( shader::type::vertex, vertex_shader_source, shader::type::vertex );
		shader_map.try_emplace( shader::type::fragment, fragment_shader_source, shader::type::fragment );

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

		//	shader program
		const program shader_program( shader_map | values );
		shader_program.use( );

		//	initialize fps controller for 60 frames per second
		fps frame_limiter( 60 );
		frame_limiter.compute( );

		bool is_running = true;
		SDL_Event event;

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

			gl_bind_vertex_array( vertex_array );
			gl_draw_arrays( GL_TRIANGLES, 0, 3 );

			application_window.swap( );
			frame_limiter.compute( );
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


