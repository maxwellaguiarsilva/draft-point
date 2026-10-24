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
#include <sak/opengl/vertex_array.hpp>
#include <sak/ranges/contains.hpp>
#include <sak/ranges/views/regex_matches.hpp>
#include <sak/sdl3/application.hpp>
#include <sak/sdl3/opengl/context.hpp>
#include <game/fps.hpp>


namespace gl {

	__using( ::std::
		,declval
		,define_static_array
		,enable_shared_from_this
		,is_trivially_copyable_v
		,make_unique
		,map
		,pair
		,ptrdiff_t
		,regex
		,size_t
		,smatch 
		,string
		,string_view
		,unique_ptr
		,vector
	)
	__using( ::std::chrono::, duration, high_resolution_clock )
	__using( ::std::meta::
		,access_context
		,enumerators_of
		,identifier_of
		,info
		,nonstatic_data_members_of
		,offset_of
		,size_of
		,type_of
	)
	__using( ::std::regex_constants::, ECMAScript, multiline )
	__using( ::std::views::, join_with, keys, transform, values )
	__using( ::sak::, ensure )
	__using( ::sak::fso::, text_file )
	__using( ::sak::opengl::, program, shader, vertex_array )
	__using( ::sak::ranges::, contains, to )
	__using( ::sak::ranges::views::, regex_matches )
	__using( ::sak::sdl3::, application, window )
	__using( ::sak::sdl3::opengl::, context )
	__using( ::game::, fps )
	__using_alias( ::sak::g3f::, color, position )
	using	direction		=	::sak::g3f::point;
	using	planar_point	=	::sak::g2f::point;
	using	shader_table	=	map< shader::type, shader >;


	//	combined glsl files are split on marker lines and compiled once per name
	class shader_loader
	{
	public:
		explicit shader_loader( const string& base_path = "data/shader" ) :m_base_path( base_path ) { }

		auto operator[ ]( const string& name ) -> const shader_table&
		{
			if( not m_cache.contains( name ) )
				m_cache.emplace( name, load( name ) );
			return	m_cache.at( name );
		}

	private:
		auto load( const string& name ) const -> shader_table
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

			shader_table table;
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
		map< string, shader_table > m_cache;
	};


	//	a type whose bytes are a contiguous payload with no padding, matching the flat float arrays read on the gpu
	consteval auto has_no_padding( const info type ) -> bool
	{
		ptrdiff_t offset = 0;
		for( const auto member : nonstatic_data_members_of( type, access_context::current( ) ) )
		{
			if( offset_of( member ).bytes not_eq offset )
				return	false;
			offset += size_of( type_of( member ) );
		}
		return	offset == static_cast< ptrdiff_t >( size_of( type ) );
	}

	template< typename t_type >
	concept is_flat = has_no_padding( ^^t_type ) and is_trivially_copyable_v< t_type >;


	//	raii shader storage buffer owning a heap array of packed payloads; binding is a separate explicit step
	template< typename t_payload >
	class shader_storage_buffer
	{
	public:
		static_assert( is_flat< t_payload >, "the ssbo payload must be a tightly packed flat structure" );
		
		//	element count and float stride prefixed to every ssbo payload so the shader iterates generically
		struct ssbo_header { GLuint m_count; GLuint m_stride; };

		shader_storage_buffer( const size_t count )
			:m_raii( make_unique< raii_destructor >( ) )
			,m_data( make_unique< t_payload[ ] >( count ) )
			,m_count( count )
			,m_payload_size( count * sizeof( t_payload ) )
			,m_byte_size( sizeof( ssbo_header ) + m_payload_size )
		{
			GLuint buffer_id = 0;
			gl_create_buffers( 1, &buffer_id );
			id( buffer_id );
			gl_named_buffer_storage( id( ), m_byte_size, nullptr, GL_DYNAMIC_STORAGE_BIT );
			const ssbo_header header{ static_cast< GLuint >( count ), sizeof( t_payload ) / sizeof( float ) };
			gl_named_buffer_sub_data( id( ), 0, sizeof( header ), &header );
		}

