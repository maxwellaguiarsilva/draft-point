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
	return *this; \
}


#define __193526415_assignment_if_zero( p_operator ) \
point& point::operator p_operator##=( const point& other ) \
{ \
	for( size_t index = 0; index < min( size( ), other.size( ) ); ++index ) \
		if( other[ index ] != 0 ) \
			( *this )[ index ] p_operator##= other[ index ]; \
	return *this; \
}


#define __193526415_assignment_scalar( p_operator ) \
point& point::operator p_operator##=( int scalar ) \
{ \
	for( auto& value : *this ) \
		value p_operator##= scalar; \
	return *this; \
}


#define __193526415_assignment_scalar_if_zero( p_operator ) \
point& point::operator p_operator##=( int scalar ) \
{ \
	if( scalar != 0 ) \
		for( auto& value : *this ) \
			value p_operator##= scalar; \
	return *this; \
}



namespace geometry {


point::point( initializer_list<int> a_list ) : vector<int>( a_list ) { }
point::~point( ) { }


__193526415_assignment( + )
__193526415_assignment( - )
__193526415_assignment( * )
__193526415_assignment_if_zero( / )
__193526415_assignment_if_zero( % )

__193526415_assignment_scalar( + )
__193526415_assignment_scalar( - )
__193526415_assignment_scalar( * )
__193526415_assignment_scalar_if_zero( / )
__193526415_assignment_scalar_if_zero( % )

point point::operator+( const point& other ) const { return ( point( *this ) += other ); }
point point::operator-( const point& other ) const { return ( point( *this ) -= other ); }
point point::operator*( const point& other ) const { return ( point( *this ) *= other ); }
point point::operator/( const point& other ) const { return ( point( *this ) /= other ); }
point point::operator%( const point& other ) const { return ( point( *this ) %= other ); }

point point::operator+( int scalar ) const { return ( point( *this ) += scalar ); }
point point::operator-( int scalar ) const { return ( point( *this ) -= scalar ); }
point point::operator*( int scalar ) const { return ( point( *this ) *= scalar ); }
point point::operator/( int scalar ) const { return ( point( *this ) /= scalar ); }
point point::operator%( int scalar ) const { return ( point( *this ) %= scalar ); }


bool point::operator==( const point& other ) const
{
	return static_cast<const vector<int>&>( *this ) == static_cast<const vector<int>&>( other );
}
bool point::operator!=( const point& other ) const { return not( *this == other ); }


}


