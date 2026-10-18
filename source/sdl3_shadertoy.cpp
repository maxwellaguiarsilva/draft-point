//	
//	SPDX-FileCopyrightText: 2026 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	SPDX-License-Identifier: GPL-3.0-or-later
//	


//	using	config/sdl3.json


#define GLAD_GL_IMPLEMENTATION


#include <format>
#include <map>
#include <span>
#include <sak/fso/text_file.hpp>
#include <sak/opengl/program.hpp>
#include <sak/opengl/vertex_array.hpp>
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
		,span
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
	__using( ::std::numbers::, pi_v )
	__using( ::std::regex_constants::, ECMAScript, multiline )
	__using( ::std::views::, join_with, keys, transform, values, zip )
	__using( ::sak::, ensure )
	__using( ::sak::fso::, text_file )
	__using( ::sak::math::, between, bit_of, cosine, rotate, sine )
	__using( ::sak::opengl::, program, shader, vertex_array )
	__using( ::sak::ranges::, contains, count_to, to )
	__using( ::sak::ranges::views::, regex_matches, rotated )
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

		shader_storage_buffer< sphere >& m_storage;

		explicit polygon( shader_storage_buffer< sphere >& storage )
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
		__using_static( geometry::, width, height )

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
			m_scene_environment[ 0 ].m_resolution = planar_point{ width( new_size ), height( new_size ) };
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
		using	shader_range	=	decltype( declval< const shader_table& >( ) | values );

		auto update( const float delta_seconds ) -> void
		{
			if( turn_direction( ) not_eq 0.0f )
				m_polygon.turn( turn_direction( ) * rotation_speed * delta_seconds );
			m_polygon.update( delta_seconds );
		}

		application	m_application;
		window		m_window;
		context		m_context;
		shader_storage_buffer< environment >	m_scene_environment;
		shader_storage_buffer< camera >	m_scene_camera;
		shader_storage_buffer< sphere >	m_scene_spheres;
		polygon		m_polygon;
		vertex_array	m_vertex_array;
		shader_loader	m_loader;
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


