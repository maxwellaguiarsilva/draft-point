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
 * Created on 2026-01-08 01:27
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


namespace demo {

__using( ::std::, forward, remove_cvref_t, invocable )
__using( ::std::ranges::, input_range, copy, range_value_t )

//	1. O "conversor" final
template< typename t_point >
struct to_point_t
{
	template< input_range t_range >
	friend constexpr auto operator | ( t_range&& a_range, const to_point_t& ) -> t_point
	{
		t_point	result;
		copy( a_range, result.begin( ) );
		return	result;
	}
};

template< typename t_point >
inline constexpr to_point_t< t_point > to_point{ };

}


//	Colocamos no namespace sak para que o ADL encontre os operadores
//	quando usados com tipos de sak:: geometry ou sak::math
namespace sak {

__using( ::std::, remove_cvref_t, invocable )
__using( ::std::ranges::, input_range, range_value_t )

//	2. Sobrecarga para Point | Invocable -> TransformView (Lazy)
template< is_point t_point, invocable< typename remove_cvref_t< t_point >::value_type > t_operation >
constexpr auto operator | ( t_point&& a_point, t_operation&& a_operation )
{
	using	::std::views::transform;
	return	transform( ::std::forward< t_point >( a_point ), ::std::forward< t_operation >( a_operation ) );
}

//	3. Sobrecarga para View | Invocable -> TransformView (Lazy)
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
	__using( ::demo::, to_point )

	const vector< string > arguments( a_argument_values, a_argument_values + a_argument_count );
	for( const auto& value : arguments )
		println( "{}", value );
	
	try
	{
		println( "starting tests for: point_ranges..." );

		using	point_type	=	point< int, 3 >;
		const	point_type	p( -10, 20, -30 );

		//	DEMONSTRAÇÃO DA SINTAXE DESEJADA:
		//	- p | abs: retorna uma view (lazy)
		//	- | sign: aninha outra view (lazy)
		//	- | to_point: executa a composição e converte para point (eager)
		const	point_type	result	=	p | abs | sign | to_point< point_type >;

		//	Verificação
		ensure( result == point_type( 1, 1, 1 ), "lazy pipe composition failed" );
		
		//	Provando que é lazy: podemos usar em algoritmos de range sem o to_point
		const	auto	soma_abs	=	sum( p | abs );
		ensure( soma_abs == 10 + 20 + 30, "lazy evaluation in sum failed" );

		println( "Result of p | abs | sign | to_point: [ {}, {}, {} ]", result[ 0 ], result[ 1 ], result[ 2 ] );
		println( "Sum of p | abs (without to_point):   {}", soma_abs );

		println( "all tests for point_ranges passed!" );
	}
	catch( const exception& error )
	{
		println( "test failed: {}", error.what( ) );
		return	exit_failure;
	}

	return	exit_success;
}}


