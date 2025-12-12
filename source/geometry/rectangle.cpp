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
 * File:   geometry/rectangle.cpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-11-15 18:23
 */



#include <geometry/rectangle.hpp>
#include <ranges>


namespace geometry {


using	::std::ranges::all_of;
using	::std::views::iota;
using	::geometry::point;


auto rectangle::contains( const point& a_point ) const noexcept -> bool
{
	return	all_of(
		 iota( ( size_t )0, start.size( ) )
		,[ & ]( size_t index ) { return a_point[ index ] >= start[ index ] and a_point[ index ] <= end[ index ]; }
	);
}


} 



