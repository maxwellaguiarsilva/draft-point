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
 * File:   geometry/point.cpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-10-16 14:02
 */



#include <geometry/point.hpp>


#define __193526415_assignment( p_operator ) \
point& point::operator p_operator##=( const point& other ) \
{ \
	for( size_t index = 0; index < min( size( ), other.size( ) ); ++index ) \
		( *this )[ index ] p_operator##= other[ index ]; \
	return	*this; \
}

#define __193526415_assignment_if_zero( p_operator ) \
point& point::operator p_operator##=( const point& other ) \
{ \
	for( size_t index = 0; index < min( size( ), other.size( ) ); ++index ) \
		if( other[ index ] not_eq 0 ) \
			( *this )[ index ] p_operator##= other[ index ]; \
	return	*this; \
}

#define __193526415_assignment_scalar( p_operator ) \
point& point::operator p_operator##=( int scalar ) \
{ \
	::std::for_each( begin( ), end( ), [ scalar ]( int& value ) { value p_operator##= scalar; } ); \
	return	*this; \
}

#define __193526415_assignment_scalar_if_zero( p_operator ) \
point& point::operator p_operator##=( int scalar ) \
{ \
	if( scalar not_eq 0 ) \
		::std::for_each( begin( ), end( ), [ scalar ]( int& value ) { value p_operator##= scalar; } ); \
	return	*this; \
}

#define __193526415_binary( p_type, p_operator ) \
point point::operator p_operator ( p_type other ) const noexcept { return ( point( *this ) p_operator##= other ); }


namespace geometry {


point::point( initializer_list<int> a_list ) : vector<int>( a_list ) { }
point::~point( ) noexcept { }


__193526415_assignment( + )
__193526415_assignment( - )
__193526415_assignment( * )
__193526415_assignment_if_zero( / )
__193526415_assignment_if_zero( % )
__193526415_binary( const point&, + )
__193526415_binary( const point&, - )
__193526415_binary( const point&, * )
__193526415_binary( const point&, / )
__193526415_binary( const point&, % )

__193526415_assignment_scalar( + )
__193526415_assignment_scalar( - )
__193526415_assignment_scalar( * )
__193526415_assignment_scalar_if_zero( / )
__193526415_assignment_scalar_if_zero( % )
__193526415_binary( int, + )
__193526415_binary( int, - )
__193526415_binary( int, * )
__193526415_binary( int, / )
__193526415_binary( int, % )

bool point::operator==( const point& other ) const noexcept
{
	return static_cast<const vector<int>&>( *this ) == static_cast<const vector<int>&>( other );
}
bool point::operator!=( const point& other ) const noexcept { return not( *this == other ); }


}


