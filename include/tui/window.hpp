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
 * File:   tui/window.hpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-07-27 15:00
 */



#pragma once
#ifndef hpp597281476
#define hpp597281476

#include <ncurses.h>

namespace tui
{

class window
{
private:
    WINDOW* m_window;
public:
    window( int height = 0, int width = 0, int start_y = 0, int start_x = 0 );
    virtual ~window( );

	window( const window& ) = delete;
	window& operator=( const window& ) = delete;
	window( window && ) = delete;
	window& operator=( window&& ) = delete;

    void print( int y, int x, const char* c_str ) const;
    int get_char( ) const;
    void refresh( ) const;
    void clear( ) const;
};

}

#endif
