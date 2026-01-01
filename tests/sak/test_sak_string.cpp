/*
 * Copyright ( C ) 2025 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * ( at your option ) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General License
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
#include <exception>

#include <sak/string.hpp>
#include <sak/ensure.hpp>
//	--------------------------------------------------


auto main( const int argument_count, const char* argument_values[ ] ) -> int
{{
	using	::std::string;
	using	::std::vector;
	using	::std::println;
	using	::std::exception;

	const vector< string > arguments( argument_values, argument_values + argument_count );
	for( const auto& value : arguments )
		println( "{}", value );
	
try
	{
		println( "starting tests for: sak/string..." );

		using	::sak::to_lower_case;
		using	::sak::to_upper_case;
		using	::sak::ensure;

		//	test char to_lower_case
		ensure( to_lower_case( 'A' ) == 'a', "to_lower_case('a') failed" );
		ensure( to_lower_case( 'Z' ) == 'z', "to_lower_case('z') failed" );
		ensure( to_lower_case( 'a' ) == 'a', "to_lower_case('a') failed" );
		ensure( to_lower_case( '1' ) == '1', "to_lower_case('1') failed" );

		//	test string to_lower_case
		ensure( to_lower_case( string( "ABC" ) ) == "abc", "to_lower_case(\"abc\") failed" );
		ensure( to_lower_case( string( "aBc123!" ) ) == "abc123!", "to_lower_case(\"abc123!\") failed" );

		//	test char to_upper_case
		ensure( to_upper_case( 'a' ) == 'A', "to_upper_case('a') failed" );
		ensure( to_upper_case( 'z' ) == 'Z', "to_upper_case('z') failed" );
		ensure( to_upper_case( 'A' ) == 'A', "to_upper_case('a') failed" );
		ensure( to_upper_case( '1' ) == '1', "to_upper_case('1') failed" );

		//	test string to_upper_case
		ensure( to_upper_case( string( "abc" ) ) == "ABC", "to_upper_case(\"abc\") failed" );
		ensure( to_upper_case( string( "aBc123!" ) ) == "ABC123!", "to_upper_case(\"abc123!\") failed" );

		println( "all tests for sak/string passed!" );
	}
	catch( const exception& error )
	{
		println( "test failed: {}", error.what( ) );
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}}


