//	
//	SPDX-FileCopyrightText: 2026 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	SPDX-License-Identifier: GPL-3.0-or-later
//	


#include <exception>
#include <sak/using.hpp>
#include <sak/math/vector.hpp>
#include <game/shadertoy.hpp>
#include <tui/terminal.hpp>
#include <tui/renderer.hpp>
#include <memory>
#include <print>
#include <vector>


namespace {


__using( ::std::, unique_ptr, make_unique, vector )
__using( ::sak::, g2f, g3f )
__using( ::sak::ranges::, to )
__using( ::sak::math::
	,sine
	,cosine
	,absolute
	,clamp
	,min
	,max
	,round_down
	,dot
	,length
	,normalize
	,cross
)
__using_static( g2f::, left, top, width, height )


using	vec2	=	g2f::point;
using	vec3	=	g3f::point;


constexpr auto clamp_vector( const auto& vector, float min_value, float max_value ) noexcept
{
	return	vector | clamp( min_value, max_value ) | to;
}

constexpr auto mix( const auto& first, const auto& second, float factor ) noexcept { return first * ( 1.0f - factor ) + second * factor; }


//	closest ray intersection carries travel distance and surface material
struct intersection
{
	float distance;
	float material_id;
};


//	checkerboard blend factor from the floor plane coordinates
constexpr auto checker_factor( vec3 position ) noexcept
{
	__using_constexpr( g3f::, left, depth )

	return	( int( round_down( left( position ) / 8.0f ) ) + int( round_down( depth( position ) / 8.0f ) ) ) & 1;
}


//	combined diffuse plus ambient plus backlight response
constexpr auto shade( vec3 normal, vec3 light_direction ) noexcept
{
	__using_constexpr( g3f::, top )

	float diffuse = clamp( dot( normal, light_direction ), 0.0f, 1.0f );
	float ambient = 0.3f + 0.2f * top( normal );
	float backlight = 0.2f * clamp( dot( normal, -light_direction ), 0.0f, 1.0f );
	return	diffuse + ambient + backlight;
}


struct primitive
{
	vec3 m_position;
	float m_material_id;

	primitive( vec3 position, float material_id )
		: m_position( position )
		, m_material_id( material_id )
	{ }

	auto material_id( ) const -> float { return m_material_id; }
	virtual auto sdf( vec3 point ) const -> float = 0;
	virtual ~primitive( ) = default;
};

struct plane final : primitive
{
	explicit plane( float height )
		: primitive( vec3{ 0.0f, height, 0.0f }, 1.0f )
	{ }

	auto sdf( vec3 point ) const -> float override
	{
		__using_constexpr( g3f::, top )

		return	top( point ) - top( m_position );
	}
};

struct sphere final : primitive
{
	float m_radius;

	sphere( vec3 position, float radius )
		: primitive( position, 45.0f )
		, m_radius( radius )
	{ }

	auto sdf( vec3 point ) const -> float override { return length( point - m_position ) - m_radius; }
};

struct torus final : primitive
{
	vec2 m_radii;

	torus( vec3 position, vec2 radii )
		: primitive( position, 200.0f )
		, m_radii( radii )
	{ }

	auto sdf( vec3 point ) const -> float override
	{
		__using_constexpr( g3f::, depth )

		auto local = point - m_position;
		vec2 relative_2d_position{ length( vec2{ local } ) - width( m_radii ), depth( local ) };
		return	length( relative_2d_position ) - height( m_radii );
	}
};


auto map( vec3 position, const vector< unique_ptr< primitive > >& primitives ) -> intersection
{
	intersection result{ 1e9f, -1.0f };
	for( const auto& pointer : primitives )
	{
		const auto& prim = *pointer;
		float distance = prim.sdf( position );
		if( distance < result.distance ) result = { distance, prim.material_id( ) };
	}
	return	result;
}

auto raycast( vec3 ray_origin, vec3 ray_direction, const vector< unique_ptr< primitive > >& primitives ) -> intersection
{
	intersection result{ -1.0f, -1.0f };
	float distance_travelled = 0.1f;
	for( int index = 0; index < 64; ++index )
	{
		intersection current = map( ray_origin + ray_direction * distance_travelled, primitives );
		if( absolute( current.distance ) < ( 0.001f * distance_travelled ) )
		{
			result = { distance_travelled, current.material_id };
			break;
		}
		distance_travelled += current.distance;
		if( distance_travelled > 200.0f ) break;
	}
	return	result;
}

auto calc_normal( vec3 position, const vector< unique_ptr< primitive > >& primitives ) -> vec3
{
	const float epsilon = 0.001f;
	const vec3 offset_x{ epsilon, 0.0f, 0.0f };
	const vec3 offset_y{ 0.0f, epsilon, 0.0f };
	const vec3 offset_z{ 0.0f, 0.0f, epsilon };

	return	normalize( vec3{
		 map( position + offset_x, primitives ).distance - map( position - offset_x, primitives ).distance
		,map( position + offset_y, primitives ).distance - map( position - offset_y, primitives ).distance
		,map( position + offset_z, primitives ).distance - map( position - offset_z, primitives ).distance
	} );
}

}


