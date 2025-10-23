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
using	::std::size_t;
using	::std::for_each;


template< typename des_type = int, size_t num_dimensions = 2 >
class point : public array< des_type, num_dimensions >
{
public:
	using super_type = array< des_type, num_dimensions >;

	explicit point( initializer_list< des_type > a_list )
	{
		size_t index = 0;
		for( const auto& value : a_list )
			if( index < num_dimensions )
				( *this )[ index++ ] = value;
			else
				break;
	}
	virtual ~point( ) noexcept { }

	point& operator+=( const point& other )
	{
		for( size_t index = 0; index < num_dimensions; ++index )
			( *this )[ index ] += other[ index ];
		return	*this;
	}
	point& operator-=( const point& other )
	{
		for( size_t index = 0; index < num_dimensions; ++index )
			( *this )[ index ] -= other[ index ];
		return	*this;
	}
	point& operator*=( const point& other )
	{
		for( size_t index = 0; index < num_dimensions; ++index )
			( *this )[ index ] *= other[ index ];
		return	*this;
	}
	point& operator/=( const point& other )
	{
		for( size_t index = 0; index < num_dimensions; ++index )
			if( other[ index ] not_eq 0 )
				( *this )[ index ] /= other[ index ];
		return	*this;
	}
	point& operator%=( const point& other )
	{
		for( size_t index = 0; index < num_dimensions; ++index )
			if( other[ index ] not_eq 0 )
				( *this )[ index ] %= other[ index ];
		return	*this;
	}

	point& operator+=( des_type scalar )
	{
		for_each( this->begin( ), this->end( ), [ scalar ]( des_type& value ){ value += scalar; } );
		return	*this;
	}
	point& operator-=( des_type scalar )
	{
		for_each( this->begin( ), this->end( ), [ scalar ]( des_type& value ){ value -= scalar; } );
		return	*this;
	}
	point& operator*=( des_type scalar )
	{
		for_each( this->begin( ), this->end( ), [ scalar ]( des_type& value ){ value *= scalar; } );
		return	*this;
	}
	point& operator/=( des_type scalar )
	{
		if( scalar not_eq 0 )
			for_each( this->begin( ), this->end( ), [ scalar ]( des_type& value ){ value /= scalar; } );
		return	*this;
	}
	point& operator%=( des_type scalar )
	{
		if( scalar not_eq 0 )
			for_each( this->begin( ), this->end( ), [ scalar ]( des_type& value ){ value %= scalar; } );
		return	*this;
	}

	point operator+( const point& other ) const noexcept { return ( point( *this ) += other ); }
	point operator-( const point& other ) const noexcept { return ( point( *this ) -= other ); }
	point operator*( const point& other ) const noexcept { return ( point( *this ) *= other ); }
	point operator/( const point& other ) const noexcept { return ( point( *this ) /= other ); }
	point operator%( const point& other ) const noexcept { return ( point( *this ) %= other ); }

	point operator+( des_type scalar ) const noexcept { return ( point( *this ) += scalar ); }
	point operator-( des_type scalar ) const noexcept { return ( point( *this ) -= scalar ); }
	point operator*( des_type scalar ) const noexcept { return ( point( *this ) *= scalar ); }
	point operator/( des_type scalar ) const noexcept { return ( point( *this ) /= scalar ); }
	point operator%( des_type scalar ) const noexcept { return ( point( *this ) %= scalar ); }

	bool operator==( const point& other ) const noexcept
	{
		return static_cast< const super_type& >( *this ) == static_cast< const super_type& >( other );
	}
	bool operator!=( const point& other ) const noexcept { return not( *this == other ); }
};


}


#endif
