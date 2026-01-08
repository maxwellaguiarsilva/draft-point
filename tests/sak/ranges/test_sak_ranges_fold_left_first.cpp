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
 * File:   sak/ranges/test_sak_ranges_fold_left_first.cpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2026-01-08 01:05
 */


#include <print>
#include <string>
#include <vector>
#include <exception>
#include <functional>
#include <sak/ranges/fold_left_first.hpp>
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
		,plus
	)
	using	::sak::ranges::fold_left_first;

	const vector< string > arguments( argument_values, argument_values + argument_count );
	for( const auto& value : arguments )
		println( "{}", value );
	
	try
	{
		println( "starting tests for: sak/ranges/fold_left_first..." );

		//	empty range
		{
			const vector< int > v;
			const auto result = fold_left_first( v, plus< int >( ) );
			ensure( not result.has_value( ), "empty range should return empty optional" );
		}

		//	single element
		{
			const vector< int > v = { 42 };
			const auto result = fold_left_first( v, plus< int >( ) );
			ensure( result.has_value( ), "single element range should return a value" );
			ensure( *result == 42, "single element range should return the element itself" );
		}

		//	multiple elements
		{
			const vector< int > v = { 1, 2, 3, 4, 5 };
			const auto result = fold_left_first( v, plus< int >( ) );
			ensure( result.has_value( ), "multiple elements range should return a value" );
			ensure( *result == 15, "sum of 1..5 should be 15" );
		}

		//	string concatenation
		{
			const vector< string > v = { "a", "b", "c" };
			const auto result = fold_left_first( v, plus< string >( ) );
			ensure( result.has_value( ), "string range should return a value" );
			ensure( *result == "abc", "concatenation of a, b, c should be abc" );
		}

		println( "all tests for sak/ranges/fold_left_first passed!" );
	}
	catch( const exception& error )
	{
		println( "test failed: {}", error.what( ) );
		return	exit_failure;
	}

	return	exit_success;
}}


