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
 * File:   game/direction.hpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-11-09 15:30
 */



#pragma once
#ifndef header_guard_499869391
#define header_guard_499869391

#include <sak/sak.hpp>
#include <geometry/point.hpp>


namespace game {




class direction
{
public:
	const ::geometry::point up    { 0, -1 };
	const ::geometry::point down  { 0, 1 };
	const ::geometry::point left  { -1, 0 };
	const ::geometry::point right { 1, 0 };
};


} 


#endif


