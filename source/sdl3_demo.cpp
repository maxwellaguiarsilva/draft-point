//	
//	SPDX-FileCopyrightText: 2026 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	SPDX-License-Identifier: GPL-3.0-or-later
//	


//	using	config/sdl3.json


#define GLAD_GL_IMPLEMENTATION


#include <format>
#include <map>
#include <sak/fso/text_file.hpp>
#include <sak/opengl/program.hpp>
#include <sak/pattern/parse.hpp>
#include <sak/pattern/value_or.hpp>
#include <sak/ranges/contains.hpp>
#include <sak/ranges/views/regex_matches.hpp>
#include <sak/sdl3/application.hpp>
#include <sak/sdl3/opengl/context.hpp>
#include <game/fps.hpp>


namespace gl {

	__using( ::std::
		,array
		,define_static_array
		,make_unique
		,map
		,pair
		,regex
		,size_t
		,smatch 
		,string
		,string_view
		,unique_ptr
		,vector
	)
	__using( ::std::meta::, enumerators_of, identifier_of )
	__using( ::std::regex_constants::, ECMAScript, multiline )
	__using( ::std::views::, filter, join_with, keys, transform, zip )
	__using( ::sak::, ensure )
	__using( ::sak::fso::, text_file )
	__using( ::sak::math::, cosine, min, rotate, sine )
	__using( ::sak::opengl::, shader )
	__using( ::sak::ranges::, contains, count_to, to )
	__using( ::sak::ranges::views::, regex_matches, rotated )
	__using( ::sak::sdl3::, application, window )
	__using_alias( ::sak::g3f::, color, position, size )
	using	direction	=	::sak::g3f::point;


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
			return	m_cache.at( name );
		}

	private:
		auto load( const string& name ) const -> map< shader::type, shader >
		{
			text_file file( m_base_path + "/" + name + ".glsl" );
			ensure( file.exists( ), "unable to read shader file: " + name );

			map< string, shader::type > kind_by_name;
			template for( constexpr auto enumerator : define_static_array( enumerators_of( ^^shader::type ) ) )
				kind_by_name.emplace( string( identifier_of( enumerator ) ), [: enumerator :] );

			const string alternation = kind_by_name | keys | join_with( string_view{ "|" } ) | to;
			const string expression = "^//\\t(" + alternation + ")-shader[\\r\\n]([\\s\\S]*?)^//\\t\\1-shader";
			const vector< pair< string, string > > sections = file.content( )
				|	regex_matches( regex( expression, ECMAScript | multiline ) )
				|	transform( [ ]( const smatch& match ) { return pair{ match[ 1 ].str( ), match[ 2 ].str( ) }; } )
				|	to;

			map< shader::type, shader > table;
			for( const auto& [ section_name, body ] : sections )
			{
				const shader::type kind = kind_by_name.at( section_name );
				ensure( not contains( table, kind ), "duplicate shader section: " + section_name + " in " + name );
				table.try_emplace( kind, body, kind );
			}

			ensure( not table.empty( ), "no shader section found in shader file: " + name );
			return	table;
		}

		string m_base_path;
		map< string, map< shader::type, shader > > m_cache;
	};


	//	the eight basic ansi colors, indexed by the rgb channel bits of the index
	inline constexpr array< color, 8 > palette = count_to( 8uz )
		|	transform( [ ]( const size_t index ) { return color{ ( index >> 0 ) & 1, ( index >> 1 ) & 1, ( index >> 2 ) & 1, 1.0f }; } )
		|	to;


	struct sphere
	{
		position	m_position;
		float		m_radius;
		color		m_color;
	};	//	total 8 floats


	//	raii shader storage buffer shared by the geometry and the environment, holding the opengl object on their behalf
	class shader_storage_buffer
	{
	public:
		shader_storage_buffer( const GLuint binding_index, const size_t size_in_bytes, const void* initial_data = nullptr )
			:m_raii( make_unique< raii_destructor >( ) )
		{
			gl_create_buffers( 1, &m_raii->m_id );
			gl_named_buffer_storage( m_raii->m_id, size_in_bytes, initial_data, GL_DYNAMIC_STORAGE_BIT );
			gl_bind_buffer_base( GL_SHADER_STORAGE_BUFFER, binding_index, m_raii->m_id );
		}

		auto update( const size_t size_in_bytes, const void* data ) const noexcept -> void
		{ gl_named_buffer_sub_data( m_raii->m_id, 0, size_in_bytes, data ); }

	private:
		struct raii_destructor
		{
			~raii_destructor( ) noexcept { gl_delete_buffers( 1, &m_id ); }
			GLuint	m_id{ 0 };
		};

		unique_ptr< raii_destructor > m_raii;
	};


	//	shared environment, transferred as flat floats and assembled manually on the gpu like spheres
	class environment
	{
	public:
		environment( )
			:m_buffer( 1, sizeof( data ) )
		{
			const float ambient = 0.3f;
			m_data = data{
				 {	0.0f	,0.0f	,-2.0f	}
				,{	0.0f	,0.0f	,1.0f	}
				,{	1.0f	,0.0f	,0.0f	}
				,{	0.0f	,1.0f	,0.0f	}
				,{	0.0f	,0.0f	,0.0f	,1.0f	}
				,2.0f
				,ambient
				,1.0f - ambient
			};
			static_assert( sizeof( data ) == 19 * sizeof( float ), "environment must stay tightly packed" );
			update( );
		}

		auto update( ) const noexcept -> void { m_buffer.update( sizeof( data ), &m_data ); }

	private:
		struct data
		{
			position	m_cam_position;
			direction	m_cam_forward;
			direction	m_cam_right;
			direction	m_cam_up;
			color		m_background_color;
			float		m_focal;
			float		m_ambient;
			float		m_volume;
		};

		shader_storage_buffer m_buffer;
		data m_data;
	};


	//	regular polygon centered at the origin, the authoritative cpu geometry
	class polygon
	{
	public:
		static constexpr float polygon_radius = 0.7f;
		static constexpr float pulsation_speed = 3.0f;
		static constexpr float pulsation_amplitude = 0.05f;
		static constexpr float pulsation_phase_step = 3.14159265f / 2.0f;

		explicit polygon( const size_t total )
			: m_spheres( )
			,m_base_radius( 0.8f * polygon_radius * sine( 3.14159265f / total ) )
			,m_buffer( 0, total * sizeof( sphere ) )
		{
			const float step = 2.0f * 3.14159265f / total;
			m_spheres.reserve( total );
			for( const size_t index : count_to( total ) )
				m_spheres.push_back( {
					 position{ polygon_radius * cosine( step * index ), polygon_radius * sine( step * index ), 1.0f }
					,m_base_radius
					,palette[ index % palette.size( ) ]
				} );
			flush( );
		}

		auto turn( const float angle ) -> void
		{
			for( sphere& current : m_spheres )
				current.m_position = rotate( current.m_position, position{ 0.0f, 0.0f, 1.0f }, angle ) | to;
			flush( );
		}

		auto cycle_colors( const bool forward ) -> void
		{
			const vector< color > colors = m_spheres | transform( &sphere::m_color ) | rotated( forward ? 1 : m_spheres.size( ) - 1 ) | to;
			for( auto [ current, color_value ] : zip( m_spheres, colors ) )
				current.m_color = color_value;
			flush( );
		}

		auto update( const float delta_seconds ) -> void
		{
			m_time += delta_seconds;
			for( const size_t index : count_to( m_spheres.size( ) ) )
				m_spheres[ index ].m_radius = m_base_radius + pulsation_amplitude * sine( pulsation_speed * m_time + index * pulsation_phase_step );
			flush( );
		}

		auto data( ) const noexcept -> const sphere* { return m_spheres.data( ); }
		auto byte_size( ) const noexcept -> size_t { return m_spheres.size( ) * sizeof( sphere ); }
		auto count( ) const noexcept -> size_t { return m_spheres.size( ); }

	private:
		auto flush( ) noexcept -> void { m_buffer.update( byte_size( ), data( ) ); }

		vector< sphere > m_spheres;
		float m_base_radius;
		shader_storage_buffer m_buffer;
		float m_time{ 0.0f };
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
	__using( ::sak::pattern::, parse, value_or )
	__using( ::sak::ranges::, contains )
	__using( ::sak::sdl3::, application, window )
	__using( ::sak::sdl3::opengl::, context )
	__using( ::std::chrono::, duration, high_resolution_clock )
	__using( ::game::, fps )
	__using( ::gl::, environment, polygon, shader_loader, window_listener )

	const vector< string > arguments( argument_values, argument_values + argument_count );
	if( contains( arguments, { "-h", "--help" } ) )
		return	println( "this executable is a modern opengl rgb shadertoy demo" ), exit_success;

	const int parsed_total = parse( value_or( arguments, 1uz, string{ "8" } ), 0 );

	try
	{
		println( "starting modern opengl rgb shadertoy example" );

		application app;

		//	create a raii window and opengl context, declared before gpu resources so they outlive them on destruction
		using	enum	window::flag;
		window application_window( "modern opengl rgb shadertoy", { opengl, resizable } );
		context gl_context( application_window );

		//	cpu-owned geometry, rewritten by input and mirrored to the gpu when it changes
		polygon mesh( between( parsed_total, 3, 16 ) ? parsed_total : 8 );

		//	allocate dummy vao for the full screen triangle strip
		GLuint vertex_array = 0;
		gl_create_vertex_arrays( 1, &vertex_array );

		//	shader program loaded on demand from data/shader
		shader_loader loader;
		const program shader_program( loader[ "shadertoy" ] | values );
		shader_program.use( );

		gl_program_uniform_1i( shader_program.id( ), 0, static_cast< GLint >( mesh.count( ) ) );

		//	shared environment owns its gpu buffer and uploads itself
		environment scene_environment;

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

			gl_bind_vertex_array( vertex_array );
			gl_draw_arrays( GL_TRIANGLE_STRIP, 0, 4 );

			application_window.swap( );
			frame_limiter.compute( );
		} );

		//	clean up raw opengl objects while the context is still current;
		gl_delete_vertex_arrays( 1, &vertex_array );

		println( "modern opengl rgb shadertoy finished successfully" );
	}
	catch( const runtime_error& error )
	{
		println( "test failed: {}", error.what( ) );
		return	exit_failure;
	}

	return	exit_success;
}


