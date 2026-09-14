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
	__using( ::sak::math::, cosine, rotate, sine )
	__using( ::sak::ranges::, count_to, to )
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


	//	regular polygon centered at the origin, the authoritative cpu geometry
	class polygon
	{
	public:
		struct vertex
		{
			point position;
			point color;
		};

		static constexpr float radius = 0.5f;

		explicit polygon( const size_t total )
			: m_vertices( )
		{
			const float step = 2.0f * 3.14159265f / total;
			m_vertices.reserve( total + 2 );
			m_vertices.push_back( { point{ 0.0f, 0.0f, 0.0f }, palette[ 0 ] } );
			for( const size_t index : count_to( total ) )
				m_vertices.push_back( {
					 {	radius * cosine( step * index )	,radius * sine( step * index )	,0.0f	}
					,palette[ index % palette.size( ) ]
				} );
			m_vertices.push_back( m_vertices[ 1 ] );
		}

		auto turn( const float angle ) -> void
		{
			const point axis{ 0.0f, 0.0f, 1.0f };
			for( vertex& current : m_vertices )
				current.position = rotate( current.position, axis, angle ) | to;
			m_changed = true;
		}

		auto cycle_colors( const bool forward ) -> void
		{
			const size_t perimeter_count = m_vertices.size( ) - 2;
			const vector< point > colors = m_vertices
				|	drop( 1 )
				|	take( perimeter_count )
				|	transform( &vertex::color )
				|	rotated( forward ? 1 : perimeter_count - 1 )
				|	to;
			for( auto [ current, color ] : zip( m_vertices | drop( 1 ), colors ) )
				current.color = color;
			m_vertices.back( ).color = m_vertices[ 1 ].color;
			m_changed = true;
		}

		auto consume_changed( ) noexcept -> bool
		{
			const bool result = m_changed;
			m_changed = false;
			return	result;
		}

		auto data( ) const noexcept -> const vertex* { return m_vertices.data( ); }
		auto byte_size( ) const noexcept -> size_t { return m_vertices.size( ) * sizeof( vertex ); }
		auto count( ) const noexcept -> size_t { return m_vertices.size( ); }

	private:
		vector< vertex > m_vertices;
		bool m_changed{ false };
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
	__using( ::gl::, polygon, vertex_shader_source, fragment_shader_source )

	const vector< string > arguments( argument_values, argument_values + argument_count );
	if( contains( arguments, { "-h", "--help" } ) )
		return	println( "this executable is a modern opengl rgb polygon demo" ), exit_success;

	const int parsed_total = to_number( value_or( arguments, 1uz, string{ "8" } ), 0 );
	const size_t total = between( parsed_total, 3, 16 ) ? parsed_total : 8;

	try
	{
		println( "starting modern opengl rgb polygon example" );

		application app;

		//	create a raii window and opengl context, declared before gpu resources so they outlive them on destruction
		using enum window::flag;
		window application_window( "modern opengl rgb polygon", { opengl, resizable } );
		context gl_context( application_window );

		//	cpu-owned geometry, rewritten by input and mirrored to the gpu when it changes
		polygon mesh( total );

		struct keyboard_listener final : public window::listener
		{
			keyboard_listener( window& target_window, application& target_application, polygon& target_mesh )
				: m_window( target_window ), m_application( target_application ), m_mesh( target_mesh )
			{ }

			void key_down( const SDL_KeyboardEvent& event ) override
			{
				constexpr float rotation_step = 3.14159265f / 6.0f;

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
		gl_vertex_array_vertex_buffer( vertex_array, 0, vertex_buffer, 0, sizeof( polygon::vertex ) );
		gl_vertex_array_attrib_format( vertex_array, 0, 3, GL_FLOAT, GL_FALSE, offsetof( polygon::vertex, position ) );
		gl_vertex_array_attrib_binding( vertex_array, 0, 0 );
		gl_enable_vertex_array_attrib( vertex_array, 0 );
		gl_vertex_array_attrib_format( vertex_array, 1, 3, GL_FLOAT, GL_FALSE, offsetof( polygon::vertex, color ) );
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
			gl_draw_arrays( GL_TRIANGLE_FAN, 0, mesh.count( ) );

			application_window.swap( );
			frame_limiter.compute( );
		} );

		//	clean up raw opengl objects while the context is still current;
		gl_delete_vertex_arrays( 1, &vertex_array );
		gl_delete_buffers( 1, &vertex_buffer );

		println( "modern opengl rgb polygon finished successfully" );
	}
	catch( const runtime_error& error )
	{
		println( "test failed: {}", error.what( ) );
		return	exit_failure;
	}

	return	exit_success;
}


