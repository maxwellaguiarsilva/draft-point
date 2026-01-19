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
//	File:   tests/sak/geometry/test_sak_geometry_line_view.cpp
//	Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	
//	Created on 2026-01-13 14:29:29
//


#include <print>
#include <string>
#include <vector>
#include <exception>
#include <sak/ensure.hpp>
#include <sak/using.hpp>
#include <sak/geometry/point.hpp>
#include <sak/geometry/line_view.hpp>
#include <sak/ranges/to_array.hpp>
#include <sak/ranges/views/join_with.hpp>


auto main( const int argument_count, const char* argument_values[ ] ) -> int
{{
	__using( ::sak::
	,exit_success
	,exit_failure
	,ensure
	,point
	,line_to )
	__using( ::sak::ranges::, to_array )
	__using( ::sak::ranges::views::, join_with )
	__using( ::std::, string, vector, println, exception )
	__using( ::std::ranges::, equal )

	println( "{}", join_with( vector< string >( argument_values, argument_values + argument_count ), "\n" ) );

	try
	{
		println( "starting tests for: sak/geometry/line_view" );

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
		
		//	test 6: view composition (to_array)
		{
			const point_2d start{ 0, 0 };
			const point_2d end{ 2, 0 };
			const ::std::array< point_2d, 2 > array_result = start | line_to( end ) | to_array;
			
			ensure( array_result.size( ) == 2, "to_array size mismatch" );
			ensure( array_result[ 0 ] == point_2d{ 0, 0 }, "element 0 mismatch" );
			ensure( array_result[ 1 ] == point_2d{ 1, 0 }, "element 1 mismatch" );
		}
		
		//	--------------------------------------------------

		println( "all tests for sak/geometry/line_view passed" );
	}
	catch( const exception& error )
	{
		println( "test failed: {}", error.what( ) );
		return	exit_failure;
	}

	return	exit_success;
}}


