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
#include <ranges>
#include <algorithm>
#include <sak/ensure.hpp>
#include <sak/using.hpp>
#include <sak/geometry/point.hpp>
#include <sak/math/math.hpp>


namespace sak {

__using( ::std::
	,declval
	,forward
	,integral_constant
	,remove_cvref_t
	,size_t
)
__using( ::std::ranges::
	,copy
	,input_range
	,owning_view
	,range_value_t
	,ref_view
)
__using( ::sak::math::, is_arithmetic )

//	trait to extract the dimension by peeling views
template< typename t_view >
struct __point_dimension;

template< is_arithmetic t_scalar, size_t num_size >
struct __point_dimension< point< t_scalar, num_size > >
	: integral_constant< size_t, num_size > { };

template< typename t_range >
struct __point_dimension< ref_view< t_range > >
	: __point_dimension< remove_cvref_t< t_range > > { };

template< typename t_range >
struct __point_dimension< owning_view< t_range > >
	: __point_dimension< remove_cvref_t< t_range > > { };

template< typename t_view >
requires requires { declval< t_view >( ).base( ); }
struct __point_dimension< t_view >
	: __point_dimension< remove_cvref_t< decltype( declval< t_view >( ).base( ) ) > > { };

struct __to_point { };
inline constexpr __to_point to_point{ };

template< input_range t_range >
constexpr auto operator | ( t_range&& a_range, __to_point )
{
	constexpr size_t num_size = __point_dimension< remove_cvref_t< t_range > >::value;
	using	t_scalar	=	range_value_t< t_range >;

	point< t_scalar, num_size >	result;
	copy( a_range, result.begin( ) );
	return	result;
}

}


//	putting in sak namespace so adl finds operators
//	when used with sak::geometry or sak::math types
namespace sak {

__using( ::std::, remove_cvref_t, invocable )
__using( ::std::ranges::, input_range, range_value_t )

//	overload for point | invocable -> transform_view (lazy)
template< is_point t_point, invocable< typename remove_cvref_t< t_point >::value_type > t_operation >
constexpr auto operator | ( t_point&& a_point, t_operation&& a_operation )
{
	using	::std::views::transform;
	return	transform( ::std::forward< t_point >( a_point ), ::std::forward< t_operation >( a_operation ) );
}

//	overload for view | invocable -> transform_view (lazy)
template< input_range t_range, invocable< range_value_t< t_range > > t_operation >
requires ( not is_point< remove_cvref_t< t_range > > )
constexpr auto operator | ( t_range&& a_range, t_operation&& a_operation )
{
	using	::std::views::transform;
	return	transform( ::std::forward< t_range >( a_range ), ::std::forward< t_operation >( a_operation ) );
}

}


auto main( const int a_argument_count, const char* a_argument_values[ ] ) -> int
{{
	__using( ::sak::
		,exit_success
		,exit_failure
		,ensure
		,point
	)
	__using( ::std::
		,string
		,vector
		,println
		,exception
	)
	__using( ::sak::math::, abs, sign, square, sum )
	__using( ::sak::, to_point )

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
		//	| to_point: executes composition and converts to point (eager)
		//	the dimension and type are now automatically deduced from the range lineage
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


