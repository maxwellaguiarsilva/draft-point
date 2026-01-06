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
 * File:   adhoc/0005_custom_chunk/0005_custom_chunk.cpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2026-01-05 20:32
 */


#include <print>
#include <string>
#include <vector>
#include <ranges>
#include <algorithm>
#include <exception>
#include <sak/ranges/chunk.hpp>
#include <sak/ensure.hpp>
#include <sak/using.hpp>
//	--------------------------------------------------


auto main( const int argument_count, const char* argument_values[ ] ) -> int
{{
	__using( ::sak::
		,exit_success
		,exit_failure
		,ensure
	)
	__using( ::std::
		,print
		,println
		,string
		,vector
		,exception
	)

	const vector< string > arguments( argument_values, argument_values + argument_count );
	for( const auto& value : arguments )
		println( "{}", value );

	try
	{
		vector< int > data	=	{ 1, 2, 3, 4, 5, 6 };
		auto chunks			=	::sak::ranges::chunk( data, 2 );
		
		int chunk_count	=	0;
		for( auto current_chunk : chunks )
		{
			for( int element : current_chunk )
				print( "{} ", element );

			print( "| " );
			++chunk_count;
		}
		println( "" );

		ensure( chunk_count == 3, "should have 3 chunks" );

		println( "all tests for 0005_custom_chunk passed!" );
	}
	catch( const exception& error )
	{
		println( "test failed: {}", error.what( ) );
		return	exit_failure;
	}

	return	exit_success;
}}


