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
 * Created on 2025-08-24 15:14
 */



#pragma once
#ifndef header_guard_185813530
#define header_guard_185813530

#include <sak.hpp>
#include <string>
#include <tui/terminal.hpp>
#include <game/point.hpp>


namespace game {


using	::std::string;
using	::tui::terminal;
using	::game::point;


class player
{
public:
	player( const point& box_size, const string& character = "█" );
	player( const point& box_size, const point& position, const string& character = "█" );
	virtual ~player( ) noexcept;

	disable_copy_move_ctc( player );

	auto draw( terminal& terminal ) const noexcept -> void;
	auto move( const point& direction ) -> void;
	auto get_position( ) const noexcept -> const point&;
	auto get_character( ) const noexcept -> const string&;
	
private:
	point m_position;
	string m_character;
	const point m_box_size;
	
};


} 


#endif


