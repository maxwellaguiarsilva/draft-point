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
#include <vector>
#include <initializer_list>
#include <algorithm>


namespace geometry {


using	::std::vector;
using	::std::initializer_list;
using	::std::min;


class point : public vector<int>
{
public:
	explicit point( initializer_list<int> a_list );
	virtual ~point( ) noexcept;

	point& operator+=( const point& other );
	point& operator-=( const point& other );
	point& operator*=( const point& other );
	point& operator/=( const point& other );
	point& operator%=( const point& other );

	point& operator+=( int scalar );
	point& operator-=( int scalar );
	point& operator*=( int scalar );
	point& operator/=( int scalar );
	point& operator%=( int scalar );

	point operator+( const point& other ) const noexcept;
	point operator-( const point& other ) const noexcept;
	point operator*( const point& other ) const noexcept;
	point operator/( const point& other ) const noexcept;
	point operator%( const point& other ) const noexcept;

	point operator+( int scalar ) const noexcept;
	point operator-( int scalar ) const noexcept;
	point operator*( int scalar ) const noexcept;
	point operator/( int scalar ) const noexcept;
	point operator%( int scalar ) const noexcept;

	bool operator==( const point& other ) const noexcept;
	bool operator!=( const point& other ) const noexcept;


};


} 


#endif