auto main( const int /*argument_count*/, const char* /*argument_values*/[ ] ) -> int
{
	__using( ::sak::, exit_success, exit_failure )
	__using( ::sak::math::
		,sine
		,cosine
		,clamp
		,exponential
		,min
		,max
	)
	__using( ::std::, exception, println )

	try
	{
		::tui::terminal terminal;
		::tui::renderer renderer( terminal );
		::game::shadertoy shadertoy( renderer );

		vector< unique_ptr< primitive > > primitives;
		primitives.push_back( make_unique< plane >( -24.0f ) );
		primitives.push_back( make_unique< torus >( vec3{ 0.0f, 1.0f, 0.0f }, vec2{ 6.0f, 1.2f } ) );
		primitives.push_back( make_unique< sphere >( vec3{ 12.0f, 1.0f, 0.0f }, 1.6f ) );

		vec3 ray_origin, camera_forward, camera_right, camera_up;
		float angle = 0.0f;
		float radius = 15.0f;

		shadertoy.run(
			[ & ]( char code, float ) -> void
			{
				constexpr float speed = 0.08f;
				switch( code )
				{
					case 'a': angle -= speed; break;
					case 'd': angle += speed; break;
					case 'w': radius = max( 3.0f, radius - 0.5f ); break;
					case 's': radius = min( 30.0f, radius + 0.5f ); break;
				}

				ray_origin = vec3{ sine( angle ) * radius, 6.0f, cosine( angle ) * radius };
				vec3 target{ 0.0f, 0.0f, 0.0f };

				camera_forward = normalize( target - ray_origin );
				camera_right = normalize( cross( camera_forward, vec3{ 0.0f, 1.0f, 0.0f } ) ) | to;
				camera_up = cross( camera_right, camera_forward ) | to;
			},
			[ & ]( vec2 input ) -> vec3
			{
				vec3 ray_direction = normalize( camera_right * left( input ) + camera_up * top( input ) + camera_forward * 1.5f );

				vec3 color{ 0.0f, 0.0f, 0.0f };
				intersection result = raycast( ray_origin, ray_direction, primitives );

				if( result.material_id > -0.5f )
				{
					float distance = result.distance;
					vec3 position = ray_origin + ray_direction * distance;
					vec3 normal = calc_normal( position, primitives );
					vec3 light_direction = normalize( vec3{ -0.5f, 0.4f, -0.6f } );

					vec3 material_color;
					if( result.material_id < 1.5f )
						material_color = mix(
							 vec3{ 0.2f, 0.2f, 0.1f }
							,vec3{ 0.3f, 0.3f, 0.2f }
							,checker_factor( position )
						);
					else
					{
						const vec3 wave = ( vec3{ 1.0f, 1.2f, 1.5f } * result.material_id * 0.01f ) | sine | to;
						material_color = 0.6f + 0.4f * wave;
					}

					color = material_color * shade( normal, light_direction );
					color = mix( color, vec3{ 0.0f, 0.0f, 0.0f }, 1.0f - exponential( -0.0001f * distance ) );
				}

				return	clamp_vector( color, 0.0f, 1.0f );
		} );

	} catch( const exception& error )
	{
		println( stderr, "error: {}", error.what( ) );
		return	exit_failure;
	} catch( ... )
	{
		println( stderr, "error: an unknown exception occurred" );
		return	exit_failure;
	}

	return	exit_success;
}


