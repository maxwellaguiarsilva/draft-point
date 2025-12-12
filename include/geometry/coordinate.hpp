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


#include <sak/sak.hpp>
#include <array>
#include <concepts>
#include <algorithm>
#include <ranges>
#include <functional>
#include <expected>
#include <cstddef>	//	size_t
#include <cmath>	//	sqrt
#include <compare>


#define __581074281_loop( a_operator, a_type, a_decl_size, a_if_zero, a_right_value ) \
coordinate& operator a_operator##=( a_type other ) \
{ \
	a_decl_size \
	a_if_zero \
	for_each( *this, [ & ]( des_type& value ){ value a_operator##= a_right_value; } ); \
	return	*this; \
} \
coordinate operator a_operator ( a_type other ) const noexcept { return ( coordinate( *this ) a_operator##= other ); } 


#define __581074281_overload( a_operator, a_if_zero_coordinate, a_if_zero_scalar ) \
__581074281_loop( a_operator, const coordinate&, size_t index = 0;, a_if_zero_coordinate, other[ index++ ] ) \
__581074281_loop( a_operator, des_type, , a_if_zero_scalar, other ) \


#define __581074281_operator( a_operator ) \
__581074281_overload( a_operator, , )

#define __581074281_operator_not_eq_zero( a_operator ) \
__581074281_overload( \
	 a_operator \
	,if( not any_of( other, [ ]( des_type value ) { return value == 0; } ) ) \
	,if( other not_eq 0 ) \
)


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
//	--------------------------------------------------
using	::std::expected;
//	--------------------------------------------------
using	::std::plus;
using	::std::minus;
using	::std::multiplies;
using	::std::divides;
using	::std::modulus;
using	::std::sqrt;
//	--------------------------------------------------


enum class math_error
{
	 division_by_zero
	,arithmetic_overflow
	,square_root_of_negative
};



template< typename des_type = int, size_t num_dimensions = 2 >
class coordinate : public array< des_type, num_dimensions >
{
public:
	using	super_type = array< des_type, num_dimensions >;
	using	math_result = expected< des_type, math_error >;

	template< typename... t_args >
		requires( sizeof...( t_args ) == num_dimensions
			and ( convertible_to< t_args, des_type > and ... )
		)
	constexpr coordinate( t_args... a_args )
		: super_type{ static_cast< des_type >( a_args )... }
	{ }
	

	__581074281_operator( + )
	__581074281_operator( - )
	__581074281_operator( * )
	__581074281_operator_not_eq_zero( / )
	__581074281_operator_not_eq_zero( % )


	bool operator==( const coordinate& other ) const noexcept = default;

	bool is_inside( const coordinate& other ) const noexcept
	{
		return	not any_of( zip( *this, other ), [ ]( const auto& pair ){
			const auto& [ a_this, a_other ] = pair;
			return	a_this > a_other;
		} );
	}

	auto get_length( ) const noexcept -> des_type
	{
		des_type sum_of_squares	=	0;
		for( const auto& value : *this )
			sum_of_squares += value * value;
		return	static_cast< des_type >( sqrt( sum_of_squares ) );
	}

private:
	template< typename t_function >
	coordinate& apply_array_operation( const coordinate& other, const t_function& operation )
	{
		for( auto&& [ left, right ] : zip( *this, other ) )
			left = operation( left, right );
		return	*this;
	}

	template< typename t_function >
	coordinate& apply_scalar_operation( des_type other, const t_function& operation )
	{
		for_each( *this, [ & ]( des_type& value ){ value = operation( value, other ); } );
		return	*this;
	}


};


}


#endif


//	coordinate& operator +=( const coordinate& other ) {
//		size_t index = 0;
//		for_each( this->begin( ), this->end( ), [ & ]( des_type& value ){ value += other[ index++ ]; } );
//		return *this;
//	}
//	coordinate& operator +=( des_type other ) {
//		for_each( this->begin( ), this->end( ), [ & ]( des_type& value ){ value += other; } );
//		return *this;
//	}
//	coordinate operator + ( const coordinate& other ) const noexcept { return ( coordinate( *this ) += other ); }
//	coordinate operator + ( des_type other ) const noexcept { return ( coordinate( *this ) += other ); }
//
//
//	coordinate& operator -=( const coordinate& other ) {
//		size_t index = 0;
//		for_each( this->begin( ), this->end( ), [ & ]( des_type& value ){ value -= other[ index++ ]; } );
//		return *this;
//	}
//	coordinate& operator -=( des_type other ) {
//		for_each( this->begin( ), this->end( ), [ & ]( des_type& value ){ value -= other; } );
//		return *this;
//	}
//	coordinate operator - ( const coordinate& other ) const noexcept { return ( coordinate( *this ) -= other ); }
//	coordinate operator - ( des_type other ) const noexcept { return ( coordinate( *this ) -= other ); }
//
//
//	coordinate& operator *=( const coordinate& other ) {
//		size_t index = 0;
//		for_each( this->begin( ), this->end( ), [ & ]( des_type& value ){ value *= other[ index++ ]; } );
//		return *this;
//	}
//	coordinate& operator *=( des_type other ) {
//		for_each( this->begin( ), this->end( ), [ & ]( des_type& value ){ value *= other; } );
//		return *this;
//	}
//	coordinate operator * ( const coordinate& other ) const noexcept { return ( coordinate( *this ) *= other ); }
//	coordinate operator * ( des_type other ) const noexcept { return ( coordinate( *this ) *= other ); }
//
//
//	coordinate& operator /=( const coordinate& other ) {
//		size_t index = 0;
//		if( not any_of( other, [ ]( des_type value ) { return value == 0; } ) )
//			for_each( this->begin( ), this->end( ), [ & ]( des_type& value ){ value /= other[ index++ ]; } );
//		return *this;
//	}
//	coordinate& operator /=( des_type other ) {
//		if( other not_eq 0 )
//			for_each( this->begin( ), this->end( ), [ & ]( des_type& value ){ value /= other; } );
//		return *this;
//	}
//	coordinate operator / ( const coordinate& other ) const noexcept { return ( coordinate( *this ) /= other ); }
//	coordinate operator / ( des_type other ) const noexcept { return ( coordinate( *this ) /= other ); }
//
//
//	coordinate& operator %=( const coordinate& other ) {
//		size_t index = 0;
//		if( not any_of( other, [ ]( des_type value ) { return value == 0; } ) )
//			for_each( this->begin( ), this->end( ), [ & ]( des_type& value ){ value %= other[ index++ ]; } );
//		return *this;
//	}
//	coordinate& operator %=( des_type other ) {
//		if( other not_eq 0 )
//			for_each( this->begin( ), this->end( ), [ & ]( des_type& value ){ value %= other; } );
//		return *this;
//	}
//	coordinate operator % ( const coordinate& other ) const noexcept { return ( coordinate( *this ) %= other ); }
//	coordinate operator % ( des_type other ) const noexcept { return ( coordinate( *this ) %= other ); }


