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
 * File:   include/sak/geometry/geometry.hpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-12-23 19:05:44
 */


#pragma once
#ifndef header_guard_356592464
#define header_guard_356592464


#include <sak/sak.hpp>
#include <sak/geometry/point.hpp>
#include <sak/geometry/line_view.hpp>


namespace sak {


template< is_point t_point = point< int, 2 > >
struct geometry 
{
	
	using	point	=	t_point;

	struct line
	{
		point start, end;
		constexpr auto get_size( ) const noexcept -> point { return end - start; }
	};

	struct rectangle
	{
		point start, end;
		constexpr auto get_size( ) const noexcept -> point { return end - start; }
		constexpr auto contains( const point& point ) const noexcept -> bool
		{
			return	start.is_inside( point ) and point.is_inside( end );
		}
		constexpr auto is_inside( const rectangle& other ) const noexcept -> bool
		{
			return	other.start.is_inside( start ) and end.is_inside( other.end );
		}
	};

};


using	g2i	=	geometry< point< int, 2 > >;
using	g3i	=	geometry< point< int, 3 > >;

using	g2f	=	geometry< point< float, 2 > >;
using	g3f	=	geometry< point< float, 3 > >;


}


#endif


