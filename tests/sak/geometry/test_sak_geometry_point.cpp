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
 * File:   sak/geometry/test_sak_geometry_point.cpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2026-01-01 23:05
 */


#include <print>
#include <string>
#include <vector>
#include <exception>
#include <sak/ensure.hpp>
#include <sak/using.hpp>
#include <sak/geometry/point.hpp>


auto main( const int argument_count, const char* argument_values[ ] ) -> int
{{
	__using( ::sak::
		,exit_success
		,exit_failure
		,ensure
	)
	__using( ::std::
		,string
		,vector
		,println
		,exception
	)

	const vector< string > arguments( argument_values, argument_values + argument_count );
	for( const auto& value : arguments )
		println( "{}", value );
	
	try
	{
		println( "starting tests for: sak/geometry/point..." );

		using	::sak::point;

		//	test 1: default construction
		{
			const point p;
			ensure( p[ 0 ] == 0 and p[ 1 ] == 0, "default construction failed" );
		}

		//	test 2: argument construction
		{
			const point p( 10, 20 );
			ensure( p[ 0 ] == 10 and p[ 1 ] == 20, "argument construction failed" );
		}

		//	test 3: addition
		{
			point p1( 1, 2 );
			const point p2( 3, 4 );
			p1 += p2;
			ensure( p1[ 0 ] == 4 and p1[ 1 ] == 6, "operator += failed" );

			const auto p3 = p1 + point( 10, 10 );
			ensure( p3[ 0 ] == 14 and p3[ 1 ] == 16, "operator + failed" );

			const auto p4 = 10 + p1;
			ensure( p4[ 0 ] == 14 and p4[ 1 ] == 16, "operator + (scalar left) failed" );

			const auto p5 = p1 + 10;
			ensure( p5[ 0 ] == 14 and p5[ 1 ] == 16, "operator + (scalar right) failed" );
		}

		//	test 4: subtraction
		{
			point p1( 10, 20 );
			const point p2( 3, 4 );
			p1 -= p2;
			ensure( p1[ 0 ] == 7 and p1[ 1 ] == 16, "operator -= failed" );

			const auto p3 = p1 - point( 2, 6 );
			ensure( p3[ 0 ] == 5 and p3[ 1 ] == 10, "operator - failed" );
		}

		//	test 5: multiplication
		{
			point p1( 2, 3 );
			p1 *= 2;
			ensure( p1[ 0 ] == 4 and p1[ 1 ] == 6, "operator *= (scalar) failed" );

			const point p2( 2, 2 );
			p1 *= p2;
			ensure( p1[ 0 ] == 8 and p1[ 1 ] == 12, "operator *= (point) failed" );
		}

		//	test 6: division
		{
			point p1( 10, 20 );
			p1 /= 2;
			ensure( p1[ 0 ] == 5 and p1[ 1 ] == 10, "operator /= failed" );
		}

		//	test 7: is_inside
		{
			const point p1( 10, 10 );
			const point p2( 5, 5 );
			const point p3( 15, 15 );

			ensure( p2.is_inside( p1 ), "p2 should be inside p1" );
			ensure( not p3.is_inside( p1 ), "p3 should not be inside p1" );
			ensure( p1.is_inside( p1 ), "p1 should be inside itself" );
		}

		//	test 8: get_length
		{
			const point p1( 3, 4 );
			//	sqrt( 3*3 + 4*4 ) = sqrt( 9 + 16 ) = sqrt( 25 ) = 5
			ensure( p1.get_length( ) == 5, "get_length failed" );

			const point< float, 3 > p2( 2.0f, 3.0f, 6.0f );
			//	sqrt( 2*2 + 3*3 + 6*6 ) = sqrt( 4 + 9 + 36 ) = sqrt( 49 ) = 7
			ensure( p2.get_length( ) == 7.0f, "get_length for 3d float point failed" );
		}

		//	test 9: 3d point
		{
			const point< int, 3 > p( 1, 2, 3 );
			ensure( p[ 0 ] == 1 and p[ 1 ] == 2 and p[ 2 ] == 3, "3d point failed" );
			ensure( p.get_length( ) == 3, "get_length for 3d int point failed (truncated)" );
		}

		println( "all tests for sak/geometry/point passed!" );
	}
	catch( const exception& error )
	{
		println( "test failed: {}", error.what( ) );
		return	exit_failure;
	}

	return	exit_success;
}}


