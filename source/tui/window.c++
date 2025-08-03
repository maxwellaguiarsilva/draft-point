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
 * File:   tui/window.c++
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-07-27 15:00
 */



#include <tui/window.hpp>
#include <cassert>

namespace tui
{

window::window( int height, int width, int start_y, int start_x )
{
	int actual_height = ( height == 0 ) ? LINES : height;
	int actual_width = ( width == 0 ) ? COLS : width;

	m_window = newwin( actual_height, actual_width, start_y, start_x );
	assert( m_window != nullptr );

	keypad( m_window, true );
	nodelay( m_window, true );
}

window::~window( )
{
	delwin( m_window );
}

void window::print( int y, int x, const char* c_str ) const
{
	mvwprintw( m_window, y, x, "%s", c_str );
}

int window::get_char( ) const
{
	return	wgetch( m_window );
}

void window::refresh( ) const
{
	wrefresh( m_window );
}

void window::clear( ) const
{
	wclear( m_window );
}

}

