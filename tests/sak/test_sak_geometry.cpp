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
//	File:   tests/sak/test_sak_geometry.cpp
//	Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	
//	Created on 2026-08-02 12:15:30
//	


#include <array>
#include <exception>
#include <print>
#include <ranges>
#include <string>
#include <vector>
#include <sak/ensure.hpp>
#include <sak/using.hpp>
#include <sak/geometry/geometry.hpp>
#include <sak/geometry/line_view.hpp>
#include <sak/geometry/point.hpp>
#include <sak/ranges/to.hpp>


auto main( const int argument_count, const char* argument_values[ ] ) -> int
{
	__using( ::sak::
		,exit_success
		,exit_failure
		,ensure
		,line_to
		,point
	)
	__using( ::sak::ranges::, to )
	__using( ::std::
		,exception
		,println
		,string
		,vector
	)
	__using( ::std::ranges::, equal )

	const vector< string > arguments( argument_values, argument_values + argument_count );
	for( const auto& value : arguments )
		println( "{}", value );
	
	try
	{
		println( "starting tests for: sak/geometry" );

		//	--------------------------------------------------
		//	point
		//	--------------------------------------------------

		//	default construction
		{
			const point p;
			ensure( p[ 0 ] == 0 and p[ 1 ] == 0, "default construction failed" );
		}

		//	argument construction
		{
			const point p( 10, 20 );
			ensure( p[ 0 ] == 10 and p[ 1 ] == 20, "argument construction failed" );
		}

		//	addition
		{
			point p1( 1, 2 );
			const point p2( 3, 4 );
			p1 += p2;
			ensure( p1[ 0 ] == 4 and p1[ 1 ] == 6, "operator += failed" );

			const auto p3 = p1 + point( 10, 10 );
			ensure( p3[ 0 ] == 14 and p3[ 1 ] == 16, "operator + failed" );

			const auto p4 = 10 + p1;
			ensure( p4[ 0 ] == 14 and p4[ 1 ] == 16, "operator + ( scalar left ) failed" );

			const auto p5 = p1 + 10;
			ensure( p5[ 0 ] == 14 and p5[ 1 ] == 16, "operator + ( scalar right ) failed" );
		}

		//	subtraction
		{
			point p1( 10, 20 );
			const point p2( 3, 4 );
			p1 -= p2;
			ensure( p1[ 0 ] == 7 and p1[ 1 ] == 16, "operator -= failed" );

			const auto p3 = p1 - point( 2, 6 );
			ensure( p3[ 0 ] == 5 and p3[ 1 ] == 10, "operator - failed" );
		}

		//	multiplication
		{
			point p1( 2, 3 );
			p1 *= 2;
			ensure( p1[ 0 ] == 4 and p1[ 1 ] == 6, "operator *= ( scalar ) failed" );

			const point p2( 2, 2 );
			p1 *= p2;
			ensure( p1[ 0 ] == 8 and p1[ 1 ] == 12, "operator *= ( point ) failed" );
		}

		//	division
		{
			point p1( 10, 20 );
			p1 /= 2;
			ensure( p1[ 0 ] == 5 and p1[ 1 ] == 10, "operator /= failed" );
		}

		//	is_inside
		{
			const point p1( 10, 10 );
			const point p2( 5, 5 );
			const point p3( 15, 15 );

			ensure( p2.is_inside( p1 ), "p2 should be inside p1" );
			ensure( not p3.is_inside( p1 ), "p3 should not be inside p1" );
			ensure( p1.is_inside( p1 ), "p1 should be inside itself" );
		}

		//	get_length
		{
			const point p1( 3, 4 );
			//	sqrt( 3*3 + 4*4 ) = sqrt( 9 + 16 ) = sqrt( 25 ) = 5
			ensure( p1.get_length( ) == 5, "get_length failed" );

			const point< float, 3 > p2( 2.0f, 3.0f, 6.0f );
			//	sqrt( 2*2 + 3*3 + 6*6 ) = sqrt( 4 + 9 + 36 ) = sqrt( 49 ) = 7
			ensure( p2.get_length( ) == 7.0f, "get_length for 3d float point failed" );
		}

		//	get_product
		{
			const point< int, 3 > p1( 2, 3, 4 );
			ensure( p1.get_product( ) == 24, "get_product failed" );

			const point< float, 2 > p2( 0.5f, 0.5f );
			ensure( p2.get_product( ) == 0.25f, "get_product for float point failed" );
		}

		//	3d point
		{
			const point< int, 3 > p( 1, 2, 3 );
			ensure( p[ 0 ] == 1 and p[ 1 ] == 2 and p[ 2 ] == 3, "3d point failed" );
			ensure( p.get_length( ) == 3, "get_length for 3d int point failed ( truncated )" );
		}

		//	--------------------------------------------------
		//	line_view
		//	--------------------------------------------------
		using	point_2d	=	point< int, 2 >; 
		
		//	test 1: horizontal line
		{
			const point_2d start{ 0, 0 };
			const point_2d end{ 5, 0 };
			const auto line = start | line_to( end );
			
			const vector< point_2d > expected = 
			{
				{ 0, 0 }, { 1, 0 }, { 2, 0 }, { 3, 0 }, { 4, 0 }
			};
			
			ensure( equal( line, expected ), "horizontal line mismatch" );
		}

		//	test 2: vertical line
		{
			const point_2d start{ 2, 2 };
			const point_2d end{ 2, 5 };
			const auto line = start | line_to( end );
			
			const vector< point_2d > expected = 
			{
				{ 2, 2 }, { 2, 3 }, { 2, 4 }
			};

			ensure( equal( line, expected ), "vertical line mismatch" );
		}

		//	test 3: diagonal line
		{
			const point_2d start{ 0, 0 };
			const point_2d end{ 3, 3 };
			const auto line = start | line_to( end );
			
			const vector< point_2d > expected = 
			{
				{ 0, 0 }, { 1, 1 }, { 2, 2 }
			};

			ensure( equal( line, expected ), "diagonal line mismatch" );
		}

		//	test 4: negative direction
		{
			const point_2d start{ 3, 0 };
			const point_2d end{ 0, 0 };
			const auto line = start | line_to( end );

			const vector< point_2d > expected = 
			{
				{ 3, 0 }, { 2, 0 }, { 1, 0 }
			};

			ensure( equal( line, expected ), "negative direction line mismatch" );
		}

		//	test 5: empty line (point)
		{
			const point_2d start{ 1, 1 };
			const point_2d end{ 1, 1 };
			const auto line = start | line_to( end );
			
			ensure( line.begin( ) == line.end( ), "empty line should be empty" );
		}
		
		//	test 6: view composition (to)
		{
			const point_2d start{ 0, 0 };
			const point_2d end{ 2, 0 };
			const ::std::array< point_2d, 2 > array_result = start | line_to( end ) | to;
			
			ensure( array_result.size( ) == 2, "to size mismatch" );
			ensure( array_result[ 0 ] == point_2d{ 0, 0 }, "element 0 mismatch" );
			ensure( array_result[ 1 ] == point_2d{ 1, 0 }, "element 1 mismatch" );
		}

		//	--------------------------------------------------
		//	rectangle (geometry)
		//	--------------------------------------------------
		using	rectangle	=	::sak::geometry< point< int, 2 > >::rectangle;

		//	get_size
		{
			const rectangle r{ { 1, 1 }, { 10, 10 } };
			const auto size = r.get_size( );
			ensure( size[ 0 ] == 9 and size[ 1 ] == 9, "get_size failed" );
		}

		//	contains point
		{
			const rectangle r{ { 1, 1 }, { 10, 10 } };
			ensure( r.contains( { 1, 1 } ), "should contain start point" );
			ensure( r.contains( { 10, 10 } ), "should contain end point" );
			ensure( r.contains( { 5, 5 } ), "should contain middle point" );
			ensure( not r.contains( { 0, 5 } ), "should not contain point outside ( right )" );
		}

		//	is_inside rectangle
		{
			const rectangle outer{ { 1, 1 }, { 10, 10 } };
			const rectangle inner{ { 2, 2 }, { 9, 9 } };
			const rectangle edge{ { 1, 1 }, { 5, 5 } };
			const rectangle outside{ { 0, 0 }, { 5, 5 } };

			ensure( inner.is_inside( outer ), "inner should be inside outer" );
			ensure( edge.is_inside( outer ), "edge should be inside outer" );
			ensure( outer.is_inside( outer ), "outer should be inside itself" );
			ensure( not outside.is_inside( outer ), "outside rectangle should not be inside outer" );
			ensure( not outer.is_inside( inner ), "outer should not be inside inner" );
		}

		println( "all tests for sak/geometry passed" );
	}
	catch( const exception& error )
	{
		println( "test failed: {}", error.what( ) );
		return	exit_failure;
	}

	return	exit_success;
}


