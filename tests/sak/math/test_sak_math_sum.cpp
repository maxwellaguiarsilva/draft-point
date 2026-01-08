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
 * File:   sak/math/test_sak_math_sum.cpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-12-31 12:30
 */


#include <print>
#include <string>
#include <vector>
#include <exception>
#include <typeinfo>
#include <sak/ensure.hpp>
#include <sak/using.hpp>
#include <sak/math/math.hpp>


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
	)
	using	::sak::math::sum;

	const vector< string > arguments( argument_values, argument_values + argument_count );
	for( const auto& value : arguments )
		println( "{}", value );

	try
	{
		println( "starting tests for sak::math::sum" );

		//	integers
		{
			vector< int > values = { 10, 20, 30 };
			ensure( sum( values ) == 60, "test 1 ( integers ) failed" );
			println( "test 1 ( integers ) passed" );
		}

		//	floating point
		{
			vector< double > values = { 1.5, 1.5 };
			auto result = sum( values );
			println( "sum of {{ 1.5, 1.5 }} = {}", result );
			println( "type of result: {}", typeid( result ).name( ) );
			ensure( result == 3.0, "test 2 ( floating point 1.5+1.5 ) failed" );
			println( "test 2 ( floating point 1.5+1.5 ) passed" );

			vector< double > values2 = { 0.9, 0.9 };
			auto result2 = sum( values2 );
			println( "sum of {{ 0.9, 0.9 }} = {}", result2 );
			println( "type of result2: {}", typeid( result2 ).name( ) );
			ensure( result2 == 1.8, "test 3 ( floating point 0.9+0.9 ) failed" );
			println( "test 3 ( floating point 0.9+0.9 ) passed" );

			vector< double > values3( 100, 0.9 );
			auto result3 = sum( values3 );
			println( "sum of 100 * 0.9 = {}", result3 );
			ensure( result3 >= 89.0, "test 4 ( 100 * 0.9 ) failed" );
			println( "test 4 ( 100 * 0.9 ) passed" );
		}

		println( "all tests for sak::math::sum passed" );
	}
	catch( const exception& error )
	{
		println( "test failed: {}", error.what( ) );
		return	exit_failure;
	}

	return	exit_success;
}}


