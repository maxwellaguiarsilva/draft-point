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
//	File:   source/draft_point.cpp
//	Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	
//	Created on 2026-01-12 16:03:05
//	


#include <climits>
#include <cstdlib>
#include <exception>
#include <limits>
#include <print>
#include <ranges>
#include <vector>
#include <sak/sak.hpp>
#include <sak/math/math.hpp>
#include <sak/ranges/count_to.hpp>
#include <sak/ranges/transform.hpp>
#include <sak/pattern/value_or.hpp>
#include <sak/pattern/to_number.hpp>
#include <tui/renderer.hpp>
#include <tui/terminal.hpp>
#include <game/shadertoy.hpp>


namespace {

__using( ::sak::, byte, g2f, g3f )
using	::sak::is_point;
__using( ::sak::math::, sum, square, square_root, dot, length, normalize, cross, rotate, sine, cosine )
__using( ::sak::ranges::, count_to, lazy_transform, to )
__using( ::std::views::, zip )

using   vec2	=   g2f::point;
using   vec3	=   g3f::point;


//	generic axis triple shared by movement, rotation, and screen space
enum class axis
{
	 axis_x	=	0
	,axis_y
	,axis_z
};


class sphere {
	vec3	m_center;
	float	m_radius;
	float	m_radius_squared;
	vec3	m_color;
public:
	sphere( vec3 center, float radius, vec3 color )
		: m_center( center ), m_radius( radius ), m_radius_squared( square( radius ) ), m_color( color ) { }

	auto center( ) const noexcept -> const vec3& { return m_center; }
	auto center( const vec3& value ) noexcept { m_center = value; }
	auto radius( ) const noexcept { return m_radius; }
	auto radius( float value ) noexcept { m_radius = value; m_radius_squared = square( value ); }
	auto radius_squared( ) const noexcept { return m_radius_squared; }
	auto color( ) const noexcept -> const vec3& { return m_color; }
};


class camera
{
	vec3	m_position;
	vec3	m_rotation;
	vec3	m_forward;
	vec3	m_right;
	vec3	m_up;
public:
	explicit camera( vec3 position, vec3 rotation = { } )
		: m_position( position ), m_rotation( rotation ) { recompute_basis( ); }

	auto position( ) const noexcept -> const vec3& { return m_position; }
	auto position( const vec3& value ) noexcept { m_position = value; }
	[ [ maybe_unused ] ] auto rotation( ) const noexcept -> const vec3& { return m_rotation; }
	[ [ maybe_unused ] ] auto rotation( const vec3& value ) noexcept { m_rotation = value; recompute_basis( ); }
	auto forward( ) const noexcept -> const vec3& { return m_forward; }
	auto right( ) const noexcept -> const vec3& { return m_right; }
	auto up( ) const noexcept -> const vec3& { return m_up; }
	auto move( const axis axis_value, const float value ) noexcept
	{
		m_position += direction( axis_value ) * value;
	}
	auto turn( const axis axis_value, const float value ) noexcept
	{
		m_rotation[ static_cast< size_t >( axis_value ) ] += value;
		recompute_basis( );
	}
private:
	auto direction( const axis axis_value ) const noexcept -> const vec3&
	{
		switch( axis_value )
		{
			case	axis::axis_x:		return	m_right;
			case	axis::axis_y:		return	m_up;
			case	axis::axis_z:		return	m_forward;
			default:					return	m_right;
		}
	}
	auto recompute_basis( ) noexcept -> void
	{
		if( length( m_rotation ) <= 0.0f )
		{
			m_forward = vec3{ 0.0f, 0.0f, 1.0f };
			m_right = vec3{ 1.0f, 0.0f, 0.0f };
			m_up = vec3{ 0.0f, 1.0f, 0.0f };
			return;
		}
		const float tilt_angle = length( m_rotation );
		const vec3 tilt_axis = normalize( m_rotation );
		m_forward = rotate( vec3{ 0.0f, 0.0f, 1.0f }, tilt_axis, tilt_angle ) | to;
		m_right = rotate( vec3{ 1.0f, 0.0f, 0.0f }, tilt_axis, tilt_angle ) | to;
		m_up = rotate( vec3{ 0.0f, 1.0f, 0.0f }, tilt_axis, tilt_angle ) | to;
	}
};


class orbit
{
	vec3	m_center;
	vec3	m_rotation;
	float	m_radius;
	float	m_speed;
	byte	m_total;
	float	m_angle;
	byte	m_pulsation_cycle;
public:
	orbit( vec3 center, float radius, float speed, byte total, vec3 rotation = { }, byte pulsation_cycle = 2 )
		: m_center( center ), m_rotation( rotation ), m_radius( radius ), m_speed( speed )
		, m_total( total ), m_angle( 0.0f ), m_pulsation_cycle( pulsation_cycle ) { }

