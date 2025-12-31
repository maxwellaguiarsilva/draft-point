/*
 * Copyright ( C ) 2025 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
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
 * You should have received a copy of the GNU General License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/* 
 * File:   test_sak_geometry_coordinate.cpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-12-26 17:43:29
 */



//	--------------------------------------------------
#include <print>
#include <string>
#include <vector>
#include <exception>
#include <sak/ensure.hpp>
#include <sak/geometry/geometry.hpp>
#include <sak/math/math.hpp>
//	--------------------------------------------------


auto main( const int argument_count, const char* argument_values[ ] ) -> int
{{
	using	::std::string;
	using	::std::vector;
	using	::std::println;
	using	::std::exception;
	using	::sak::ensure;
	using	::sak::math::sum;
	using	::sak::geometry::coordinate;

	const vector< string > arguments( argument_values, argument_values + argument_count );
	for( const auto& value : arguments )
		println( "{}", value );
	
	try
	{
		using	point = coordinate< int, 2 >;

		println( "starting tests for class coordinate..." );

		//	test 1: default constructor and access
		{
			point point_a;
			ensure( point_a[ 0 ] == 0 and point_a[ 1 ] == 0, "test 1: default constructor failed" );
			println( "test 1 (default constructor) passed" );
		}

		//	test 2: constructor with parameters
		{
			point point_a( 10, 20 );
			ensure( point_a[ 0 ] == 10 and point_a[ 1 ] == 20, "test 2: constructor with parameters failed" );
			println( "test 2 (constructor with parameters) passed" );
		}

		//	test 3: arithmetic operations (+, -)
		{
			point point_a( 1, 2 );
			point point_b( 3, 4 );
			point point_c = point_a + point_b;
			ensure( point_c[ 0 ] == 4 and point_c[ 1 ] == 6, "test 3: addition failed" );

			point point_d = point_b - point_a;
			ensure( point_d[ 0 ] == 2 and point_d[ 1 ] == 2, "test 3: subtraction failed" );
			println( "test 3 (addition and subtraction) passed" );
		}

		//	test 4: operations with scalars (*, /, %)
		{
			point point_a( 10, 20 );
			point point_b = point_a * 2;
			ensure( point_b[ 0 ] == 20 and point_b[ 1 ] == 40, "test 4: multiplication failed" );

			point point_c = point_a / 2;
			ensure( point_c[ 0 ] == 5 and point_c[ 1 ] == 10, "test 4: division failed" );

			point point_d = point_a % 3;
			ensure( point_d[ 0 ] == 1 and point_d[ 1 ] == 2, "test 4: modulo failed" );
			println( "test 4 (operations with scalars) passed" );
		}

		//	test 5: compound assignment
		{
			point point_a( 1, 1 );
			point_a += point( 2, 3 );
			ensure( point_a[ 0 ] == 3 and point_a[ 1 ] == 4, "test 5: compound addition failed" );

			point_a *= 2;
			ensure( point_a[ 0 ] == 6 and point_a[ 1 ] == 8, "test 5: compound multiplication failed" );
			println( "test 5 (compound assignment) passed" );
		}

		//	test 6: encloses
		{
			point point_a( 0, 0 );
			point point_b( 5, 5 );
			point point_c( 10, 10 );

			ensure( point_a.encloses( point_b ), "test 6: encloses 1 failed" );
			ensure( point_b.encloses( point_c ), "test 6: encloses 2 failed" );
			ensure( not point_c.encloses( point_b ), "test 6: encloses 3 failed" );
			println( "test 6 (encloses) passed" );
		}

		//	test 7: get_length
		{
			point point_a( 3, 4 );
			ensure( point_a.get_length( ) == 5, "test 7: get_length failed" );
			println( "test 7 (get_length - int) passed" );
		}

		//	test 8: float coordinate
		{
			using float_point = coordinate< float, 3 >;
			float_point float_point_a( 1.0f, 2.0f, 2.0f );
			
			//	length of (1, 2, 2) is sqrt(1+4+4) = sqrt(9) = 3
			ensure( float_point_a.get_length( ) == 3.0f, "test 8: float get_length failed" );

			float_point float_point_b = float_point_a * 0.5f;
			ensure( float_point_b[ 0 ] == 0.5f and float_point_b[ 1 ] == 1.0f and float_point_b[ 2 ] == 1.0f, "test 8: float multiplication failed" );
			println( "test 8 (float coordinate and get_length) passed" );
		}

		//	test 9: iterators
		{
			point point_a( 10, 20 );
			ensure( sum( point_a ) == 30, "test 9: sum failed" );
			println( "test 9 (iterators/sum) passed" );
		}

		//	test 10: left-hand scalar
		{
			point point_a( 1, 2 );
			point point_b = 10 + point_a;
			ensure( point_b[ 0 ] == 11 and point_b[ 1 ] == 12, "test 10: left-hand addition failed" );

			point point_c = 20 - point_a;
			ensure( point_c[ 0 ] == 19 and point_c[ 1 ] == 18, "test 10: left-hand subtraction failed" );
			println( "test 10 (left-hand scalar) passed" );
		}

		println( "all tests for class coordinate passed successfully!" );
	}
	catch( const exception& error )
	{
		println( "test failed: {}", error.what( ) );
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}};


