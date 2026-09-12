//	
//	SPDX-FileCopyrightText: 2026 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	SPDX-License-Identifier: GPL-3.0-or-later
//	


#include <exception>
#include <print>
#include <string>
#include <vector>
#include <sak/ensure.hpp>
#include <sak/using.hpp>
#include <sak/string.hpp>


auto main( const int argument_count, const char* argument_values[ ] ) -> int
{
	__using( ::sak::
		,exit_success
		,exit_failure
		,ensure
		,to_lower_case
		,to_upper_case
	)
	__using( ::std::
		,exception
		,println
		,string
		,vector
	)

	const vector< string > arguments( argument_values, argument_values + argument_count );
	for( const auto& value : arguments )
		println( "{}", value );

	try
	{
		println( "starting tests for: sak" );

		//	--------------------------------------------------
		//	string
		//	--------------------------------------------------

		//	test char to_lower_case
		ensure( to_lower_case( 'A' ) == 'a', "to_lower_case( 'a' ) failed" );
		ensure( to_lower_case( 'Z' ) == 'z', "to_lower_case( 'z' ) failed" );
		ensure( to_lower_case( 'a' ) == 'a', "to_lower_case( 'a' ) failed" );
		ensure( to_lower_case( '1' ) == '1', "to_lower_case( '1' ) failed" );

		//	test string to_lower_case
		ensure( to_lower_case( string( "ABC" ) ) == "abc", "to_lower_case( \"abc\" ) failed" );
		ensure( to_lower_case( string( "aBc123!" ) ) == "abc123!", "to_lower_case( \"abc123!\" ) failed" );

		//	test char to_upper_case
		ensure( to_upper_case( 'a' ) == 'A', "to_upper_case( 'a' ) failed" );
		ensure( to_upper_case( 'z' ) == 'Z', "to_upper_case( 'z' ) failed" );
		ensure( to_upper_case( 'A' ) == 'A', "to_upper_case( 'a' ) failed" );
		ensure( to_upper_case( '1' ) == '1', "to_upper_case( '1' ) failed" );

		//	test string to_upper_case
		ensure( to_upper_case( string( "abc" ) ) == "ABC", "to_upper_case( \"abc\" ) failed" );
		ensure( to_upper_case( string( "aBc123!" ) ) == "ABC123!", "to_upper_case( \"abc123!\" ) failed" );

		println( "all tests for sak passed" );
	}
	catch( const exception& error )
	{
		println( "test failed: {}", error.what( ) );
		return	exit_failure;
	}

	return	exit_success;
}


