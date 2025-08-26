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


namespace game {


using	::std::string;
using	::tui::terminal;


class player
{
public:

	enum class movement
	{
		 up
		,down
		,left
		,right
	};

	player( terminal& terminal, int top, int left, const string& character = "\u2588" );
	virtual ~player( );

	disable_copy_move_ctc( player );

	void draw( ) const;
	void move( movement movement );

private:

	int m_top;
	int m_left;
	string m_character;
	terminal& m_terminal;

	void clear_old_position( );

};


} 


#endif


