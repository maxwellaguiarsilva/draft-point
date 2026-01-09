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
 * File:   adhoc/0006_join_with/0006_join_with.cpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2026-01-09 16:39
 */


#include <print>
#include <string>
#include <vector>
#include <exception>
#include <ranges>
#include <iostream>
#include <sak/ensure.hpp>
#include <sak/using.hpp>
#include <sak/ranges/fold_left_first.hpp>


namespace sak::ranges {

struct __join_with
{
	template< ::std::ranges::input_range t_range, typename t_delimiter >
	constexpr auto operator ( ) ( t_range&& range, t_delimiter&& delimiter ) const
	{
		return	::sak::ranges::fold_left_first( range, [ &delimiter ]( auto a, auto b ) {
			return	a + delimiter + b;
		} );
	}
};
inline constexpr auto join_with = __join_with{ };

}


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
	__using( ::sak::ranges::
		,join_with
	)

	const vector< string > arguments( argument_values, argument_values + argument_count );
	
	try
	{
		println( "starting tests for: join_with" );

		const vector< string > words = { "hello", "world", "from", "sak" };
		const string delimiter = " ";
		
		const auto joined = join_with( words, delimiter );
		
		ensure( joined.has_value( ), "joined should have a value" );
		ensure( joined.value( ) == "hello world from sak", "joined string mismatch" );
		
		println( "joined result: {}", joined.value( ) );

		const vector< string > empty_vector = { };
		const auto empty_joined = join_with( empty_vector, delimiter );
		ensure( not empty_joined.has_value( ), "empty join should not have a value" );

		const vector< string > single_word = { "only" };
		const auto single_joined = join_with( single_word, delimiter );
		ensure( single_joined.has_value( ), "single word join should have a value" );
		ensure( single_joined.value( ) == "only", "single word join mismatch" );

		println( "all tests for join_with passed" );
	}
	catch( const exception& error )
	{
		println( "test failed: {}", error.what( ) );
		return	exit_failure;
	}

	return	exit_success;
}}


