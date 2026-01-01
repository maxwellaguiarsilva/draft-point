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
 * File:   tests/sak/pattern/test_sak_pattern_value_or.cpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-12-27 12:50
 */


//	--------------------------------------------------
#include <print>
#include <string>
#include <vector>
#include <cstdlib>

#include <sak/pattern/value_or.hpp>
#include <sak/ensure.hpp>
#include <map>
#include <unordered_map>
#include <string>

//	--------------------------------------------------


auto main( const int argument_count, const char* argument_values[ ] ) -> int
{{
	using	::std::string;
	using	::std::vector;
	using	::std::println;

	const vector< string > arguments( argument_values, argument_values + argument_count );
	for( const auto& value : arguments )
		println( "{}", value );
	
	println( "Starting tests for: sak/pattern/value_or..." );

	using	::std::map;
	using	::std::unordered_map;
	using	::sak::pattern::value_or;
	using	::sak::ensure;

	{
		map< string, float > fruits_price = { { "apple", 1.5f }, { "banana", 0.75f } };
		
		ensure( value_or( fruits_price, "apple", 0.0f ) == 1.5f, "Value for 'apple' should be 1.5f" );
		ensure( value_or( fruits_price, "banana", 0.0f ) == 0.75f, "Value for 'banana' should be 0.75f" );
		ensure( value_or( fruits_price, "orange", 0.0f ) == 0.0f, "Value for 'orange' should be 0.0f" );
	}

	{
		unordered_map< int, string > id_to_name = { { 1, "Alice" }, { 2, "Bob" } };
		string unknown = "Unknown";

		ensure( value_or( id_to_name, 1, unknown ) == "Alice", "Value for ID 1 should be 'Alice'" );
		ensure( value_or( id_to_name, 2, unknown ) == "Bob", "Value for ID 2 should be 'Bob'" );
		ensure( value_or( id_to_name, 3, unknown ) == "Unknown", "Value for ID 3 should be 'Unknown'" );
	}

	println( "All tests for sak/pattern/value_or passed!" );

    return	EXIT_SUCCESS;
}}


