//	
//	SPDX-FileCopyrightText: 2026 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	SPDX-License-Identifier: GPL-3.0-or-later
//	


#include <game/shadertoy.hpp>


namespace {

__using( ::sak::, g2f, g3f )
__using( ::sak::ranges::, to )
__using( ::sak::math::
	,sine
	,cosine
	,absolute
	,clamp
	,round_down
	,square_root
	,dot
	,length
	,normalize
	,cross
)
__using_static( g2f::, left, top )


using	vec2	=	g2f::point;
using	vec3	=	g3f::point;

//	glsl-like helper for c++ using sak::point
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
	__using_constexpr( g3f::, top )

	//	if the ray is parallel to the floor, no hit
	if( absolute( top( ray_direction ) ) < 0.0001f )
		return	-1.0f;

	//	t = (plane_height - origin.y) / direction.y
	float t = ( height - top( ray_origin ) ) / top( ray_direction );
	if( t > 0.0f )
		return	t;

	return	-1.0f;
}

//	analytical raycast: tests intersection with all objects and returns the closest one
auto raycast( vec3 ray_origin, vec3 ray_direction ) -> intersection
{
	intersection result{ 99999.0f, -1.0f };

	//	test the floor
	float t_plane = intersect_plane( ray_origin, ray_direction, floor_height );
	if( t_plane > 0.0f and t_plane < result.distance )
		result = { t_plane, material_floor };

	//	test the sphere
	float t_sphere = intersect_sphere( ray_origin, ray_direction, sphere_position, sphere_radius );
	if( t_sphere > 0.0f and t_sphere < result.distance )
		result = { t_sphere, material_sphere };

	//	if nothing was hit, return -1
	if( result.material_id < 0.0f )
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
	__using( ::std::, exception, println )

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
				vec3 ray_direction = normalize( camera_right * left( input ) + camera_up * top( input ) + camera_forward * 1.5f );

				//	render
				vec3 color{ 0.0f, 0.0f, 0.0f };	//	black background
				intersection result = raycast( ray_origin, ray_direction );

				if( result.material_id > -0.5f )
				{
					float distance = result.distance;
					vec3 position = ray_origin + ray_direction * distance;
					vec3 normal = calc_normal( position, result.material_id );
					vec3 light_direction = normalize( vec3{ -0.5f, 0.4f, -0.6f } );

					vec3 material_color;
					if( result.material_id < 1.5f )
						material_color = mix(
							 vec3{ 0.2f, 0.2f, 0.1f }
							,vec3{ 0.3f, 0.3f, 0.2f }
							,checker_factor( position )
						);
					else
						material_color = vec3{ 0.8f, 0.2f, 0.2f };	//	red sphere

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


