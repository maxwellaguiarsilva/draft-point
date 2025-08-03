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
 * File:   tui/screen.hpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-08-03 12:10
 */

#pragma once
#ifndef hpp881342869
#define hpp881342869

#include <ncurses.h>

namespace tui
{

class screen
{
public:
	screen( );
	virtual ~screen( );

	screen( const screen& ) = delete;
	screen& operator=( const screen& ) = delete;
	screen( screen&& ) = delete;
	screen& operator=( screen&& ) = delete;
};


}

#endif

