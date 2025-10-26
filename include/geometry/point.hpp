/*
 * Copyright (C) 2025 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
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
 * File:   geometry/point.hpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-10-16 14:02
 */



#pragma once
#ifndef header_guard_581074281
#define header_guard_581074281


#include <sak.hpp>
#include <array>
#include <initializer_list>
#include <algorithm>
#include <cstddef>


namespace geometry {


using	::std::array;
using	::std::initializer_list;
using	::std::for_each;
using	::std::size_t;


#define __581074281_binary( a_operator, p_type ) \
point operator a_operator##( p_type other ) const noexcept { return ( point( *this )  a_operator##= other ); }


#define __581074281_assignment( a_operator, a_not_zero_point, a_not_zero_scalar ) \
point& operator a_operator##=( const point& other ) \
{ \
	size_t index = 0; \
	for_each( this->begin( ), this->end( ), [ & ]( des_type& value ){ \
		a_not_zero_point \
			value a_operator##= other[index]; \
		++index; \
	} ); \
	return	*this; \
} \
point& operator a_operator##=( des_type other ) \
{ \
	a_not_zero_scalar \
		for_each( this->begin( ), this->end( ), [ & ]( des_type& value ){ \
			value a_operator##= other; \
		} ); \
	return	*this; \
} \
__581074281_binary( a_operator, const point& ) \
__581074281_binary( a_operator, des_type )


#define __581074281_operator( a_operator ) \
__581074281_assignment( a_operator, , )

#define __581074281_operator_not_eq_zero( a_operator ) \
__581074281_assignment( a_operator, if( other[index] not_eq 0 ), if( other not_eq 0 ) )


template< typename desa_type = int, size_t num_dimensions = 2 >
class point : public array< des_type, num_dimensions >
{
public:
	using super_type = array< des_type, num_dimensions >;

	explicit point( initializer_list< des_type > a_list )
	{
		assert( a_list.size( ) == num_dimensions, "Initializer list size must match number of dimensions." );
		std::copy( a_list.begin( ), a_list.end( ), this->begin( ) );
	}
	virtual ~point( ) noexcept { }

	__581074281_operator( + )
	__581074281_operator( - )
	__581074281_operator( * )
	__581074281_operator_not_eq_zero( / )
	__581074281_operator_not_eq_zero( % )


	bool operator==( const point& other ) const noexcept
	{
		return static_cast< const super_type& >( *this ) == static_cast< const super_type& >( other );
	}
	bool operator!=( const point& other ) const noexcept { return not( *this == other ); }
};


}


#endif
