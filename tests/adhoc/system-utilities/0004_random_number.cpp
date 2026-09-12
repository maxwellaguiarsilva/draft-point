//	
//	SPDX-FileCopyrightText: 2026 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	SPDX-License-Identifier: GPL-3.0-or-later
//	


#include <print>
#include <ranges>
#include <string>
#include <vector>
#include <exception>
#include <sak/ensure.hpp>
#include <sak/using.hpp>
#include <random>
#include <iostream>


auto main( const int argument_count, const char* argument_values[ ] ) -> int
{
	__using( ::sak::, exit_success, exit_failure, ensure )
	__using( ::std::
		,string
		,vector
		,println
		,exception
		,random_device
		,mt19937_64
		,uniform_int_distribution
	)
	__using( ::std::views::, join_with )
	__using( ::std::ranges::, to )

	println( "{}", vector< string >( argument_values, argument_values + argument_count ) | join_with( '\n' ) | to< string >( ) );

	try
	{
		random_device	entropy;
		mt19937_64		engine( entropy( ) );

		println( "random number: {}", engine( ) );

		uniform_int_distribution< int >	range( 16, 231 );
		println( "random number [16, 231]: {}", range( engine ) );
	}
	catch( const exception& error )
	{
		println( "test failed: {}", error.what( ) );
		return	exit_failure;
	}

	return	exit_success;
}