		auto bind( const GLuint index ) const noexcept -> void { gl_bind_buffer_base( GL_SHADER_STORAGE_BUFFER, index, id( ) ); }
		auto flush( ) const noexcept -> void { gl_named_buffer_sub_data( id( ), sizeof( ssbo_header ), m_payload_size, m_data.get( ) ); }

		auto operator[ ]( const size_t index ) noexcept -> t_payload& { return m_data[ index ]; }
		auto data( ) const noexcept -> t_payload* { return m_data.get( ); }
		auto count( ) const noexcept -> size_t { return m_count; }

	private:
		auto id( ) const noexcept -> GLuint { return m_raii->m_id; }
		auto id( const GLuint value ) noexcept -> void { m_raii->m_id = value; }

		struct raii_destructor
		{
			~raii_destructor( ) noexcept { gl_delete_buffers( 1, &m_id ); }
			GLuint	m_id{ 0 };
		};

		unique_ptr< raii_destructor > m_raii;
		unique_ptr< t_payload[ ] > m_data;
		size_t m_count;
		const size_t m_payload_size;
		const size_t m_byte_size;
	};


	//	shared environment, transferred as flat floats and assembled manually on the gpu
	struct environment
	{
		color	m_background_color;
		float	m_focal;
		float	m_ambient;
		float	m_volume;
		planar_point	m_resolution;
	};	//	total 9 floats


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
		__using_static( geometry::, width, height )

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
			m_scene_environment[ 0 ].m_resolution = planar_point{ width( new_size ), height( new_size ) };
			m_scene_environment.flush( );
		}

		void key_down( const SDL_KeyboardEvent& event ) override
		{
			if( event.key == SDLK_ESCAPE )
				m_application.quit( );
			else if( event.key == SDLK_F11 and not event.repeat )
				m_window.toggle_fullscreen( );
			else if( event.key == SDLK_W )
				m_forward_pressed = true;
			else if( event.key == SDLK_S )
				m_backward_pressed = true;
			else if( event.key == SDLK_A )
				m_left_pressed = true;
			else if( event.key == SDLK_D )
				m_right_pressed = true;
		}

		void key_up( const SDL_KeyboardEvent& event ) override
		{
			if( event.key == SDLK_W )
				m_forward_pressed = false;
			else if( event.key == SDLK_S )
				m_backward_pressed = false;
			else if( event.key == SDLK_A )
				m_left_pressed = false;
			else if( event.key == SDLK_D )
				m_right_pressed = false;
		}

		void focus_lost( ) override
		{
			m_forward_pressed = false;
			m_backward_pressed = false;
			m_left_pressed = false;
			m_right_pressed = false;
		}

	private:
		using	shader_range	=	decltype( declval< const shader_table& >( ) | values );

		auto update( const float delta_seconds ) -> void
		{
			const float horizontal = m_right_pressed - m_left_pressed;
			const float depth = m_forward_pressed - m_backward_pressed;
			if( horizontal == 0.0f and depth == 0.0f )
				return;
			const float distance = camera_speed * delta_seconds;
			position& current = m_scene_camera[ 0 ].m_position;
			current[ 0 ] += horizontal * distance;
			current[ 2 ] += depth * distance;
			m_scene_camera.flush( );
		}

		application	m_application;
		window		m_window;
		context		m_context;
		shader_storage_buffer< environment >	m_scene_environment;
		shader_storage_buffer< camera >	m_scene_camera;
		shader_storage_buffer< sphere >	m_scene_spheres;
		vertex_array	m_vertex_array;
		shader_loader	m_loader;
		program< shader_range >	m_shader_program;
		fps		m_frame_limiter{ 60 };
		bool		m_forward_pressed{ false };
		bool		m_backward_pressed{ false };
		bool		m_left_pressed{ false };
		bool		m_right_pressed{ false };
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


