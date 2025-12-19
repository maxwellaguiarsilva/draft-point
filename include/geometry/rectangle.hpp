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
 * Created on 2025-11-15 18:23
 */



#pragma once
#ifndef header_guard_192621987
#define header_guard_192621987

#include <sak/sak.hpp>
#include <array>
#include <geometry/point.hpp>


#define __192621987_export_m_data_index( a_method, a_index ) \
constexpr auto a_method( ) noexcept -> point& { return m_data[ a_index ]; } \
constexpr auto a_method( ) const noexcept -> const point& { return m_data[ a_index ]; }


namespace geometry {


using	::std::array;
using	::geometry::point;


class rectangle
{
public:
	__192621987_export_m_data_index( start, 0 )
	__192621987_export_m_data_index( end, 1 )

	auto contains( const point& a_point ) const noexcept -> bool;

private:
	array< point, 2 > m_data;
};


} 


#endif


