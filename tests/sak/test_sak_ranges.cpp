//	
//	Copyright (C) 2026 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	
//	This program is free software: you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//	
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//	
//	You should have received a copy of the GNU General Public License
//	along with this program.  If not, see <http://www.gnu.org/licenses/>.
//	
//	
//	File:   tests/sak/test_sak_ranges.cpp
//	Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	
//	Created on 2026-08-02 12:18:05
//	


#include <array>
#include <exception>
#include <functional>
#include <print>
#include <ranges>
#include <string>
#include <vector>
#include <sak/ensure.hpp>
#include <sak/using.hpp>
#include <sak/ranges/chunk.hpp>
#include <sak/ranges/count_to.hpp>
#include <sak/ranges/fold_left_first.hpp>
#include <sak/ranges/operators.hpp>
#include <sak/ranges/to.hpp>
#include <sak/ranges/transform.hpp>
#include <sak/geometry/point.hpp>


auto main( const int argument_count, const char* argument_values[ ] ) -> int
{
	__using( ::sak::
		,exit_success
		,exit_failure
		,ensure
		,is_point
		,point
	)
	__using( ::std::
		,array
		,exception
		,plus
		,print
		,println
		,string
		,vector
	)
	__using( ::std::ranges::, fold_left )
	__using( ::sak::ranges::
		,chunk
		,count_to
		,fold_left_first
		,lazy_transform
		,operator+
		,operator-
		,operator*
		,operator/
		,operator%
		,operator+=
		,operator-=
		,operator*=
		,operator/=
		,operator%=
		,to
	)

	const vector< string > arguments( argument_values, argument_values + argument_count );
	for( const auto& value : arguments )
		println( "{}", value );
	
	try
	{
		println( "starting tests for: sak/ranges" );

		//	--------------------------------------------------
		//	chunk
		//	--------------------------------------------------
		{
			vector< int > data	=	{ 1, 2, 3, 4, 5, 6 };
			auto chunks			=	chunk( data, 2 );
			
			int chunk_count	=	0;
			for( auto current_chunk : chunks )
			{
				for( int element : current_chunk )
					print( "{} ", element );

				print( "| " );
				++chunk_count;
			}
			println( "" );

			ensure( chunk_count == 3, "should have 3 chunks" );
		}

		//	--------------------------------------------------
		//	count_to
		//	--------------------------------------------------

		//	byte bound
		{
			const vector< int > numbers = count_to( 5 ) | to;
			ensure( numbers.size( ) == 5, "count_to byte bound size failed" );
			ensure( numbers == vector< int >{ 0, 1, 2, 3, 4 }, "count_to byte bound contents failed" );
			println( "test 1 ( byte bound ) passed" );
		}

		//	size_t bound
		{
			const vector< size_t > numbers = count_to( 3uz ) | to;
			ensure( numbers.size( ) == 3, "count_to size_t bound size failed" );
			ensure( numbers == vector< size_t >{ 0, 1, 2 }, "count_to size_t bound contents failed" );
			println( "test 2 ( size_t bound ) passed" );
		}

		//	zero produces an empty range
		{
			const vector< int > numbers = count_to( 0 ) | to;
			ensure( numbers.empty( ), "count_to zero bound failed" );
			println( "test 3 ( zero bound ) passed" );
		}

		//	composable with transform
		{
			const vector< int > squares = count_to( 4 ) | lazy_transform( [ ]( int value ) { return value * value; } ) | to;
			ensure( squares == vector< int >{ 0, 1, 4, 9 }, "count_to with transform failed" );
			println( "test 4 ( with transform ) passed" );
		}

		//	--------------------------------------------------
		//	fold_left_first
		//	--------------------------------------------------

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

		//	--------------------------------------------------
		//	operators
		//	--------------------------------------------------

		//	eager: fixed-size container (array)
		{
			const array< int, 2 > left{ 1, 2 };
			const array< int, 2 > right{ 3, 4 };

			const auto added = left + right;
			ensure( added[ 0 ] == 4 and added[ 1 ] == 6, "eager array + array failed" );

			const auto scaled = left * 2;
			ensure( scaled[ 0 ] == 2 and scaled[ 1 ] == 4, "eager array * scalar failed" );

			const auto reverse = 10 - left;
			ensure( reverse[ 0 ] == 9 and reverse[ 1 ] == 8, "eager scalar - array failed" );

			const auto negated = -left;
			ensure( negated[ 0 ] == -1 and negated[ 1 ] == -2, "eager unary minus failed" );

			const auto remainder = array< int, 2 >{ 7, 8 } % 3;
			ensure( remainder[ 0 ] == 1 and remainder[ 1 ] == 2, "eager array % scalar failed" );
		}

		//	eager: resizable container (vector)
		{
			vector< int > subject{ 1, 2, 3 };

			const auto doubled = subject * 2;
			ensure( doubled.size( ) == 3 and doubled[ 2 ] == 6, "eager vector * scalar failed" );

			subject += 10;
			ensure( subject[ 0 ] == 11 and subject[ 2 ] == 13, "eager vector += scalar failed" );

			const vector< int > increment{ 1, 1, 1 };
			subject += increment;
			ensure( subject[ 1 ] == 13, "eager vector += vector failed" );

			const auto difference = subject - vector< int >{ 10, 10, 10 };
			ensure( difference[ 0 ] == 2 and difference[ 2 ] == 4, "eager vector - vector failed" );
		}

		//	lazy: view arithmetic stays a view until consumed
		{
			const vector< int > base{ 1, 2, 3 };
			const auto boosted = base | lazy_transform( [ ]( int value ) { return value * 10; } );

			const auto sum = fold_left( boosted + base, 0, ::std::plus{ } );
			ensure( sum == 66, "lazy view + vector sum failed" );

			const auto product = fold_left( boosted * 2, 0, ::std::plus{ } );
			ensure( product == 120, "lazy view * scalar sum failed" );

			const auto materialized = array< int, 3 >{ } = ( boosted + base ) | to;
			ensure( materialized[ 0 ] == 11 and materialized[ 2 ] == 33, "lazy to materialization failed" );
		}

		//	composition: eager and lazy mixing, scalar broadcast both sides
		{
			const array< int, 2 > a{ 1, 2 };
			const array< int, 2 > b{ 3, 4 };

			const auto expression = ( a + b ) * 2;
			ensure( expression[ 0 ] == 8 and expression[ 1 ] == 12, "eager composition failed" );

			const auto offset = 100 + ( a * b );
			ensure( offset[ 0 ] == 103 and offset[ 1 ] == 108, "scalar-left composition failed" );
		}

		//	point stays eager (generic operators must not couple to point)
		{
			const point< int, 3 > left{ 1, 2, 3 };
			const point< int, 3 > right{ 4, 5, 6 };

			const auto result = left + right;
			ensure( is_point< decltype( result ) >, "point + point lost eager point semantics" );
			ensure( result[ 0 ] == 5 and result[ 2 ] == 9, "point + point arithmetic failed" );
		}

		//	string concatenation stays standard (generic operators must not hijack it)
		{
			const string head = "ab";
			const string tail = "cd";

			const string joined = head + tail;
			ensure( joined == "abcd", "string + string was hijacked by generic operators" );
		}

		println( "all tests for sak/ranges passed" );
	}
	catch( const exception& error )
	{
		println( "test failed: {}", error.what( ) );
		return	exit_failure;
	}

	return	exit_success;
}


