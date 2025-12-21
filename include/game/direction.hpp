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


namespace game {
namespace direction {

using	::std::string;
using	::std::unordered_map;


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


struct __use_direction
{
	constexpr auto operator ( )( direction a_direction ) const -> const string&
	{
		static const unordered_map< direction, string > s_directions =
		{
			 {	direction::up			,"up"			}
			,{	direction::down			,"down"			}
			,{	direction::left			,"left"			}
			,{	direction::right		,"right"		}
			,{	direction::up_left		,"up_left"		}
			,{	direction::up_right		,"up_right"		}
			,{	direction::down_left	,"down_left"	}
			,{	direction::down_right	,"down_right"	}
		};

		static const string s_none	=	"none";

		const auto name = s_directions.find( a_direction );
		return	( name not_eq s_directions.end( ) ) ? name->second : s_none;
	}
};
inline constexpr auto use_direction	=	__use_direction{ };


} } 


#endif
