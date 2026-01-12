/*
 * Copyright (C) 2026 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/* 
 * File:   adhoc/0007_raymarching_primitives/0007_raymarching_primitives.cpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2026-01-12 16:00
 */


#include <exception>
#include <sak/using.hpp>
#include <game/shadertoy.hpp>
#include <tui/terminal.hpp>
#include <tui/renderer.hpp>
#include <cmath>
#include <algorithm>
#include <iostream>


namespace {


__using( ::std::, sqrt, min, max, pow, exp, floor )
__using( ::sak::, g2f, g3f, to_point )
__using( ::sak::math::, sum, sin, cos, abs, clamp, bind_back )


using   vec2    =   g2f::point;
using   vec3    =   g3f::point;


//	GLSL-like helpers for C++ using sak::point
constexpr auto dot( const auto& left, const auto& right ) noexcept { return sum( left * right ); }
constexpr auto length( const auto& vector ) noexcept { return vector.get_length( ); }
constexpr auto normalize( const auto& vector ) noexcept { return vector / length( vector ); }

template< typename t_point >
constexpr auto clamp_vector( const t_point& vector, float min_value, float max_value ) noexcept -> t_point
{
	return	static_cast< t_point >( vector | bind_back( clamp, min_value, max_value ) | to_point );
}

constexpr auto mix( const auto& first, const auto& second, float factor ) noexcept { return first * ( 1.0f - factor ) + second * factor; }


//	SDF Primitives
auto sd_sphere( vec3 position, float radius ) { return length( position ) - radius; }


auto sd_torus( vec3 position, vec2 torus_params )
{
	vec2 distance{ length( vec2{ position[ 0 ], position[ 1 ] } ) - torus_params[ 0 ], position[ 2 ] };
	return	length( distance ) - torus_params[ 1 ];
}

//	Simplified map for performance in TUI
auto map( vec3 position ) -> vec2
{
	//	torus (centered, doubled size, standing up)
	float distance_torus = sd_torus( position - vec3{ 0.0f, 1.0f, 0.0f }, { 6.0f, 1.2f } );
	vec2 result{ distance_torus, 200.0f };

	//	sphere (moved away by its size: torus 4.0 + sphere 2.0 + gap 2.0)
	float distance_sphere = sd_sphere( position - vec3{ 12.0f, 1.0f, 0.0f }, 1.6f );
	if( distance_sphere < result[ 0 ] ) result = { distance_sphere, 45.0f };

	return	result;
}

auto raycast( vec3 ray_origin, vec3 ray_direction ) -> vec2
{
	vec2 result{ -1.0f, -1.0f };
	float distance_travelled = 0.1f;
	for( int index = 0; index < 64; ++index )
	{
		vec2 hit = map( ray_origin + ray_direction * distance_travelled );
		if( abs( hit[ 0 ] ) < ( 0.001f * distance_travelled ) )
		{
			result = { distance_travelled, hit[ 1 ] };
			break;
		}
		distance_travelled += hit[ 0 ];
		if( distance_travelled > 400.0f ) break;
	}
	return	result;
}

auto calc_normal( vec3 position ) -> vec3
{
	const float epsilon = 0.001f;
	return	normalize( vec3{
		map( position + vec3{ epsilon, 0.0f, 0.0f } )[ 0 ] - map( position - vec3{ epsilon, 0.0f, 0.0f } )[ 0 ],
		map( position + vec3{ 0.0f, epsilon, 0.0f } )[ 0 ] - map( position - vec3{ 0.0f, epsilon, 0.0f } )[ 0 ],
		map( position + vec3{ 0.0f, 0.0f, epsilon } )[ 0 ] - map( position - vec3{ 0.0f, 0.0f, epsilon } )[ 0 ]
	} );
}

}


auto main( int, char*[ ] ) -> int
{{
	__using( ::sak::, exit_success, exit_failure, g2f, g3f, to_point )
	__using( ::sak::math::, sin, cos, clamp, exp )
	__using( ::std::, exception, cerr, endl )

	try
	{
		::tui::terminal terminal;
		::tui::renderer renderer( terminal );
		::game::shadertoy shadertoy( renderer );

		const auto size = renderer.size( );
		const float aspect = static_cast< float >( size[ 0 ] ) / static_cast< float >( size[ 1 ] );

		shadertoy.run( [ aspect ]( vec3 input ) -> vec3
		{
			const float time = input[ 2 ] * 3.0f;
			//	invert y: 1.0 - input[1]
			vec2 uv_coordinates{ ( input[ 0 ] * 2.0f - 1.0f ) * aspect, ( 1.0f - input[ 1 ] ) * 2.0f - 1.0f };

			//	camera orbit
			float angle = 0.4f * time;
			vec3 ray_origin{ 15.0f * sin( angle ), 6.0f, 15.0f * cos( angle ) };
			vec3 target{ 0.0f, 1.0f, 0.0f };
			
			//	camera matrix
			vec3 camera_forward = normalize( target - ray_origin );
			vec3 camera_up_approx{ 0.0f, 1.0f, 0.0f };
			vec3 camera_right = normalize( vec3{ camera_forward[ 1 ] * camera_up_approx[ 2 ] - camera_forward[ 2 ] * camera_up_approx[ 1 ], camera_forward[ 2 ] * camera_up_approx[ 0 ] - camera_forward[ 0 ] * camera_up_approx[ 2 ], camera_forward[ 0 ] * camera_up_approx[ 1 ] - camera_forward[ 1 ] * camera_up_approx[ 0 ] } );
			vec3 camera_up{ camera_right[ 1 ] * camera_forward[ 2 ] - camera_right[ 2 ] * camera_forward[ 1 ], camera_right[ 2 ] * camera_forward[ 0 ] - camera_right[ 0 ] * camera_forward[ 2 ], camera_right[ 0 ] * camera_forward[ 1 ] - camera_right[ 1 ] * camera_forward[ 0 ] };
			
			vec3 ray_direction = normalize( camera_right * uv_coordinates[ 0 ] + camera_up * uv_coordinates[ 1 ] + camera_forward * 1.5f );

			//	render
			vec3 color{ 0.0f, 0.0f, 0.0f }; // black background
			vec2 result = raycast( ray_origin, ray_direction );
			
			if( result[ 1 ] > -0.5f )
			{
				float distance = result[ 0 ];
				vec3 position = ray_origin + ray_direction * distance;
				vec3 normal = calc_normal( position );
				vec3 light_direction = normalize( vec3{ -0.5f, 0.4f, -0.6f } );
				
				float diffuse = clamp( dot( normal, light_direction ), 0.0f, 1.0f );
				float ambient = 0.3f + 0.2f * normal[ 1 ];
				float backlight = 0.2f * clamp( dot( normal, -light_direction ), 0.0f, 1.0f );
				
				vec3 material_color = 0.6f + 0.4f * static_cast< vec3 >( ( vec3{ result[ 1 ], result[ 1 ] * 1.2f, result[ 1 ] * 1.5f } * 0.01f ) | sin | to_point );
				
				color = material_color * ( diffuse + ambient + backlight );
				color = mix( color, vec3{ 0.0f, 0.0f, 0.0f }, 1.0f - exp( -0.0001f * distance * distance * distance ) );
			}

			return	clamp_vector( color, 0.0f, 1.0f );
		} );

	} catch( const exception& e )
	{
		cerr << "error: " << e.what( ) << endl;
		return	exit_failure;
	}

	return	exit_success;
}}


