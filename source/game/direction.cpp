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
 * File:   game/direction.cpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-08-26 20:15
 */



#include <game/direction.hpp>


namespace game {

const point direction::m_up		{	0	,-1	};
const point direction::m_down	{	0	,1	};
const point direction::m_left	{	-1	,0	};
const point direction::m_right	{	1	,0	};
const point direction::m_none	{	0	,0	};


direction::direction( const direction_value& a_direction )
	:m_direction{ a_direction }
{ };

direction::operator const point& ( ) const noexcept
{
	switch( m_direction )
	{
		case direction_value::up:		return	m_up;
		case direction_value::down:		return	m_down;
		case direction_value::left:		return	m_left;
		case direction_value::right:	return	m_right;
	}
	return	m_none;
} 


}


