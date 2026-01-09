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
 * File:   adhoc/0007_point_ranges/0007_point_ranges.cpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2026-01-08 02:23
 */


#include <print>
#include <string>
#include <vector>
#include <exception>
#include <sak/ensure.hpp>
#include <sak/using.hpp>
#include <sak/geometry/point.hpp>
#include <sak/math/math.hpp>


auto main( const int a_argument_count, const char* a_argument_values[ ] ) -> int
{{
	__using( ::sak::
		,exit_success
		,exit_failure
		,ensure
		,point
		,to_point
	)
	__using( ::std::
		,string
		,vector
		,println
		,exception
	)
	__using( ::sak::math::, abs, sign, sum )

	const vector< string > arguments( a_argument_values, a_argument_values + a_argument_count );
	for( const auto& value : arguments )
		println( "{}", value );
	
	try
	{
		println( "starting tests for: point_ranges" );

		using	point_type	=	point< int, 3 >;
		const	point_type	p( -10, 20, -30 );

		//	demonstration of desired syntax
		//	p | abs: returns a view (lazy)
		//	| sign: nests another view (lazy)
		//	| to_point: returns a __point_from proxy (lazy)
		//	the assignment to point_type triggers the conversion operator (eager)
		const	point_type	result	=	p | abs | sign | to_point;

		//	verification
		ensure( result == point_type( 1, 1, 1 ), "lazy pipe composition failed" );
		
		//	proving it is lazy: we can use it in range algorithms without to_point
		const	auto	sum_abs	=	sum( p | abs );
		ensure( sum_abs == 10 + 20 + 30, "lazy evaluation in sum failed" );

		println( "result of p | abs | sign | to_point: [ {}, {}, {} ]", result[ 0 ], result[ 1 ], result[ 2 ] );
		println( "sum of p | abs (without to_point):   {}", sum_abs );

		println( "all tests for point_ranges passed" );
	}
	catch( const exception& error )
	{
		println( "test failed: {}", error.what( ) );
		return	exit_failure;
	}

	return	exit_success;
}}


