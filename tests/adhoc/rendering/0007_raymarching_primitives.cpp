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
//	File:   tests/adhoc/rendering/0007_raymarching_primitives.cpp
//	Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	
//	Created on 2026-01-12 16:03:05
//	


#include <exception>
#include <sak/using.hpp>
#include <sak/math/vector.hpp>
#include <game/shadertoy.hpp>
#include <tui/terminal.hpp>
#include <tui/renderer.hpp>
#include <cmath>
#include <algorithm>
#include <memory>
#include <print>
#include <vector>


namespace {


__using( ::std::
	,min
	,max
	,floor
)
__using( ::std::, unique_ptr, make_unique, vector )
__using( ::sak::, g2f, g3f )
__using( ::sak::ranges::, to )
__using( ::sak::math::, sum, sine, cosine, absolute, clamp, dot, length, normalize, cross )


using   vec2    =   g2f::point;
using   vec3    =   g3f::point;


constexpr auto clamp_vector( const auto& vector, float min_value, float max_value ) noexcept
{
	return	vector | clamp( min_value, max_value ) | to;
}

constexpr auto mix( const auto& first, const auto& second, float factor ) noexcept { return first * ( 1.0f - factor ) + second * factor; }


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
		return	point[ 1 ] - m_position[ 1 ];
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
		auto local = point - m_position;
		vec2 relative_2d_position{ length( vec2{ local } ) - m_radii[ 0 ], local[ 2 ] };
		return	length( relative_2d_position ) - m_radii[ 1 ];
	}
};


auto map( vec3 position, const vector< unique_ptr< primitive > >& primitives ) -> vec2
{
	vec2 result{ 1e9f, -1.0f };
	for( const auto& pointer : primitives )
	{
		const auto& prim = *pointer;
		float distance = prim.sdf( position );
		if( distance < result[ 0 ] ) result = { distance, prim.material_id( ) };
	}
	return	result;
}

auto raycast( vec3 ray_origin, vec3 ray_direction, const vector< unique_ptr< primitive > >& primitives ) -> vec2
{
	vec2 result{ -1.0f, -1.0f };
	float distance_travelled = 0.1f;
	for( int index = 0; index < 64; ++index )
	{
		vec2 hit = map( ray_origin + ray_direction * distance_travelled, primitives );
		if( absolute( hit[ 0 ] ) < ( 0.001f * distance_travelled ) )
		{
			result = { distance_travelled, hit[ 1 ] };
			break;
		}
		distance_travelled += hit[ 0 ];
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
		 map( position + offset_x, primitives )[ 0 ] - map( position - offset_x, primitives )[ 0 ]
		,map( position + offset_y, primitives )[ 0 ] - map( position - offset_y, primitives )[ 0 ]
		,map( position + offset_z, primitives )[ 0 ] - map( position - offset_z, primitives )[ 0 ]
	} );
}

}


auto main( const int /*argument_count*/, const char* /*argument_values*/[ ] ) -> int
{
	__using( ::sak::, exit_success, exit_failure )
	__using( ::sak::math::, sine, cosine, clamp, exponential )
	__using( ::std::, exception, floor, println )

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
				vec3 ray_direction = normalize( camera_right * input[ 0 ] + camera_up * input[ 1 ] + camera_forward * 1.5f );

			vec3 color{ 0.0f, 0.0f, 0.0f };
			vec2 result = raycast( ray_origin, ray_direction, primitives );

			if( result[ 1 ] > -0.5f )
			{
				float distance = result[ 0 ];
				vec3 position = ray_origin + ray_direction * distance;
				vec3 normal = calc_normal( position, primitives );
				vec3 light_direction = normalize( vec3{ -0.5f, 0.4f, -0.6f } );

				vec3 material_color;
				if( result[ 1 ] < 1.5f )
					material_color = mix(
						 vec3{ 0.2f, 0.2f, 0.1f }
						,vec3{ 0.3f, 0.3f, 0.2f }
						,( int( floor( position[ 0 ] / 8.0f ) + floor( position[ 2 ] / 8.0f ) ) & 1 )
					);
				else
					material_color = 0.6f + 0.4f * ( ( vec3{ 1.0f, 1.2f, 1.5f } * result[ 1 ] * 0.01f ) | sine | to );

				color = material_color * (
						clamp( dot( normal, light_direction ), 0.0f, 1.0f )
					+	( 0.3f + 0.2f * normal[ 1 ] )
					+	( 0.2f * clamp( dot( normal, -light_direction ), 0.0f, 1.0f ) )
				);
				color = mix( color, vec3{ 0.0f, 0.0f, 0.0f }, 1.0f - exponential( -0.0001f * distance ) );
			}

			return	clamp_vector( color, 0.0f, 1.0f );
		} );

	} catch( const exception& e )
	{
		println( "error: {}", e.what( ) );
		return	exit_failure;
	}

	return	exit_success;
}


