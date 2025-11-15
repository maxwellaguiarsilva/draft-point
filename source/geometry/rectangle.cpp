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
 * Created on 2025-11-15 08:34
 */



#include <geometry/rectangle.hpp>


namespace geometry {


using	::std::string;
using	::geometry::point;





auto rectangle::contains( const point& point ) const noexcept -> bool
{
	const auto&	[ x, y ] = point;

	return	x >= ::std::min( start.x, end.x )	&&
			x <= ::std::max( start.x, end.x )	&&
			y >= ::std::min( start.y, end.y )	&&
			y <= ::std::max( start.y, end.y );
}


} 



