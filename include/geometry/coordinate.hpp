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
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/* 
 * File:   geometry/coordinate.hpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-10-16 14:02
 */



#pragma once
#ifndef header_guard_581074281
#define header_guard_581074281


#include <array>
#include <concepts>
#include <algorithm>
#include <ranges>
#include <functional>
#include <expected>
#include <cstddef>	//	size_t
#include <cmath>	//	sqrt
#include <compare>


#define __581074281_compound_operator( a_operator, a_math_operation, a_array_operation, a_scalar_operation ) \
inline auto operator a_operator##= ( const coordinate& other ) noexcept { return a_array_operation( other, a_math_operation<>{} ); } \
inline auto operator a_operator##= ( t_scalar other ) noexcept { return a_scalar_operation( other, a_math_operation<>{} ); } \


namespace geometry {


//	--------------------------------------------------
using	::std::array;
//	--------------------------------------------------
using	::std::convertible_to;
//	--------------------------------------------------
using	::std::size_t;
using	::std::views::zip;
using	::std::ranges::for_each;
using	::std::ranges::any_of;
using	::std::ranges::all_of;
//	--------------------------------------------------
using	::std::expected;
using	::std::unexpected;
//	--------------------------------------------------
using	::std::plus;
using	::std::minus;
using	::std::multiplies;
using	::std::divides;
using	::std::modulus;
using	::std::sqrt;
//	--------------------------------------------------

namespace math {


enum class error
{
	 division_by_zero
	,arithmetic_overflow
	,square_root_of_negative
};


using	::std::is_arithmetic_v;
template< typename t_arithmetic >
concept arithmetic = is_arithmetic_v< t_arithmetic >;

template< arithmetic t_scalar >
constexpr auto square( t_scalar value ) noexcept { return value * value; }


}


using	::geometry::math::error::division_by_zero;


template< typename t_scalar = int, size_t num_dimensions = 2 >
class coordinate : public array< t_scalar, num_dimensions >
{
public:
	using	super_type = array< t_scalar, num_dimensions >;
	using	math_result = expected< coordinate, math_error >;

	template< typename... t_args >
		requires( sizeof...( t_args ) == num_dimensions
			and ( convertible_to< t_args, t_scalar > and ... )
		)
	constexpr coordinate( t_args... a_args )
		: super_type{ static_cast< t_scalar >( a_args )... }
	{ }
	

	__581074281_compound_operator( +	,plus		,apply_array_operation		,apply_scalar_operation			)
	__581074281_compound_operator( -	,minus		,apply_array_operation		,apply_scalar_operation			)
	__581074281_compound_operator( *	,multiplies	,apply_array_operation		,apply_scalar_operation			)
	__581074281_compound_operator( /	,divides	,apply_safe_array_operation	,apply_safe_scalar_operation	)
	__581074281_compound_operator( %	,modulus	,apply_safe_array_operation	,apply_safe_scalar_operation	)


	auto is_inside( const coordinate& other ) const noexcept -> bool
	{
		return	all_of( zip( *this, other ), [ ]( const auto& pair ){
			const auto& [ left, right ] = pair;
			return	left <= right;
		} );
	}

	auto get_length( ) const noexcept -> t_scalar
	{
		t_scalar sum_of_squares	=	0;
		for( const auto& value : *this )
			sum_of_squares += value * value;
		return	static_cast< t_scalar >( sqrt( sum_of_squares ) );
	}

private:
	template< typename t_function >
	auto apply_array_operation( const coordinate& other, const t_function& operation ) -> coordinate&
	{
		for( auto&& [ left, right ] : zip( *this, other ) )
			left = operation( left, right );
		return	*this;
	}

	template< typename t_function >
	auto apply_scalar_operation( t_scalar other, const t_function& operation ) -> coordinate&
	{
		for_each( *this, [ & ]( t_scalar& value ){ value = operation( value, other ); } );
		return	*this;
	}

	auto is_safe_denominator( ) const noexcept -> bool
	{
		return	not any_of( *this, [ ]( const auto& value ){ return value == 0; } );
	}

	template< typename t_function >
	auto apply_safe_array_operation( const coordinate& other, const t_function& operation ) -> math_result
	{
		if( not other.is_safe_denominator( ) )
			return	unexpected( division_by_zero );
		return	apply_array_operation( other, operation );
	}

	template< typename t_function >
	auto apply_safe_scalar_operation( t_scalar other, const t_function& operation ) -> math_result
	{
		if( other == 0 )
			return	unexpected( division_by_zero );
		return	apply_scalar_operation( other, operation );
	}


};


}


#define __581074281_binary_operator( a_operator ) \
template< typename type_t, typename other_t > \
inline constexpr auto operator a_operator ( type_t left, const other_t& right ) noexcept { return left a_operator##= right; };


__581074281_binary_operator( + )
__581074281_binary_operator( - )
__581074281_binary_operator( * )
__581074281_binary_operator( / )
__581074281_binary_operator( % )


#endif


