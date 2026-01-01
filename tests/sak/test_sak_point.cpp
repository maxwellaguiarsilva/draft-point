/*
 * Copyright ( C ) 2026 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * ( at your option ) any later version.
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
 * File:   tests/sak/test_sak_point.cpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2026-01-01 19:20
 */


//	--------------------------------------------------
#include <sak/point.hpp>
#include <sak/ensure.hpp>
#include <cstdlib>
#include <print>
//	--------------------------------------------------


auto main( ) -> int
{{
	using	::std::println;
	using	::sak::point;
	using	::sak::ensure;

	println( "starting tests for sak::point..." );

	//	test 1: basic creation and size
	{
		point< int, 2 > p{ 10, 20 };
		ensure( p.size( ) == 2, "point size should be 2" );
		ensure( p[ 0 ] == 10, "p[ 0 ] should be 10" );
		ensure( p[ 1 ] == 20, "p[ 1 ] should be 20" );
		println( "test 1 ( creation ) passed" );
	}

	//	test 2: addition
	{
		point< int, 2 > p1{ 1, 2 };
		point< int, 2 > p2{ 3, 4 };
		auto p3 = p1 + p2;
		ensure( p3[ 0 ] == 4, "p3[ 0 ] should be 4" );
		ensure( p3[ 1 ] == 6, "p3[ 1 ] should be 6" );
		println( "test 2 ( addition ) passed" );
	}

	//	test 3: scalar multiplication
	{
		point< int, 2 > p{ 5, 10 };
		auto p2 = p * 2;
		ensure( p2[ 0 ] == 10, "p2[ 0 ] should be 10" );
		ensure( p2[ 1 ] == 20, "p2[ 1 ] should be 20" );
		println( "test 3 ( scalar multiplication ) passed" );
	}

	//	test 4: encloses ( clone of coordinate behavior )
	{
		point< int, 2 > p1{ 10, 10 };
		point< int, 2 > p2{ 5, 5 };
		//	p2.encloses( p1 ) means all_of( p2 <= p1 )
		ensure( p2.encloses( p1 ), "p2 should enclose p1" );
		ensure( not p1.encloses( p2 ), "p1 should not enclose p2" );
		println( "test 4 ( encloses ) passed" );
	}

	//	test 5: get_length
	{
		point< double, 2 > p{ 3.0, 4.0 };
		ensure( p.get_length( ) == 5.0, "p length should be 5.0" );
		println( "test 5 ( get_length ) passed" );
	}

	println( "all tests for sak::point passed!" );
	return	EXIT_SUCCESS;
}}


