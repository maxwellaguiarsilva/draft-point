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
 * File:   game/renderer/renderer.hpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-11-09 10:03
 */



#pragma once
#ifndef header_guard_219958687
#define header_guard_219958687

#include <sak/sak.hpp>
#include <game/game.hpp>


namespace game {
namespace renderer {


using	::game::point;


class renderer
{
public:
	enum class color
	{
		 black			=	0
		,red			=	1
		,green			=	2
		,yellow			=	3
		,blue			=	4
		,magenta		=	5
		,cyan			=	6
		,white			=	7
	};
	
	explicit renderer( ) = default;


};


} } 


#endif


