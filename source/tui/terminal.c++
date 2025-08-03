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
 * File:   tui/terminal.c++
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-08-03 14:35
 */



#include <tui/terminal.hpp>
#include <format>
#include <iostream>


namespace tui
{

using ::std::cout;
using ::std::flush;
using ::std::format;

terminal::terminal( ) { }
terminal::~terminal( ) { }


void terminal::refresh( ) { cout << flush; }
void terminal::clear_screen( ) { cout << "\033[2J"; }
void terminal::move_cursor( int row, int column ) { cout << format( "\033[{};{}H", row, column ); }
void terminal::print( const string &text ) { cout << text; }
void terminal::set_text_style( text_style style ) { cout << format( "\033[{}m", static_cast<int>( style ) ); }
void terminal::set_color( color color, bool background ) { cout << format( "\033[{}m", static_cast<int>( color ) + ( background ? 10 : 0 ) ); }


};


