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
 * File:   game/player.cpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-08-24 15:14
 */



#include <game/player.hpp>


namespace game {


using	::std::string;
using	color		=	::tui::terminal::color;
using	movement	=	::game::player::movement;


player::player( terminal& terminal, int top, int left, const string& character )
	:m_top{ top }
	,m_left{ left }
	,m_character{ character }
	,m_terminal{ terminal }
{ }


player::~player( ) { }


auto player::clear_old_position( ) -> void
{
	m_terminal.set_color( color::cyan );
	m_terminal.print( m_left, m_top, m_character );
}


auto player::draw( ) const -> void
{
	m_terminal.set_color( color::white );
	m_terminal.print( m_left, m_top, m_character );
}


auto player::move( movement movement ) -> void
{
	using	movement::up;
	using	movement::down;
	using	movement::left;
	using	movement::right;
	clear_old_position( );

	int width = m_terminal.get_width( );
	int height = m_terminal.get_height( );

	switch( movement )
	{
		case up:	--m_top;	break;
		case down:	++m_top;	break;
		case left:	--m_left;	break;
		case right:	++m_left;	break;
	}

	m_left += width;
	m_left %= width;
	m_top += height;
	m_top %= height;

	draw( );
}


} 



