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
 * File:   sak/math/math.hpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-12-19 18:56
 */


#pragma once
#ifndef header_guard_935812590
#define header_guard_935812590


#include <type_traits>
#include <exception>
#include <ranges>
#include <functional>
#include <algorithm>
#include <utility>
#include <cmath>


namespace sak {
namespace math {


inline constexpr auto plus			= 	::std::plus{ };
inline constexpr auto minus			= 	::std::minus{ };
inline constexpr auto multiplies	= 	::std::multiplies{ };
inline constexpr auto divides		= 	::std::divides{ };
inline constexpr auto modulus		= 	::std::modulus{ };
inline constexpr auto equal_to		= 	::std::equal_to{ };
inline constexpr auto less_equal	= 	::std::less_equal{ };
inline constexpr auto greater_equal	= 	::std::greater_equal{ };


enum class error
{
	 division_by_zero
	,arithmetic_overflow
	,square_root_of_negative
};


struct exception : ::std::exception
{
	error m_error;
	constexpr explicit exception( error a_error ) : m_error( a_error ) { }
	
	const char* what( ) const noexcept override
	{
		switch( m_error )
		{
			case error::division_by_zero: return "math: division by zero";
			case error::arithmetic_overflow: return "math: arithmetic overflow";
			case error::square_root_of_negative: return "math: square root of negative number";
		}
		return	"math: unknown error";
	}
};


using	::std::is_arithmetic_v;
template< typename t_arithmetic >
concept is_arithmetic = is_arithmetic_v< t_arithmetic >;


struct __between
{
	constexpr auto operator ( ) ( auto value, auto start, auto end ) const noexcept -> bool
	{
		return	value >= start and value <= end;
	}
};
inline constexpr auto between = __between{ };


using	::std::ranges::fold_left;
struct __sum
{
	constexpr auto operator ( ) ( const auto& list ) const noexcept { return fold_left( list, 0, plus ); }
};
inline constexpr auto sum = __sum{ };


struct __square
{
	constexpr auto operator ( ) ( auto value ) const noexcept { return value * value; }
};
inline constexpr auto square = __square{ };


struct __square_root
{
	constexpr auto operator ( ) ( const auto& value ) const noexcept { return sqrt( value ); }
};
inline constexpr auto square_root = __square_root{ };


struct __is_multiple
{ constexpr auto operator ( ) ( auto left, auto right ) const noexcept { return modulus( left, right ) == 0; } };
inline constexpr auto is_multiple = __is_multiple{ };
struct __is_even
{ constexpr auto operator ( ) ( auto value ) const noexcept { return is_multiple( value, 2 ); } };
inline constexpr auto is_even = __is_even{ };
struct __is_odd
{ constexpr auto operator ( ) ( auto value ) const noexcept { return not is_even( value ); } };
inline constexpr auto is_odd = __is_odd{ };


struct __abs
{
	constexpr auto operator ( ) ( auto value ) const noexcept { return value < 0 ? -value : value; }
};
inline constexpr auto abs = __abs{ };


struct __sign
{
	constexpr auto operator ( ) ( auto value ) const noexcept { return ( value > 0 ) - ( value < 0 ); }
};
inline constexpr auto sign = __sign{ };


} }


#endif


