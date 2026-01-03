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
 * File:   tests/sak/geometry/test_sak_geometry_rectangle.cpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2026-01-02 00:32
 */


#include <print>
#include <string>
#include <vector>
#include <cstdlib>
#include <exception>

#include <sak/ensure.hpp>
#include <sak/geometry/geometry.hpp>


auto main( const int argument_count, const char* argument_values[ ] ) -> int
{{
	using	::std::string;
	using	::std::vector;
	using	::std::println;
	using	::std::exception;
	using	::sak::ensure;

	const vector< string > arguments( argument_values, argument_values + argument_count );
	for( const auto& value : arguments )
		println( "{}", value );
	
	try
	{
		println( "starting tests for: sak/geometry/rectangle..." );

		using	point	=	::sak::point< int, 2 >;
		using	rectangle	=	::sak::geometry< point >::rectangle;

		//	test 1: get_size
		{
			const rectangle r{ { 1, 1 }, { 10, 10 } };
			const auto size = r.get_size( );
			ensure( size[ 0 ] == 9 and size[ 1 ] == 9, "get_size failed" );
		}

		//	test 2: contains point
		{
			const rectangle r{ { 1, 1 }, { 10, 10 } };
			ensure( r.contains( { 1, 1 } ), "should contain start point" );
			ensure( r.contains( { 10, 10 } ), "should contain end point" );
			ensure( r.contains( { 5, 5 } ), "should contain middle point" );
			ensure( not r.contains( { 0, 5 } ), "should not contain point outside ( right )" );
		}

		//	test 3: encloses rectangle
		{
			const rectangle outer{ { 1, 1 }, { 10, 10 } };
			const rectangle inner{ { 2, 2 }, { 9, 9 } };
			const rectangle edge{ { 1, 1 }, { 5, 5 } };
			const rectangle outside{ { 0, 0 }, { 5, 5 } };

			ensure( outer.encloses( inner ), "outer should enclose inner" );
			ensure( outer.encloses( edge ), "outer should enclose edge" );
			ensure( outer.encloses( outer ), "outer should enclose itself" );
			ensure( not outer.encloses( outside ), "outer should not enclose outside rectangle" );
			ensure( not inner.encloses( outer ), "inner should not enclose outer" );
		}

		println( "all tests for sak/geometry/rectangle passed!" );
	}
	catch( const exception& error )
	{
		println( "test failed: {}", error.what( ) );
		return	EXIT_FAILURE;
	}

	return	EXIT_SUCCESS;
}}


