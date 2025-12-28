/*
 * Copyright (C) 2025 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
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
 * File:   tests/sak/test_sak_string.cpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-12-28 14:51
 */


//	--------------------------------------------------
#include <print>
#include <string>
#include <vector>
#include <cstdlib>

#include <sak/string.hpp>
#include <sak/ensure.hpp>


//	--------------------------------------------------


auto main( const int argument_count, const char* argument_values[ ] ) -> int
{{
	using	::std::string;
	using	::std::vector;
	using	::std::println;

	const vector< string > arguments( argument_values, argument_values + argument_count );
	for( const auto& value : arguments )
		println( "{}", value );
	
	println( "Starting tests for: sak/string..." );

	using	::sak::to_lower_case;
	using	::sak::to_upper_case;
	using	::sak::ensure;

	// Test char to_lower_case
	ensure( to_lower_case( 'A' ) == 'a', "to_lower_case('A') failed" );
	ensure( to_lower_case( 'Z' ) == 'z', "to_lower_case('Z') failed" );
	ensure( to_lower_case( 'a' ) == 'a', "to_lower_case('a') failed" );
	ensure( to_lower_case( '1' ) == '1', "to_lower_case('1') failed" );

	// Test string to_lower_case
	ensure( to_lower_case( string( "ABC" ) ) == "abc", "to_lower_case(\"ABC\") failed" );
	ensure( to_lower_case( string( "aBc123!" ) ) == "abc123!", "to_lower_case(\"aBc123!\") failed" );

	// Test char to_upper_case
	ensure( to_upper_case( 'a' ) == 'A', "to_upper_case('a') failed" );
	ensure( to_upper_case( 'z' ) == 'Z', "to_upper_case('z') failed" );
	ensure( to_upper_case( 'A' ) == 'A', "to_upper_case('A') failed" );
	ensure( to_upper_case( '1' ) == '1', "to_upper_case('1') failed" );

	// Test string to_upper_case
	ensure( to_upper_case( string( "abc" ) ) == "ABC", "to_upper_case(\"abc\") failed" );
	ensure( to_upper_case( string( "aBc123!" ) ) == "ABC123!", "to_upper_case(\"aBc123!\") failed" );

	println( "All tests for sak/string passed!" );

    return	EXIT_SUCCESS;
}};


