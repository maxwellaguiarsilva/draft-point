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
 * File:   geometry/rectangle.hpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-11-15 08:34
 */



#pragma once
#ifndef header_guard_670255237
#define header_guard_670255237

#include <sak.hpp>
#include <string>
#include <geometry/point.hpp>
#include <array>


namespace geometry {


using	::std::string;
using	::geometry::point;


class rectangle : public ::std::array< point, 2 >
{
public:

	point& start = ( *this )[ 0 ];
	point& end = ( *this )[ 1 ];

	rectangle( );
	virtual ~rectangle( );

	auto contains( const point& point ) const noexcept -> bool;

	disable_copy_move_ctc( rectangle );

};


} 


#endif


