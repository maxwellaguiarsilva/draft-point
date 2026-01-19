//	
//	Copyright (C) 2025 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
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
//	File:   include/sak/math/math.hpp
//	Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	
//	Created on 2025-12-23 19:05:44
//


#pragma once
#ifndef header_guard_935812590
#define header_guard_935812590


#include <concepts>
#include <ranges>
#include <exception>
#include <functional>
#include <cmath>
#include <algorithm>


namespace sak {
namespace math {


inline constexpr auto plus			= 	::std::plus{ };
inline constexpr auto minus			= 	::std::minus{ };
inline constexpr auto negate		= 	::std::negate{ };
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
	,sqrt_of_negative
};


struct exception : ::std::exception
{
	error m_error;
	constexpr explicit exception( error error_code ) : m_error( error_code ) { }
	
	const char* what( ) const noexcept override
	{
		switch( m_error )
		{
			case error::division_by_zero: return "math: division by zero";
			case error::arithmetic_overflow: return "math: arithmetic overflow";
			case error::sqrt_of_negative: return "math: square root of negative number";
		}
		return	"math: unknown error";
	}
};


using	::std::is_arithmetic_v;
using	::std::bind_front;
using	::std::bind_back;
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


#define __935812590_unary( a_name ) \
struct __##a_name \
{ \
	constexpr auto operator ( ) ( auto a_value ) const noexcept { return ::std::a_name( a_value ); } \
}; \
inline constexpr auto a_name = __##a_name{ };


#define __935812590_binary( a_name ) \
struct __##a_name \
{ \
	constexpr auto operator ( ) ( auto a_left, auto a_right ) const noexcept { return ::std::a_name( a_left, a_right ); } \
}; \
inline constexpr auto a_name = __##a_name{ };


__935812590_unary( acos )
__935812590_unary( asin )
__935812590_unary( atan )
__935812590_unary( ceil )
__935812590_unary( cos )
__935812590_unary( exp )
__935812590_unary( floor )
__935812590_unary( log )
__935812590_unary( log2 )
__935812590_unary( round )
__935812590_unary( sin )
__935812590_unary( sqrt )
__935812590_unary( tan )
__935812590_unary( trunc )
__935812590_binary( atan2 )
__935812590_binary( pow )


#undef __935812590_unary
#undef __935812590_binary


struct __min
{
	constexpr auto operator ( ) ( auto left, auto right ) const noexcept { return left < right ? left : right; }
	constexpr auto operator ( ) ( auto&& range ) const noexcept { return ::std::ranges::min( range ); }
};
inline constexpr auto min = __min{ };


struct __max
{
	constexpr auto operator ( ) ( auto left, auto right ) const noexcept { return left > right ? left : right; }
	constexpr auto operator ( ) ( auto&& range ) const noexcept { return ::std::ranges::max( range ); }
};
inline constexpr auto max = __max{ };


struct __clamp
{
	constexpr auto operator ( ) ( auto value, auto low, auto high ) const noexcept
	{
		return	min( max( value, low ), high );
	}
};
inline constexpr auto clamp = __clamp{ };


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


