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
 * Created on 2026-01-09 16:36
 */


#include <print>
#include <string>
#include <vector>
#include <exception>
#include <ranges>
#include <iostream>
#include <sak/ensure.hpp>
#include <sak/using.hpp>
#include <sak/math/math.hpp>
#include <sak/ranges/fold_left_first.hpp>


namespace sak::ranges {

struct __join_with
{
	template< ::std::ranges::input_range t_range, typename t_delimiter >
	constexpr auto operator ( ) ( t_range&& range, t_delimiter&& delimiter ) const
	{
		__using( ::sak::math:: ,plus )
		__using( ::sak::ranges:: ,fold_left_first )
		__using( ::std:: ,bind )
		__using( ::std::ranges:: ,range_value_t )
		__using( ::std::placeholders:: ,_1 ,_2 )
		return	fold_left_first( range, bind( plus, bind( plus, _1, delimiter ), _2 ) ).value_or( range_value_t< t_range >{ } );
	}
};
inline constexpr auto join_with = __join_with{ };

}


auto main( const int argument_count, const char* argument_values[ ] ) -> int
{{
	__using( ::sak:: ,exit_success ,exit_failure ,ensure )
	__using( ::std:: ,string ,vector ,println ,exception )
	__using( ::sak::ranges:: ,join_with )

	const vector< string > arguments( argument_values, argument_values + argument_count );
	
	try
	{
		println( "starting tests for: join_with" );

		const vector< string > words = { "hello", "world", "from", "sak" };
		const string delimiter = " ";
		
		const auto joined = join_with( words, delimiter );
		
		ensure( joined == "hello world from sak", "joined string mismatch" );
		
		println( "joined result: {}", joined );

		const vector< string > empty_vector = { };
		const auto empty_joined = join_with( empty_vector, delimiter );
		ensure( empty_joined == "", "empty join should return an empty string" );

		const vector< string > single_word = { "only" };
		const auto single_joined = join_with( single_word, delimiter );
		ensure( single_joined == "only", "single word join mismatch" );

		println( "all tests for join_with passed" );
	}
	catch( const exception& error )
	{
		println( "test failed: {}", error.what( ) );
		return	exit_failure;
	}

	return	exit_success;
}}


