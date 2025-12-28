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
 * File:   sak/test_sak_string.cpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-12-28 14:41
 */


#include <sak/string.hpp>
#include <print>
#include <string>


auto main( ) -> int
{
	using	::std::println;
	using	::std::string;
	using	::sak::to_lower_case;
	using	::sak::to_upper_case;

	// Test char
	println( "Char tests:" );
	println( "A -> {}", to_lower_case( 'A' ) );
	println( "z -> {}", to_upper_case( 'z' ) );
	println( "1 -> {}", to_lower_case( '1' ) );

	// Test string
	string original = "Hello World! 123";
	println( "\nString tests:" );
	println( "Original: {}", original );
	println( "Lower:    {}", to_lower_case( original ) );
	println( "Upper:    {}", to_upper_case( original ) );

	return	0;
};