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
//	File:   tests/adhoc/data-processing/0006_successive_even_numbers.cpp
//	Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	
//	Created on 2026-02-22 18:23:47
//	


#include <print>
#include <string>
#include <vector>
#include <exception>
#include <sak/ensure.hpp>
#include <sak/using.hpp>
#include <array>
#include <cstddef>
#include <random>
#include <ranges>
#include <algorithm>
#include <iostream>


auto main( const int /*argument_count*/, const char* /*argument_values*/[ ] ) -> int
{
	__using( ::sak::, exit_success, exit_failure )
	__using( ::std::
		,println
		,exception
		,array
		,byte
		,to_integer
		,random_device
		,mt19937
		,uniform_int_distribution
	)
	__using( ::std::ranges::
		,for_each
		,generate
		,distance
	)
	__using( ::std::ranges::views::, chunk_by )
	
	try
	{
		println( "starting tests for: successive_even_numbers" );

		array< byte, 100 > random_bytes;
		random_device device;
		mt19937 generator( device( ) );
		uniform_int_distribution< int > distribution( 0, 255 );

		generate( random_bytes, [ & ]( )
		{
			return	static_cast< byte >( distribution( generator ) );
		} );

		auto is_even = [ ]( const byte byte_value ) noexcept
		{
			return	to_integer< int >( byte_value ) % 2 == 0;
		};

		auto successive_evens = random_bytes
			| chunk_by( [ &is_even ]( const byte first, const byte second ) noexcept
				{
					return	is_even( first ) and is_even( second );
				} );

		for_each( successive_evens, [ &random_bytes, &is_even ]( const auto range )
		{
			if( range.size( ) > 1 and is_even( range[ 0 ] ) )
				println( "found {} even numbers at position {}", range.size( ), distance( random_bytes.begin( ), range.begin( ) ) );
		} );

		println( "all tests for successive_even_numbers passed" );
	}
	catch( const exception& error )
	{
		println( "test failed: {}", error.what( ) );
		return	exit_failure;
	}

	return	exit_success;
}


