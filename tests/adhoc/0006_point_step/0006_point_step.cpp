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
 * Created on 2026-01-06 14:50
 */


#include <print>
#include <string>
#include <vector>
#include <exception>
#include <cmath>
#include <ranges>
#include <algorithm>
#include <sak/geometry/point.hpp>
#include <sak/ensure.hpp>
#include <sak/using.hpp>


//	--------------------------------------------------
using	point_2d	= 	::sak::point< int, 2 >;
using	point_3d	= 	::sak::point< int, 3 >;
//	--------------------------------------------------


template< typename t_point >
void step( t_point& a_point )
{
	__using( ::std::views::, transform )
	__using( ::sak::math::, square, sum, square_root )
	__using( ::std::, round, max )
	using	t_scalar	= 	typename t_point::value_type;

	const auto	length	= 	square_root( static_cast< double >( sum( transform( a_point, square ) ) ) );
	const auto	factor	= 	length > 0.0 ? max( 0.0, ( length - 1.0 ) / length ) : 0.0;

	if( length > 0.0 )
		a_point = a_point.map( [ factor ] ( const auto& a_scalar ) {
			return	static_cast< t_scalar >( round( a_scalar * factor ) );
		} );
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

	const vector< string >	arguments( a_argument_values, a_argument_values + a_argument_count );
	for( const auto& value : arguments )
		println( "{}", value );
	
	try
	{
		println( "starting tests for: point_step..." );

		//	test 2d: axis aligned
		point_2d	point_2d_axis( 10, 0 );
		step( point_2d_axis );
		ensure( point_2d_axis == point_2d( 9, 0 ), "2d axis step failed" );
		
		//	test 2d: diagonal
		point_2d	point_2d_diagonal( 5, 5 );
		step( point_2d_diagonal );
		ensure( point_2d_diagonal == point_2d( 4, 4 ), "2d diagonal step failed" );

		//	test 3d: generic dimensions
		point_3d	point_3d_generic( 10, 10, 10 );
		step( point_3d_generic );
		ensure( point_3d_generic == point_3d( 9, 9, 9 ), "3d generic step failed" );
		
		//	test: reaching zero
		point_2d	point_2d_small( 1, 0 );
		step( point_2d_small );
		ensure( point_2d_small == point_2d( 0, 0 ), "reaching zero failed" );

		//	test 3d: path to zero
		point_3d	point_3d_path( 11, 13, 17 );
		const point_3d	point_3d_zero( 0, 0, 0 );

		println( "path for {{ 11, 13, 17 }} to zero:" );
		while( not ( point_3d_path == point_3d_zero ) )
		{
			println( "  - {{ {}, {}, {} }}", point_3d_path[ 0 ], point_3d_path[ 1 ], point_3d_path[ 2 ] );
			step( point_3d_path );
		}
		println( "  - {{ {}, {}, {} }}", point_3d_path[ 0 ], point_3d_path[ 1 ], point_3d_path[ 2 ] );
		ensure( point_3d_path == point_3d_zero, "path to zero failed" );

		println( "all tests for point_step passed!" );
	}
	catch( const exception& error )
	{
		println( "test failed: {}", error.what( ) );
		return	exit_failure;
	}

	return	exit_success;
}}


