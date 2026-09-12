//	
//	SPDX-FileCopyrightText: 2026 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	SPDX-License-Identifier: GPL-3.0-or-later
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


