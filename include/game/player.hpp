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
 * File:   game/player.hpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-08-24 16:06
 */


#pragma once
#ifndef header_guard_185813530
#define header_guard_185813530


#include <game/renderer.hpp>
#include <sak/sak.hpp>
#include <game/geometry.hpp>
#include <game/direction.hpp>


namespace game {


using	::game::direction;
using	enum	::game::direction;


class player
{
public:
	explicit player( const g2i::point& position ) noexcept;

	auto get_direction( ) const noexcept -> const direction&;
	auto set_direction( const direction& heading ) noexcept -> void;

	auto step_move( ) noexcept -> void;

	auto draw( renderer& renderer ) const noexcept -> void;
	
	g2i::point position;
private:
	direction m_direction;
};


} 


#endif