	auto center( ) const noexcept -> const vec3& { return m_center; }
	auto center( const vec3& value ) noexcept { m_center = value; }
	[ [ maybe_unused ] ] auto rotation( ) const noexcept -> const vec3& { return m_rotation; }
	[ [ maybe_unused ] ] auto rotation( const vec3& value ) noexcept { m_rotation = value; }
	auto radius( ) const noexcept { return m_radius; }
	auto radius( float value ) noexcept { m_radius = value; }
	auto speed( ) const noexcept { return m_speed; }
	auto speed( float value ) noexcept { m_speed = value; }
	[ [ maybe_unused ] ] auto total( ) const noexcept { return m_total; }
	[ [ maybe_unused ] ] auto total( byte value ) noexcept { m_total = value; }
	auto angle( ) const noexcept { return m_angle; }
	auto angle( float value ) noexcept { m_angle = value; }
	[ [ maybe_unused ] ] auto pulsation_cycle( ) const noexcept { return m_pulsation_cycle; }
	[ [ maybe_unused ] ] auto pulsation_cycle( byte value ) noexcept { m_pulsation_cycle = value; }
	auto move( const axis axis_value, const float value ) noexcept
	{
		m_center[ static_cast< size_t >( axis_value ) ] += value;
	}
	auto turn( const axis axis_value, const float value ) noexcept
	{
		m_rotation[ static_cast< size_t >( axis_value ) ] += value;
	}
	auto element_position( const byte index ) const noexcept -> vec3
	{
		const float step = 2.0f * 3.14159265f / m_total;
		const float element_angle = m_angle + index * step;
		vec3 position{ cosine( element_angle ), sine( element_angle ), sine( element_angle ) };
		const float tilt_angle = length( m_rotation );
		if( tilt_angle > 0.0f )
			position = rotate( position, normalize( m_rotation ), tilt_angle ) | to;
		return	m_center + position * m_radius;
	}
	auto element_pulsation( const byte index ) const noexcept -> float
	{
		const float step = 2.0f * 3.14159265f / m_total;
		const float phase = ( index % m_pulsation_cycle == 0 ) ? 1.0f : -1.0f;
		return	0.5f * ( 1.0f + phase * cosine( ( m_angle + index * step ) * m_total ) );
	}
};


//	spatial subdivision sequence: extremes (0, 1) first, then recursive midpoints
//	via the base-2 radical inverse (van der corput) of (index - 1)
constexpr auto subdivision_value( byte index ) -> float
{
	if( index == 0 )
		return	0.0f;
	if( index == 1 )
		return	1.0f;
	float result = 0.0f;
	float weight = 0.5f;
	for( auto remaining = index - 1; remaining > 0; remaining >>= 1 )
	{
		result += weight * ( remaining & 1 );
		weight *= 0.5f;
	}
	return	result;
}


//	deinterleave bits from index for the given dimension (morton/z-order curve)
//	dimension d collects bits at positions d, d+n, d+2n, ... where n is the dimension count
constexpr auto deinterleave( byte index, size_t dimension, size_t dimensions ) -> byte
{
	byte channel_index = 0;
	for( size_t source_bit = dimension, dest_bit = 0; source_bit < sizeof( byte ) * CHAR_BIT; source_bit += dimensions, ++dest_bit )
		channel_index |= ( ( index >> source_bit ) & 1 ) << dest_bit;
	return	channel_index;
}


//	generate a single color from a 1d index by deinterleaving bits across dimensions
//	and mapping each channel through the spatial subdivision sequence
template< is_point t_point = vec3 >
auto create_color( byte index, float min_value = 0.0f, float max_value = 1.0f ) -> t_point
{
	constexpr size_t dimensions = t_point{ }.size( );
	const float range = max_value - min_value;

	auto calc_channel = [ = ]( size_t dimension ) -> float
	{
		const byte channel_index = deinterleave( index, dimension, dimensions );
		return	min_value + subdivision_value( channel_index ) * range;
	};

	return	count_to( dimensions ) | lazy_transform( calc_channel ) | to;
}


}


