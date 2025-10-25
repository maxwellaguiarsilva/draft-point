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
 * File:   game/game.hpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-08-26 19:41
 */



#pragma once
#ifndef header_guard_578639205
#define header_guard_578639205


#include <sak.hpp>
#include <string>
#include <tui/terminal.hpp>
#include <game/player.hpp>
#include <game/fps.hpp>
#include <geometry/point.hpp>


namespace game {


using	::std::string;
using	::tui::terminal;
using	::game::player;
using	::game::fps;
using	point = ::geometry::point< int, 2 >;


class game
{
public:
	game( );
	virtual ~game( ) noexcept;

	disable_copy_move_ctc( game );

	auto run( ) -> void;

private:
	terminal	m_terminal;
	player		m_player;
	fps			m_fps;

};


} 


#endif


