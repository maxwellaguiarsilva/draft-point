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
 * Created on 2025-12-21 17:39
 */



#pragma once
#ifndef header_guard_399468167
#define header_guard_399468167

#include <sak/sak.hpp>
#include <string>
#include <unordered_map>
#include <geometry/point.hpp>


namespace game {
namespace direction {

using	::std::string;
using	::std::unordered_map;
using	::geometry::point;


enum class direction
{
	 up	=	0
	,down
	,left
	,right
	,up_left
	,up_right
	,down_left
	,down_right
	,none
};

using enum direction;

struct __use_direction
{
	constexpr auto operator ( )( direction a_direction ) const -> const string&
	{
		const auto item = m_directions.find( a_direction );
		return	( item not_eq m_directions.end( ) ) ? item->second.name : s_none.name;
	}

	constexpr auto operator ( )( direction a_direction ) const -> const point&
	{
		const auto item = m_directions.find( a_direction );
		return	( item not_eq m_directions.end( ) ) ? item->second.value : s_none.value;
	}
private:
	struct __direction { string name; point value; }
	static const unordered_map< direction, __direction > m_directions	=
	{
		 {	up			,{	"up"			,{	0	,-1	} }
		,{	down		,{	"down"			,{	0	,1	} }
		,{	left		,{	"left"			,{	-1	,0	} }
		,{	right		,{	"right"			,{	1	,0	} }
		,{	up_left		,{	"up_left"		,{	-1	,-1	} }
		,{	up_right	,{	"up_right"		,{	1	,-1	} }
		,{	down_left	,{	"down_left"		,{	-1	,1	} }
		,{	down_right	,{	"down_right"	,{	1	,1	} }
	};
	static const __direction m_none	=	{ "none", { 0, 0 } };

};
inline constexpr auto use_direction	=	__use_direction{ };


} } 


#endif
