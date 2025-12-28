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
 * File:   sak/geometry/geometry.hpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-11-15 18:14
 */



#pragma once
#ifndef header_guard_356592464
#define header_guard_356592464


#include <sak/sak.hpp>
#include <sak/geometry/coordinate.hpp>


namespace sak {
namespace geometry {


template< is_coordinate t_point = coordinate< int, 2 > >
struct shapes
{
	
	using	point	=	t_point;

	class rectangle
	{
	private:
		array< point, 2 > m_data;
	public:
		constexpr rectangle( ) : m_data{ } { }
		point&	start	=	m_data[0];
		point&	end		=	m_data[1];

		constexpr auto contains( const point& a_point ) const noexcept -> bool
		{
			return	start.encloses( a_point ) and a_point.encloses( end );
		};
	};

};


} }


#endif


