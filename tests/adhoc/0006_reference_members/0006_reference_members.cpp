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
 * File:   tests/adhoc/0006_reference_members/0006_reference_members.cpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-12-31 20:08
 */


//	--------------------------------------------------
#include <print>
#include <cstdlib>
#include <exception>
#include <memory>

#include <sak/ensure.hpp>
#include <game/geometry.hpp>

//	--------------------------------------------------


namespace {


using	::game::geometry::point;


class bad_rectangle
{
public:
	bad_rectangle( ) : m_data{ }, from( m_data[ 0 ] ), to( m_data[ 1 ] ) { }

	point m_data[ 2 ];
	const point& from;
	const point& to;
};

class another_bad_rectangle
{
public:
	point m_data[ 2 ];
	const point& from	=	m_data[ 0 ];
	const point& to		=	m_data[ 1 ];
};


}


auto main( ) -> int
{{
	using	::std::println;
	using	::std::exception;
	using	::std::addressof;
	using	::sak::ensure;

	try
	{
		println( "starting tests for: reference_members..." );

		println( "--- testing bad_rectangle ---" );
		bad_rectangle original;
		original.m_data[ 0 ][ 0 ] = 10;
		
		println( "creating copy of original (original.m_data[0] is 10)..." );
		bad_rectangle copy = original;
		
		println( "--- initial state ---" );
		println( "original.m_data[0][0]: {}", original.m_data[ 0 ][ 0 ] );
		println( "copy.m_data[0][0]:     {}", copy.m_data[ 0 ][ 0 ] );
		println( "copy.from[0]:          {}", copy.from[ 0 ] );
		println( "addresses: original.m_data[0]: {}, copy.from: {}", static_cast< const void* >( addressof( original.m_data[ 0 ] ) ), static_cast< const void* >( addressof( copy.from ) ) );

		println( "\n--- modifying original.m_data[0][0] to 20 ---" );
		original.m_data[ 0 ][ 0 ] = 20;
		println( "original.m_data[0][0]: {}", original.m_data[ 0 ][ 0 ] );
		println( "copy.m_data[0][0]:     {}", copy.m_data[ 0 ][ 0 ] );
		println( "copy.from[0]:          {}", copy.from[ 0 ] );

		println( "\n--- modifying copy.m_data[0][0] to 30 ---" );
		copy.m_data[ 0 ][ 0 ] = 30;
		println( "original.m_data[0][0]: {}", original.m_data[ 0 ][ 0 ] );
		println( "copy.m_data[0][0]:     {}", copy.m_data[ 0 ][ 0 ] );
		println( "copy.from[0]:          {}", copy.from[ 0 ] );

		ensure( addressof( copy.from ) == addressof( original.m_data[ 0 ] ), "copy.from should point to original's data" );
		ensure( addressof( copy.from ) not_eq addressof( copy.m_data[ 0 ] ), "copy.from should NOT point to its own data" );

		println( "\nconclusion: copy.from points to ORIGINAL's data, not its own!" );

		println( "\n--- testing another_bad_rectangle ---" );
		another_bad_rectangle another_original;
		another_original.m_data[ 0 ][ 0 ] = 100;

		println( "creating copy of another_original (another_original.m_data[0] is 100)..." );
		another_bad_rectangle another_copy = another_original;

		println( "--- initial state ---" );
		println( "another_original.m_data[0][0]: {}", another_original.m_data[ 0 ][ 0 ] );
		println( "another_copy.m_data[0][0]:     {}", another_copy.m_data[ 0 ][ 0 ] );
		println( "another_copy.from[0]:          {}", another_copy.from[ 0 ] );

		println( "\n--- modifying another_original.m_data[0][0] to 200 ---" );
		another_original.m_data[ 0 ][ 0 ] = 200;
		println( "another_original.m_data[0][0]: {}", another_original.m_data[ 0 ][ 0 ] );
		println( "another_copy.m_data[0][0]:     {}", another_copy.m_data[ 0 ][ 0 ] );
		println( "another_copy.from[0]:          {}", another_copy.from[ 0 ] );

		println( "\n--- modifying another_copy.m_data[0][0] to 300 ---" );
		another_copy.m_data[ 0 ][ 0 ] = 300;
		println( "another_original.m_data[0][0]: {}", another_original.m_data[ 0 ][ 0 ] );
		println( "another_copy.m_data[0][0]:     {}", another_copy.m_data[ 0 ][ 0 ] );
		println( "another_copy.from[0]:          {}", another_copy.from[ 0 ] );

		ensure( addressof( another_copy.from ) == addressof( another_original.m_data[ 0 ] ), "another_copy.from should point to another_original's data" );
		ensure( addressof( another_copy.from ) not_eq addressof( another_copy.m_data[ 0 ] ), "another_copy.from should NOT point to its own data" );

		println( "\nconclusion: another_copy.from also points to ORIGINAL's data!" );

		println( "\nall tests for reference_members passed!" );
	}
	catch( const exception& error )
	{
		println( "test failed: {}", error.what( ) );
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}}


