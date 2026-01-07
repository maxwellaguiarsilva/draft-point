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
 * File:   adhoc/0006_point_step/0006_point_step.cpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2026-01-06 14:53
 */


#include <print>
#include <string>
#include <vector>
#include <exception>
#include <limits>
#include <cmath>
#include <ranges>
#include <algorithm>
#include <format>
#include <sak/geometry/point.hpp>
#include <sak/ensure.hpp>
#include <sak/using.hpp>


//	--------------------------------------------------
__using( ::std::, numeric_limits )
__using( ::sak::, is_point, point )
__using( ::std::ranges::, max )
//	--------------------------------------------------


template< typename t_point >
requires	is_point< t_point >
auto to_string( const t_point& a_point ) -> ::std::string
{
	__using( ::std::, format, string );
	__using( ::std::views::, transform, join );

	return	format( "|{} |", a_point 
		| transform( [ ]( const auto a_value ) { return format( " {}", a_value ); } ) 
		| join
		| ::std::ranges::to< string >( ) 
	);
}


auto main( const int a_argument_count, const char* a_argument_values[ ] ) -> int
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

	__using( ::sak::math::, sign, abs );

	const vector< string >	arguments( a_argument_values, a_argument_values + a_argument_count );
	for( const auto& value : arguments )
		println( "{}", value );
	
	try
	{
		println( "starting tests for: point_step..." );

		using	point	=	point< int, 3 >;

		//	test 3d: path to zero
		point	pixel( 11, 13, 17 );
		const	point	zero( 0, 0, 0 );
		const	auto	total	=	max( pixel );

		const	point	step	=	pixel.map( sign ) * -1;	//	{ -1, -1, -1};

		const	point	walker_step	=	pixel.map( abs );
		point	walker	=	walker_step;
		
		auto	is_greater_equal	=	( [ & ]( auto value ){ return value >= total; } );
		point	direction	=	walker.map( is_greater_equal );
		point	current	=	pixel;

		auto	count	=	total;
		while( --count >= 0 )
		{
			println( "current: {}", to_string( current ) );
			direction	=	walker.map( is_greater_equal );
			current	+=	step * direction;
			walker	+=	walker_step - direction * total;
		}

		ensure( current != zero, "path to zero failed" );

		println( "all tests for point_step passed!" );
	}
	catch( const exception& error )
	{
		println( "test failed: {}", error.what( ) );
		return	exit_failure;
	}

	return	exit_success;
}}


