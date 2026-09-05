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
//	File:   tests/sak/ranges/test_sak_ranges_views.cpp
//	Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	
//	Created on 2026-08-02 12:24:30
//	


#include <exception>
#include <print>
#include <ranges>
#include <string>
#include <vector>
#include <sak/ensure.hpp>
#include <sak/using.hpp>
#include <sak/ranges/views/enumerate.hpp>
#include <sak/ranges/views/rotated.hpp>


auto main( const int argument_count, const char* argument_values[ ] ) -> int
{
	__using( ::sak::
		,exit_success
		,exit_failure
		,ensure
	)
	__using( ::std::
		,exception
		,println
		,ptrdiff_t
		,string
		,vector
	)
	__using( ::std::ranges::, to )
	__using( ::sak::ranges::views::
		,enumerate
		,rotated
	)

	const vector< string > arguments( argument_values, argument_values + argument_count );
	for( const auto& value : arguments )
		println( "{}", value );

	try
	{
		println( "starting tests for: sak/ranges/views" );

		//	--------------------------------------------------
		//	enumerate
		//	--------------------------------------------------
		{
			const vector< int > values = { 10, 20, 30 };
			ptrdiff_t expected_index = 0;
			for( auto [ index, val ] : enumerate( values ) )
			{
				ensure( index == expected_index, "index should match expected sequence" );
				ensure( val == ( expected_index + 1 ) * 10, "value should match expected multiplier" );
				++expected_index;
			}
			ensure( expected_index == 3, "should iterate through all elements" );
		}

		{
			const vector< int > values = { 5, 6 };
			ptrdiff_t expected_index = 0;
			for( auto [ index, val ] : values bitor enumerate )
			{
				ensure( index == expected_index, "index should match expected sequence on pipe" );
				ensure( val == expected_index + 5, "value should match expected sum on pipe" );
				++expected_index;
			}
			ensure( expected_index == 2, "should iterate through all elements on pipe" );
		}

		//	--------------------------------------------------
		//	rotated
		//	--------------------------------------------------
		{
			const auto input	=	vector< int >{ 1, 2, 3, 4, 5 };
			const auto expected	=	vector< int >{ 2, 3, 4, 5, 1 };
			const auto result	=	to< vector< int > >( input | rotated );

			ensure( result == expected, "rotated view (default) failed" );
		}

		{
			const auto input	=	vector< int >{ 1, 2, 3, 4, 5 };
			const auto expected	=	vector< int >{ 3, 4, 5, 1, 2 };
			const auto result	=	to< vector< int > >( input | rotated( 2 ) );

			ensure( result == expected, "rotated view (offset 2) failed" );
		}

		{
			const auto input	=	vector< int >{ 1, 2, 3, 4, 5 };
			const auto expected	=	vector< int >{ 1, 2, 3, 4, 5 };
			const auto result	=	to< vector< int > >( input | rotated( 5 ) );

			ensure( result == expected, "rotated view (offset equal to size) failed" );
		}

		println( "all tests for sak/ranges/views passed" );
	}
	catch( const exception& error )
	{
		println( "test failed: {}", error.what( ) );
		return	exit_failure;
	}

	return	exit_success;
}


