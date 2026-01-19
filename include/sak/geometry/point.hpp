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
//	File:   include/sak/geometry/point.hpp
//	Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	
//	Created on 2025-12-23 19:05:44
//


#pragma once
#ifndef header_guard_352612026
#define header_guard_352612026


#include <array>
#include <ranges>
#include <cstddef>
#include <concepts>
#include <sak/math/math.hpp>
#include <sak/pattern/tupled.hpp>
#include <sak/using.hpp>


namespace sak {


//	--------------------------------------------------
using	::sak::pattern::tupled;
//	--------------------------------------------------
__using( ::std::
	,array
	,convertible_to
	,invocable
	,size_t
	,true_type
	,false_type
	,remove_cvref_t
)
__using( ::std::ranges::
	,copy
	,input_range
	,range_value_t
)
__using( ::sak::math::
	,plus
	,minus
	,negate
	,multiplies
	,divides
	,modulus
	,sum
	,square
	,sqrt
	,equal_to
	,less_equal
	,greater_equal
	,is_arithmetic
	,fold_left
)
//	--------------------------------------------------


template< is_arithmetic t_scalar, size_t num_dimensions >
requires	( num_dimensions > 1 )
class point;
template< typename t_point >
struct __is_point : false_type { };
template< is_arithmetic t_scalar, size_t num_dimensions >
struct __is_point< point< t_scalar, num_dimensions > > : true_type { };
template< typename t_point >
concept is_point = __is_point< remove_cvref_t< t_point > >::value;


//	--------------------------------------------------
//	proxy to handle conversion from range to point
//	it does not support auto deduction, forcing a strong type definition
template< input_range t_range >
struct __point_from
{
	t_range m_range;

	template< is_arithmetic t_scalar, size_t num_size >
	constexpr operator point< t_scalar, num_size >( ) &&
	{
		point< t_scalar, num_size >	result;
		copy( m_range, result.begin( ) );
		return	result;
	}
};

struct __to_point { };
inline constexpr __to_point to_point{ };

template< input_range t_range >
constexpr auto operator | ( t_range&& subject, __to_point )
{
	return	__point_from< t_range >{ ::std::forward< t_range >( subject ) };
}

//	overload for point | invocable -> transform_view (lazy)
template< is_point t_point, invocable< typename remove_cvref_t< t_point >::value_type > t_operation >
constexpr auto operator | ( t_point&& pixel, t_operation&& operation )
{
	using	::std::views::transform;
	return	transform( ::std::forward< t_point >( pixel ), ::std::forward< t_operation >( operation ) );
}

//	overload for view | invocable -> transform_view (lazy)
template< input_range t_range, invocable< range_value_t< t_range > > t_operation >
requires ( not is_point< remove_cvref_t< t_range > > )
constexpr auto operator | ( t_range&& subject, t_operation&& operation )
{
	using	::std::views::transform;
	return	transform( ::std::forward< t_range >( subject ), ::std::forward< t_operation >( operation ) );
}
//	--------------------------------------------------


#define __352612026_operator( a_operator, a_operation ) \
constexpr auto operator a_operator##= ( const point& other ) noexcept -> point& \
{ \
	using	::std::ranges::transform; \
	transform( *this, other, begin( ), a_operation ); \
	return	*this; \
} \
constexpr auto operator a_operator##= ( t_scalar other ) noexcept -> point& \
{ \
	using	::std::ranges::transform; \
	using	::std::views::repeat; \
	transform( *this, repeat( other ), begin( ), a_operation ); \
	return	*this; \
} \
friend constexpr auto operator a_operator ( point left, const point& right ) noexcept -> point { return left a_operator##= right; } \
friend constexpr auto operator a_operator ( point left, t_scalar right          ) noexcept -> point { return left a_operator##= right; } \
friend constexpr auto operator a_operator ( t_scalar left, const point& right   ) noexcept -> point \
{ \
	point result{ }; \
	result.fill( left ); \
	return	result a_operator##= right; \
}


template< is_arithmetic t_scalar = int, size_t num_dimensions = 2 >
requires	( num_dimensions > 1 )
class point final : private array< t_scalar, num_dimensions >
{
public:
	using	super_type	= 	array< t_scalar, num_dimensions >; 

	__using( typename super_type::
		,value_type
		,size_type
		,difference_type
		,pointer
		,const_pointer
		,reference
		,const_reference
		,iterator
		,const_iterator
	)

	__using( super_type::
		,begin
		,end
		,size
		,operator []
		,data
		,fill
	)

	constexpr point( ) : super_type{ } { }

	template< typename... t_args >
		requires( sizeof...( t_args ) == num_dimensions
			and ( convertible_to< t_args, t_scalar > and ... )
		)
	constexpr point( t_args... args )
		: super_type{ static_cast< t_scalar >( args )... }
	{ }

	__352612026_operator( + ,plus		)
	__352612026_operator( - ,minus		)
	__352612026_operator( * ,multiplies	)
	__352612026_operator( / ,divides	)
	__352612026_operator( % ,modulus	)

	constexpr auto operator - ( ) const noexcept -> point { return *this | negate | to_point; }
	
	template< invocable< t_scalar, t_scalar > t_operation >
	constexpr auto is_all( const point& other, const t_operation& operation ) const noexcept -> bool
	{
		using	::std::views::zip;
		using	::std::ranges::all_of;
		return	all_of( zip( *this, other ), tupled( operation ) );
	}
	constexpr auto operator == ( const point& other ) const noexcept -> bool { return is_all( other, equal_to ); }
	constexpr auto is_inside( const point& other ) const noexcept -> bool { return is_all( other, less_equal ); }

	constexpr auto get_length( ) const noexcept -> t_scalar
	{
		return	sqrt( sum( *this | square ) );
	}

	constexpr auto get_product( ) const noexcept -> t_scalar
	{
		return	fold_left( *this, 1, multiplies );
	}

};


#undef __352612026_operator


}


#endif


