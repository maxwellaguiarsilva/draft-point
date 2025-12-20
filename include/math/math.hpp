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
 * File:   math/math.hpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-12-19 18:56
 */



#pragma once
#ifndef header_guard_935812590
#define header_guard_935812590


#include <type_traits>
#include <exception>


namespace math {


enum class error
{
	 division_by_zero
	,arithmetic_overflow
	,square_root_of_negative
};


struct exception : ::std::exception
{
	error m_error;
	constexpr exception( error a_error ) : m_error( a_error ) { }
	
	const char* what( ) const noexcept override
	{
		switch( m_error )
		{
			case error::division_by_zero: return "math: division by zero";
			case error::arithmetic_overflow: return "math: arithmetic overflow";
			case error::square_root_of_negative: return "math: square root of negative number";
		}
		return "math: unknown error";
	}
};


using	::std::is_arithmetic_v;
template< typename t_arithmetic >
concept arithmetic = is_arithmetic_v< t_arithmetic >;


struct __square
{
	template< arithmetic t_scalar >
	constexpr inline auto operator () ( t_scalar value ) const noexcept { return value * value; }
};
inline constexpr auto square = __square( );


}


#endif