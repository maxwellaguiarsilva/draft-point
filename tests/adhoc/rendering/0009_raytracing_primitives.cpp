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
//	File:   tests/adhoc/rendering/0009_raytracing_primitives.cpp
//	Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	
//	Created on 2026-01-12 16:03:05
//	


#include <exception>
#include <sak/using.hpp>
#include <game/shadertoy.hpp>
#include <tui/terminal.hpp>
#include <tui/renderer.hpp>
#include <cmath>
#include <algorithm>
#include <print>


namespace {

__using( ::std::
	,min
	,max
	,floor
)
__using( ::sak::, g2f, g3f )
__using( ::sak::ranges::, to )
__using( ::sak::math::
	,sum
	,sine
	,cosine
	,absolute
	,clamp
	,square_root
	,dot
	,length
	,normalize
	,cross
)

using   vec2    =   g2f::point;
using   vec3    =   g3f::point;

//	glsl-like helper for c++ using sak::point
constexpr auto clamp_vector( const auto& vector, float min_value, float max_value ) noexcept
{
	return	vector | clamp( min_value, max_value ) | to;
}

constexpr auto mix( const auto& first, const auto& second, float factor ) noexcept { return first * ( 1.0f - factor ) + second * factor; }

//	scene object material ids
constexpr float material_floor = 1.0f;
constexpr float material_sphere = 45.0f;

//	scene layout
constexpr float floor_height = -24.0f;
constexpr vec3 sphere_position{ 0.0f, 1.0f, 0.0f };
constexpr float sphere_radius = 6.0f;

//	analytical ray-sphere intersection
//	returns the distance to the intersection, or -1.0 if no hit
auto intersect_sphere( vec3 ray_origin, vec3 ray_direction, vec3 center, float radius ) -> float
{
	vec3 oc = ray_origin - center;
	float a = dot( ray_direction, ray_direction );	//	always 1.0 if the direction is normalized
	float b = 2.0f * dot( oc, ray_direction );
	float c = dot( oc, oc ) - radius * radius;
	float discriminant = b * b - 4.0f * a * c;

	if( discriminant < 0.0f )
		return	-1.0f;	//	did not hit the sphere

	//	calculates the smallest positive root (the sphere entry point)
	float t = ( -b - square_root( discriminant ) ) / ( 2.0f * a );
	if( t > 0.0f )
		return	t;

	return	-1.0f;
}

//	analytical ray-plane intersection
auto intersect_plane( vec3 ray_origin, vec3 ray_direction, float height ) -> float
{
	//	if the ray is parallel to the floor, no hit
	if( absolute( ray_direction[ 1 ] ) < 0.0001f )
		return	-1.0f;

	//	t = (plane_height - origin.y) / direction.y
	float t = ( height - ray_origin[ 1 ] ) / ray_direction[ 1 ];
	if( t > 0.0f )
		return	t;

	return	-1.0f;
}

//	analytical raycast: tests intersection with all objects and returns the closest one
auto raycast( vec3 ray_origin, vec3 ray_direction ) -> vec2
{
	vec2 result{ 99999.0f, -1.0f };	//	{ distance, material_id }

	//	test the floor
	float t_plane = intersect_plane( ray_origin, ray_direction, floor_height );
	if( t_plane > 0.0f and t_plane < result[ 0 ] )
		result = { t_plane, material_floor };

	//	test the sphere
	float t_sphere = intersect_sphere( ray_origin, ray_direction, sphere_position, sphere_radius );
	if( t_sphere > 0.0f and t_sphere < result[ 0 ] )
		result = { t_sphere, material_sphere };

	//	if nothing was hit, return -1
	if( result[ 1 ] < 0.0f )
		return	{ -1.0f, -1.0f };

	return	result;
}

//	calculates the normal analytically (without needing sdf)
auto calc_normal( vec3 position, float material_id ) -> vec3
{
	if( material_id == material_floor )
		return	vec3{ 0.0f, 1.0f, 0.0f };	//	floor normal points up
	else if( material_id == material_sphere )
		return	normalize( position - sphere_position );	//	sphere normal is from center to point
	return	vec3{ 0.0f, 1.0f, 0.0f };
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

		vec3 ray_origin, camera_forward, camera_right, camera_up;

		shadertoy.run(
			[ & ]( char, float time ) -> void
			{
				float angle = 0.4f * time * 3.0f;
				ray_origin = vec3{ sine( -angle ), 0.0f, cosine( -angle ) } * 15.0f + vec3{ 0.0f, 6.0f, 0.0f };
				vec3 target{ 0.0f, 0.0f, 0.0f };

				camera_forward = normalize( target - ray_origin );
				camera_right = normalize( cross( camera_forward, vec3{ 0.0f, 1.0f, 0.0f } ) ) | to;
				camera_up = cross( camera_right, camera_forward ) | to;
			},
			[ & ]( vec2 input ) -> vec3
			{
				vec3 ray_direction = normalize( camera_right * input[ 0 ] + camera_up * input[ 1 ] + camera_forward * 1.5f );

			//	render
			vec3 color{ 0.0f, 0.0f, 0.0f };	//	black background
			vec2 result = raycast( ray_origin, ray_direction );
			
			if( result[ 1 ] > -0.5f )
			{
				float distance = result[ 0 ];
				vec3 position = ray_origin + ray_direction * distance;
				vec3 normal = calc_normal( position, result[ 1 ] );
				vec3 light_direction = normalize( vec3{ -0.5f, 0.4f, -0.6f } );
				
				float diffuse = clamp( dot( normal, light_direction ), 0.0f, 1.0f );
				float ambient = 0.3f + 0.2f * normal[ 1 ];
				float backlight = 0.2f * clamp( dot( normal, -light_direction ), 0.0f, 1.0f );
				
				vec3 material_color;
				if( result[ 1 ] < 1.5f )
				{
					int x = floor( position[ 0 ] / 8.0f );
					int z = floor( position[ 2 ] / 8.0f );
					float f = ( x + z ) & 1;
					material_color = mix( vec3{ 0.2f, 0.2f, 0.1f }, vec3{ 0.3f, 0.3f, 0.2f }, f );
				}
				else
					material_color = vec3{ 0.8f, 0.2f, 0.2f };	//	red sphere
				
				color = material_color * ( diffuse + ambient + backlight );
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


