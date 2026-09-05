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
//	File:   tests/sak/test_sak_math.cpp
//	Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	
//	Created on 2026-08-02 12:16:47
//	


#include <array>
#include <exception>
#include <print>
#include <ranges>
#include <string>
#include <typeinfo>
#include <vector>
#include <sak/ensure.hpp>
#include <sak/using.hpp>
#include <sak/math/math.hpp>
#include <sak/math/vector.hpp>
#include <sak/geometry/point.hpp>
#include <sak/ranges/to.hpp>
#include <sak/ranges/transform.hpp>


auto main( const int argument_count, const char* argument_values[ ] ) -> int
{
	__using( ::sak::
		,exit_success
		,exit_failure
		,ensure
		,point
	)
	__using( ::std::
		,array
		,exception
		,println
		,string
		,vector
	)
	__using( ::sak::math::
		,absolute
		,cross
		,is_triplet
		,length
		,normalize
		,rotate
		,sum
	)
	__using( ::sak::ranges::
		,operator+
		,operator-
		,operator*
		,operator/
		,to
		,lazy_transform
	)

	const vector< string > arguments( argument_values, argument_values + argument_count );
	for( const auto& value : arguments )
		println( "{}", value );
	
	try
	{
		println( "starting tests for: sak/math" );

		//	--------------------------------------------------
		//	sum
		//	--------------------------------------------------

		//	integers
		{
			vector< int > values = { 10, 20, 30 };
			ensure( sum( values ) == 60, "test 1 ( integers ) failed" );
			println( "test 1 ( integers ) passed" );
		}

		//	floating point
		{
			vector< double > values = { 1.5, 1.5 };
			auto result = sum( values );
			println( "sum of {{ 1.5, 1.5 }} = {}", result );
			println( "type of result: {}", typeid( result ).name( ) );
			ensure( result == 3.0, "test 2 ( floating point 1.5+1.5 ) failed" );
			println( "test 2 ( floating point 1.5+1.5 ) passed" );

			vector< double > values2 = { 0.9, 0.9 };
			auto result2 = sum( values2 );
			println( "sum of {{ 0.9, 0.9 }} = {}", result2 );
			println( "type of result2: {}", typeid( result2 ).name( ) );
			ensure( result2 == 1.8, "test 3 ( floating point 0.9+0.9 ) failed" );
			println( "test 3 ( floating point 0.9+0.9 ) passed" );

			vector< double > values3( 100, 0.9 );
			auto result3 = sum( values3 );
			println( "sum of 100 * 0.9 = {}", result3 );
			ensure( result3 >= 89.0, "test 4 ( 100 * 0.9 ) failed" );
			println( "test 4 ( 100 * 0.9 ) passed" );
		}

		//	--------------------------------------------------
		//	vector
		//	--------------------------------------------------

		//	cross product: right-hand rule on unit axes
		{
			const point< float, 3 > axis_x{ 1.0f, 0.0f, 0.0f };
			const point< float, 3 > axis_y{ 0.0f, 1.0f, 0.0f };
			const point< float, 3 > axis_z{ 0.0f, 0.0f, 1.0f };

			const point< float, 3 > x_cross_y = cross( axis_x, axis_y ) | to;
			ensure( x_cross_y == axis_z, "cross of x and y must be z" );

			const point< float, 3 > y_cross_x = cross( axis_y, axis_x ) | to;
			ensure( y_cross_x == -axis_z, "cross must be anti-commutative" );

			const point< float, 3 > z_cross_x = cross( axis_z, axis_x ) | to;
			ensure( z_cross_x == axis_y, "cross of z and x must be y" );
		}

		//	cross: lazy result keeps composing with normalize and length
		{
			const point< float, 3 > axis_x{ 1.0f, 0.0f, 0.0f };
			const point< float, 3 > axis_y{ 0.0f, 1.0f, 0.0f };

			const point< float, 3 > unit = normalize( cross( axis_x, axis_y ) ) | to;
			ensure( unit == point< float, 3 >{ 0.0f, 0.0f, 1.0f }, "normalize of lazy cross failed" );

			const auto magnitude = length( cross( axis_x, axis_y ) );
			ensure( magnitude == 1.0f, "length of lazy cross failed" );
		}

		//	cross: works on any 3-element range (array, view)
		{
			const array< float, 3 > left{ 2.0f, 0.0f, 0.0f };
			const point< float, 3 > right{ 0.0f, 3.0f, 0.0f };

			const array< float, 3 > product = cross( left, right ) | to;
			ensure( product[ 0 ] == 0.0f and product[ 1 ] == 0.0f and product[ 2 ] == 6.0f, "cross over heterogeneous ranges failed" );

			const auto boosted = left | lazy_transform( [ ]( float value ) { return value * 2.0f; } );
			const point< float, 3 > from_view = cross( boosted, right ) | to;
			ensure( from_view == point< float, 3 >{ 0.0f, 0.0f, 12.0f }, "cross over a view failed" );
		}

		//	length: 3-4-5 triangle, on point and on lazy view
		{
			const point< float, 2 > triangle{ 3.0f, 4.0f };
			ensure( length( triangle ) == 5.0f, "length of 3-4-5 triangle failed" );

			const point< float, 3 > spatial{ 2.0f, 3.0f, 6.0f };
			ensure( length( spatial ) == 7.0f, "length of 3d vector failed" );

			const auto scaled = triangle | lazy_transform( [ ]( float value ) { return value * 2.0f; } );
			ensure( length( scaled ) == 10.0f, "length of lazy view failed" );

			const point< int, 2 > integral{ 1, 2 };
			ensure( length( integral ) > 2.2f and length( integral ) < 2.3f, "length must not truncate integral scalars" );
		}

		//	normalize: unit length and preserved direction
		{
			const point< float, 3 > direction{ 0.0f, 3.0f, 4.0f };
			const point< float, 3 > unit = normalize( direction ) | to;
			ensure( unit == point< float, 3 >{ 0.0f, 0.6f, 0.8f }, "normalize direction failed" );
			ensure( length( unit ) == 1.0f, "normalize must produce unit length" );

			const point< float, 3 > lazy_unit = normalize( direction | lazy_transform( [ ]( float value ) { return value * 2.0f; } ) ) | to;
			ensure( lazy_unit == unit, "normalize over a lazy view failed" );
		}

		//	is_triplet: rejects non-3-dimensional ranges at compile time
		{
			static_assert( is_triplet< point< float, 3 > >, "3d point must be a triplet" );
			static_assert( is_triplet< array< int, 3 > >, "3-element array must be a triplet" );
			static_assert( not is_triplet< point< float, 2 > >, "2d point must not be a triplet" );
			static_assert( not is_triplet< array< int, 4 > >, "4-element array must not be a triplet" );
		}

		//	rotate: rodrigues formula around the axis, lazy result
		{
			const point< float, 3 > axis_x{ 1.0f, 0.0f, 0.0f };
			const point< float, 3 > axis_y{ 0.0f, 1.0f, 0.0f };
			const point< float, 3 > axis_z{ 0.0f, 0.0f, 1.0f };
			constexpr float half_pi = 1.5707963268f;

			const point< float, 3 > quarter = rotate( axis_x, axis_z, half_pi ) | to;
			ensure( length( quarter - axis_y ) < 0.001f, "rotate around z by 90 degrees failed" );

			const point< float, 3 > back = rotate( quarter, axis_z, -half_pi ) | to;
			ensure( length( back - axis_x ) < 0.001f, "rotate inverse must restore the vector" );

			const point< float, 3 > source{ 0.0f, 3.0f, 4.0f };
			const point< float, 3 > tilted = rotate( source, axis_x, 0.5f ) | to;
			const float preserved = length( tilted );
			ensure( absolute( preserved - length( source ) ) < 0.001f, "rotate must preserve vector length" );

			const point< float, 3 > rotated_view = rotate( source | lazy_transform( [ ]( float value ) { return value * 1.0f; } ), axis_y, 0.3f ) | to;
			const point< float, 3 > reference = rotate( source, axis_y, 0.3f ) | to;
			ensure( length( rotated_view - reference ) < 0.001f, "rotate over a lazy view failed" );
		}

		//	direct-application pipe: v | normalize applies to the whole vector, no transform
		{
			const point< float, 3 > direction{ 0.0f, 3.0f, 4.0f };
			const point< float, 3 > piped_unit = direction | normalize | to;
			ensure( piped_unit == point< float, 3 >{ 0.0f, 0.6f, 0.8f }, "v | normalize failed" );
			ensure( length( piped_unit ) == 1.0f, "v | normalize must produce unit length" );

			const point< float, 3 > axis_x{ 1.0f, 0.0f, 0.0f };
			const point< float, 3 > axis_y{ 0.0f, 1.0f, 0.0f };
			const point< float, 3 > axis_z{ 0.0f, 0.0f, 1.0f };
			const point< float, 3 > piped_cross = axis_x | cross( axis_y ) | to;
			ensure( piped_cross == axis_z, "v | cross( w ) failed" );

			constexpr float half_pi = 1.5707963268f;
			const point< float, 3 > piped_rotated = axis_x | rotate( axis_z, half_pi ) | to;
			ensure( length( piped_rotated - axis_y ) < 0.001f, "v | rotate( axis, angle ) failed" );

			//	const point< float, 3 > chained = direction | normalize | rotate( axis_z, half_pi ) | to;
			//	ensure( length( chained ) == 1.0f, "chained direct-application pipe failed" );
		}

		println( "all tests for sak/math passed" );
	}
	catch( const exception& error )
	{
		println( "test failed: {}", error.what( ) );
		return	exit_failure;
	}

	return	exit_success;
}


