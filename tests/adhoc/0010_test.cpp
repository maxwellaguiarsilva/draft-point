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
//	File:   tests/adhoc/0010_test.cpp
//	Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	
//	Created on 2026-08-15 19:54:28
//	


#include <print>
#include <ranges>
#include <string>
#include <vector>
#include <exception>
#include <sak/ensure.hpp>
#include <sak/ranges/contains.hpp>
#include <sak/using.hpp>


auto main( const int argument_count, const char* argument_values[ ] ) -> int
{
	__using( ::sak::, exit_success, exit_failure, ensure )
	__using( ::std::, string, vector, println, exception )
	__using( ::sak::ranges::, contains )

	const vector< string > arguments( argument_values, argument_values + argument_count );
	if( contains( arguments, { "-h", "--help" } ) )
		return	println( "this executable is a battery of tests about: test" ), exit_success;

	try
	{
		println( "starting tests for: test" );

		//	--------------------------------------------------
		//	todo: implement tests here
		//	signature: `ensure( bool expression, const std::string& message )`
		//	example:   `ensure( 1 + 1 == 2, "math is broken" );`
		//	--------------------------------------------------

		println( "all tests for test passed" );
	}
	catch( const exception& error )
	{
		println( "test failed: {}", error.what( ) );
		return	exit_failure;
	}

	return	exit_success;
}