auto main( const int argument_count, const char* argument_values[ ] ) -> int
{
	__using( ::std::
		,bind_back
		,exception
		,println
		,numeric_limits
		,string
		,vector
	)
	__using( ::sak::, exit_success, exit_failure )
	__using( ::sak::pattern::, value_or, to_number )
	__using( ::sak::math::, sine, cosine, between )

	try
	{
		::tui::terminal terminal;
		::tui::renderer renderer( terminal );
		::game::shadertoy shadertoy( renderer );

		const vector< string > arguments( argument_values, argument_values + argument_count );
		const string total_default{ "8" };
		const int parsed = to_number( value_or( arguments, 1uz, total_default ), 0 );
		const byte total = between( parsed, 1, 255 ) ? parsed : 8;

		const vector< vec3 > colors_list = count_to( total ) | lazy_transform( bind_back( create_color< >, 0.5f, 1.0f ) ) | to;

		const vec3 zero_center{ 0.0f, 0.0f, 0.0f };
		const float sphere_radius = total / 2;
		auto make_sphere = [ & ]( size_t index ) { return sphere{ zero_center, sphere_radius, colors_list[ index ] }; };
		vector< sphere > objects = count_to( total ) | lazy_transform( make_sphere ) | to;

		const float	edge_shade = 0.7f;
		camera	cam{ vec3{ 0.0f, 0.0f, -2.0f } };
		const vec3	background_color{ 0.0f, 0.0f, 0.0f };
		orbit	orb{ vec3{ 0.0f, total / 2, total * 4 }, 1.5f * total, 0.6f, total };

		shadertoy.run( 
			[ & ]( char code, float time ) -> void
			{
				constexpr float rotation_step = 0.1f;
				constexpr float move_step = 0.5f;
				switch( code )
				{
					//	camera, the inverted pattern between lowercase and uppercase is intentional
					case 'W': cam.turn( axis::axis_x, -rotation_step ); break;
					case 'S': cam.turn( axis::axis_x, +rotation_step ); break;
					case 'a': cam.turn( axis::axis_y, -rotation_step ); break;
					case 'd': cam.turn( axis::axis_y, +rotation_step ); break;
					case 'Z': cam.turn( axis::axis_z, -rotation_step ); break;
					case 'X': cam.turn( axis::axis_z, +rotation_step ); break;
					case 'A': cam.move( axis::axis_x, -move_step ); break;
					case 'D': cam.move( axis::axis_x, +move_step ); break;
					case 'z': cam.move( axis::axis_y, -move_step ); break;
					case 'x': cam.move( axis::axis_y, +move_step ); break;
					case 's': cam.move( axis::axis_z, -move_step ); break;
					case 'w': cam.move( axis::axis_z, +move_step ); break;
					//	orbit
					case 'J': orb.turn( axis::axis_x, -rotation_step ); break;
					case 'L': orb.turn( axis::axis_x, +rotation_step ); break;
					case 'K': orb.turn( axis::axis_y, -rotation_step ); break;
					case 'I': orb.turn( axis::axis_y, +rotation_step ); break;
					case 'O': orb.turn( axis::axis_z, -rotation_step ); break;
					case 'P': orb.turn( axis::axis_z, +rotation_step ); break;
					case 'k': orb.move( axis::axis_y, -move_step ); break;
					case 'i': orb.move( axis::axis_y, +move_step ); break;
					case 'j': orb.move( axis::axis_x, -move_step ); break;
					case 'l': orb.move( axis::axis_x, +move_step ); break;
					case 'o': orb.move( axis::axis_z, -move_step ); break;
					case 'p': orb.move( axis::axis_z, +move_step ); break;
				}

				orb.angle( time * orb.speed( ) );
				for( auto [ index, object ] : zip( count_to( total ), objects ) )
				{
					const byte element_index = static_cast< byte >( index );
					object.center( orb.element_position( element_index ) );
					const float pulsation = orb.element_pulsation( element_index );
					object.radius( sphere_radius + ( sphere_radius / 4 ) * pulsation );
				}
			},
			[ & ]( vec2 input ) -> vec3
			{
				constexpr float focal = 2.0f;
				const vec3 direction = cam.forward( ) * focal
					+	cam.right( ) * input[ static_cast< size_t >( axis::axis_x ) ]
					+	cam.up( ) * input[ static_cast< size_t >( axis::axis_y ) ];

				float best_distance = numeric_limits< float >::infinity( );
				vec3 color = background_color;

				for( const auto& object : objects )
				{
					const vec3 hypotenuse = object.center( ) - cam.position( );
					const float opposite_leg_squared = dot( hypotenuse ) - square( dot( hypotenuse, direction ) ) / dot( direction );
					const float ratio = opposite_leg_squared / object.radius_squared( );

					const float distance = length( hypotenuse );

					if( distance < best_distance and ratio <= 1.0f )
					{
						best_distance = distance;
						color = object.color( ) * ( 1.0f - edge_shade * ratio );
					}
				}

				return	color;

			}
		 );

	}
	catch( const exception& error )
	{
		println( "error: {}", error.what( ) );
		return	exit_failure;
	}

	return	exit_success;
}


