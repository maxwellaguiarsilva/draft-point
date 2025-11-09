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
#include <tui/terminal.hpp>


namespace game {


using	::std::string;
using	color		=	::tui::terminal::color;


player::player( const point& box_size )
	: player( box_size, box_size / 2 )
{ }

player::player( const point& box_size, const point& position )
	:m_position{ position }
	,m_box_size{ box_size }
{ }
player::~player( ) noexcept { }

auto player::get_position( ) const noexcept -> const point& { return m_position; }

auto player::move( const point& direction ) -> void
{
	m_position += direction;
	m_position += m_box_size;
	m_position %= m_box_size;
}

auto player::draw( terminal& terminal ) const noexcept -> void
{
	terminal.set_color( color::white );
	terminal.print( m_position[ 0 ], m_position[ 1 ], "█" );
}


} 



