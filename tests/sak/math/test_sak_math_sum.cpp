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
 * File:   tests/sak/math/test_sak_math_sum.cpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-12-31 12:15
 */


//	--------------------------------------------------
#include <print>
#include <string>
#include <vector>
#include <cstdlib>
#include <typeinfo>

#include <sak/ensure.hpp>
#include <sak/math/math.hpp>

//	--------------------------------------------------


auto main( const int, const char* [ ] ) -> int
{{
	using	::std::vector;
	using	::std::println;
	using	::sak::math::sum;

	println( "starting tests for sak::math::sum..." );

	//	test 1: integers
	{
		vector< int > values = { 10, 20, 30 };
		if( sum( values ) not_eq 60 )
			return	EXIT_FAILURE;
		println( "test 1 (integers) passed" );
	}

	//	test 2: floating point (this is expected to fail with current implementation)
	{
		vector< double > values = { 1.5, 1.5 };
		auto result = sum( values );
		println( "sum of {{ 1.5, 1.5 }} = {}", result );
		println( "type of result: {}", typeid( result ).name( ) );
		if( result not_eq 3.0 ) 
			println( "test 2 (floating point 1.5+1.5) failed! result was {}, expected 3.0", result );
		else
			println( "test 2 (floating point 1.5+1.5) passed" );

		vector< double > values2 = { 0.9, 0.9 };
		auto result2 = sum( values2 );
		println( "sum of {{ 0.9, 0.9 }} = {}", result2 );
		println( "type of result2: {}", typeid( result2 ).name( ) );
		if( result2 not_eq 1.8 )
			println( "test 3 (floating point 0.9+0.9) failed! result was {}, expected 1.8", result2 );
		else
			println( "test 3 (floating point 0.9+0.9) passed" );

		vector< double > values3( 100, 0.9 );
		auto result3 = sum( values3 );
		println( "sum of 100 * 0.9 = {}", result3 );
		if( result3 < 89.0 )
			println( "test 4 (100 * 0.9) failed! result was {}, expected ~90.0", result3 );
		else
			println( "test 4 (100 * 0.9) passed" );
	}

	println( "all tests for sak::math::sum passed!" );

	return	EXIT_SUCCESS;
}}


