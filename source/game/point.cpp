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
 * File:   game/point.cpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-10-16 14:02
 */



#include <game/point.hpp>

namespace game {


point::point( initializer_list<int> a_list ) : vector<int>( a_list ) { }
point::~point( ) { }

const point point::up{ 0, -1 };
const point point::down{ 0, 1 };
const point point::left{ -1, 0 };
const point point::right{ 1, 0 };

point& point::operator+=( const point& other )
{
	for( size_t index = 0; index < min( size( ), other.size( ) ); ++index )
		( *this )[ index ] += other[ index ];
	return *this;
}

point& point::operator-=( const point& other )
{
	for( size_t index = 0; index < min( size( ), other.size( ) ); ++index )
		( *this )[ index ] -= other[ index ];
	return *this;
}

point& point::operator*=( const point& other )
{
	for( size_t index = 0; index < min( size( ), other.size( ) ); ++index )
		( *this )[ index ] *= other[ index ];
	return *this;
}

point& point::operator/=( const point& other )
{
	for( size_t index = 0; index < min( size( ), other.size( ) ); ++index )
		if( other[ index ] != 0 )
			( *this )[ index ] /= other[ index ];
	return *this;
}

point& point::operator%=( const point& other )
{
	for( size_t index = 0; index < min( size( ), other.size( ) ); ++index )
		if( other[ index ] != 0 )
			( *this )[ index ] %= other[ index ];
	return *this;
}

point& point::operator+=( int scalar )
{
	for( auto& value : *this )
		value += scalar;
	return *this;
}

point& point::operator-=( int scalar )
{
	for( auto& value : *this )
		value -= scalar;
	return *this;
}

point& point::operator*=( int scalar )
{
	for( auto& value : *this )
		value *= scalar;
	return *this;
}

point& point::operator/=( int scalar )
{
	if( scalar != 0 )
		for( auto& value : *this )
			value /= scalar;
	return *this;
}

point& point::operator%=( int scalar )
{
	if( scalar != 0 )
		for( auto& value : *this )
			value %= scalar;
	return *this;
}

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


