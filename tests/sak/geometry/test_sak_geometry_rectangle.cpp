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
 * File:   sak/geometry/test_sak_geometry_rectangle.cpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2026-01-02 00:33
 */


#include <print>
#include <string>
#include <vector>
#include <exception>
#include <sak/ensure.hpp>
#include <sak/using.hpp>
#include <sak/geometry/geometry.hpp>


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
		println( "starting tests for: sak/geometry/rectangle..." );

		using	point	=	::sak::point< int, 2 >;
		using	rectangle	=	::sak::geometry< point >::rectangle;

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

		println( "all tests for sak/geometry/rectangle passed!" );
	}
	catch( const exception& error )
	{
		println( "test failed: {}", error.what( ) );
		return	exit_failure;
	}

	return	exit_success;
}}


