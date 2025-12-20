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
#include <cstddef>	//	 size_t
#include <cmath>	//	 sqrt
#include <math/math.hpp>
#include <pattern/tupled.hpp>


#define __581074281_compound_operator( a_operator, a_math_operation, a_check, a_no_except ) \
inline constexpr auto operator a_operator##= ( const coordinate& other ) a_no_except -> coordinate& { return apply_operation( other, other, a_math_operation, a_check ); } \
inline constexpr auto operator a_operator##= ( t_scalar other ) a_no_except -> coordinate& { return apply_operation( repeat( other ), other, a_math_operation, a_check ); } \


#define __581074281_export_m_data_method( a_method ) \
constexpr auto a_method( ) noexcept { return m_data.a_method( ); } \
constexpr auto a_method( ) const noexcept { return m_data.a_method( ); }



namespace geometry {


//	--------------------------------------------------
using	::std::array;
//	--------------------------------------------------
using	::std::convertible_to;
using	::std::invocable;
using	::std::same_as;
//	--------------------------------------------------
using	::std::size_t;
using	::pattern::tupled;
using	::std::bind_back;
using	::std::views::repeat;
using	::std::views::zip;
using	::std::ranges::transform;
using	::std::ranges::any_of;
using	::std::ranges::all_of;
using	::std::ranges::fold_left;
//	--------------------------------------------------
using	::std::sqrt;
inline constexpr auto plus			= 	::std::plus( );
inline constexpr auto minus			= 	::std::minus( );
inline constexpr auto multiplies	= 	::std::multiplies( );
inline constexpr auto divides		= 	::std::divides( );
inline constexpr auto modulus		= 	::std::modulus( );
inline constexpr auto equal_to		= 	::std::equal_to( );
inline constexpr auto less_equal	= 	tupled{ ::std::less_equal( ) };
//	--------------------------------------------------


using	::math::square;
using	::math::arithmetic;
using	::math::error::division_by_zero;


struct __unsafe_denominator {
	template< typename T >
	inline constexpr auto operator()( const T& ) const noexcept -> void { } 
};
inline constexpr auto unsafe_denominator = __unsafe_denominator{ };

struct __safe_denominator {
	template< typename T >
	inline constexpr auto operator()( const T& value ) const -> void { check( value ); } 
private:
	template< typename T >
	inline constexpr auto check( const T& value ) const -> void requires requires { value.is_safe_denominator( ); } {
		if ( not value.is_safe_denominator( ) ) throw ::math::exception{ division_by_zero };
	}
	template< typename T >
	inline constexpr auto check( const T& value ) const -> void {
		if ( value == 0 ) throw ::math::exception{ division_by_zero };
	}
};
inline constexpr auto safe_denominator = __safe_denominator{ };


template< typename T >
concept denominator_security_checker = same_as< T, __unsafe_denominator > or same_as< T, __safe_denominator >; 


template< arithmetic t_scalar = int, size_t num_dimensions = 2 >
class coordinate
{
public:
	using		super_type = array< t_scalar, num_dimensions >; 

	template< typename... t_args >
		requires( sizeof...( t_args ) == num_dimensions
			and ( convertible_to< t_args, t_scalar > and ... )
		)
	constexpr coordinate( t_args... a_args )
		: m_data{ static_cast< t_scalar >( a_args )... }
	{ }
	

	__581074281_compound_operator( + 	,plus		,unsafe_denominator	,noexcept	)
	__581074281_compound_operator( - 	,minus		,unsafe_denominator	,noexcept	)
	__581074281_compound_operator( * 	,multiplies	,unsafe_denominator	,noexcept	)
	__581074281_compound_operator( / 	,divides	,safe_denominator	,			)
	__581074281_compound_operator( % 	,modulus	,safe_denominator	,			)

	__581074281_export_m_data_method( begin )
	__581074281_export_m_data_method( end )

	constexpr auto size( ) const noexcept -> size_t { return num_dimensions; } 
	

	constexpr auto operator []( size_t index ) noexcept -> t_scalar& { return m_data[ index ]; }
	constexpr auto operator []( size_t index ) const noexcept -> const t_scalar& { return m_data[ index ]; }


	constexpr auto is_inside( const coordinate& other ) const noexcept -> bool
	{
		return	all_of( zip( m_data, other.m_data ), less_equal );
	}

	//	truncation may occur depending on the type of scalar chosen
	//	this is acceptable for this project
	constexpr auto get_length( ) const noexcept -> t_scalar
	{
		using	::std::views::transform;
		return	static_cast< t_scalar >( sqrt( fold_left( m_data | transform( square ), 0, plus ) ) );
	}

	constexpr auto is_safe_denominator( ) const noexcept -> bool
	{
		return	not any_of( m_data, bind_back( equal_to, 0 ) );
	}

private:
	array< t_scalar, num_dimensions >			m_data;

	template< typename t_other_data, typename t_check_value, invocable< t_scalar, t_scalar > t_function, denominator_security_checker t_check >
	constexpr auto apply_operation( t_other_data other_data, t_check_value check_value, const t_function& operation, const t_check& check ) -> coordinate&
	{
		check( check_value );
		transform( m_data, other_data, m_data.begin( ), operation );
		return *this;
	}


};


}


#define __581074281_binary_operator( a_operator ) \
template< typename type_t, typename other_t > \
inline constexpr auto operator a_operator ( type_t left, const other_t& right ) { return left a_operator##= right; };


__581074281_binary_operator( + )
__581074281_binary_operator( - )
__581074281_binary_operator( * )
__581074281_binary_operator( / )
__581074281_binary_operator( % )


#endif


