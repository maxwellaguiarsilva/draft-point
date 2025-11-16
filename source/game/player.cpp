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


using	color	=	::tui::terminal::color;


player::player( const point& position ) noexcept
	:m_position{ position }
{ }


auto player::get_position( ) const noexcept -> const point& { return m_position; }
auto player::set_position( const point& p_position ) noexcept -> void { m_position = p_position; }

auto player::step_move( ) noexcept -> void { m_position += direction; }

auto player::draw( terminal& terminal ) const noexcept -> void
{
	terminal.set_color( color::white );
	terminal.print( m_position, "█" );
}


} 


