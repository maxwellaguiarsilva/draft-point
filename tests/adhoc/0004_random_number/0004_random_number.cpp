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
 * File:   adhoc/0004_random_number/0004_random_number.cpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2026-01-11 15:41
 */


#include <print>
#include <string>
#include <vector>
#include <exception>
#include <sak/ensure.hpp>
#include <sak/using.hpp>
#include <sak/ranges/views/join_with.hpp>
#include <random>
#include <iostream>


auto main( const int argument_count, const char* argument_values[ ] ) -> int
{{
	__using( ::sak::, exit_success, exit_failure, ensure )
	__using( ::sak::ranges::views::, join_with )
	__using( ::std::
		,string
		,vector
		,println
		,exception
		,random_device
		,mt19937_64
		,uniform_int_distribution
	)

	println( "{}", join_with( vector< string >( argument_values, argument_values + argument_count ), "\n" ) );

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
}}


