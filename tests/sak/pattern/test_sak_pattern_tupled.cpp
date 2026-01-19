//	
//	Copyright (C) 2025 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
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
//	File:   tests/sak/pattern/test_sak_pattern_tupled.cpp
//	Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	
//	Created on 2025-12-28 16:19:36
//


#include <print>
#include <string>
#include <vector>
#include <exception>
#include <array>
#include <ranges>
#include <algorithm>
#include <functional>
#include <utility>
#include <sak/pattern/tupled.hpp>
#include <sak/ensure.hpp>
#include <sak/using.hpp>


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
		,array
		,make_pair
		,less_equal
	)
	__using( ::std::ranges::, all_of )
	__using( ::std::views::, zip )
	using	::sak::pattern::tupled;

	const vector< string > arguments( argument_values, argument_values + argument_count );
	for( const auto& value : arguments )
		println( "{}", value );
	
	try
	{
		println( "starting tests for: sak/pattern/tupled" );

		const auto tupled_less_equal = tupled( less_equal{} );

		{
			const array left	=	{ 1, 2, 3, 4, 5 };
			const array right	=	{ 2, 3, 4, 5, 6 };
			
			ensure( all_of( zip( left, right ), tupled_less_equal ), "all_of zip( left, right ) with tupled less_equal should be true" );
		}

		{
			ensure( tupled_less_equal( make_pair( 1, 2 ) ), "tupled less_equal( pair( 1, 2 ) ) should be true" );
			ensure( not tupled_less_equal( make_pair( 2, 1 ) ), "tupled less_equal( pair( 2, 1 ) ) should be false" );
		}

		{
			ensure( tupled_less_equal( array{ 5, 5 } ), "tupled less_equal( array{ 5, 5 } ) should be true" );
			ensure( not tupled_less_equal( array{ 6, 5 } ), "tupled less_equal( array{ 6, 5 } ) should be false" );
		}

		println( "all tests for sak/pattern/tupled passed" );
	}
	catch( const exception& error )
	{
		println( "test failed: {}", error.what( ) );
		return	exit_failure;
	}

    return exit_success;
}}


