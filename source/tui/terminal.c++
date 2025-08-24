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



#include <tui/terminal.h++>
#include <format>
#include <iostream>
#include <unistd.h>		//	For STDIN_FILENO, read
#include <sys/ioctl.h>	//	For TIOCGWINSZ



namespace tui
{


using ::std::cout;
using ::std::flush;
using ::std::format;
using	color		=	::tui::terminal::color;
using	text_style	=	::tui::terminal::text_style;



terminal::terminal( )
{
	assert( tcgetattr( STDIN_FILENO, &m_original_termios ) != 1 );
	assert( ioctl( STDOUT_FILENO, TIOCGWINSZ, &m_ws ) != 1 );
	clear_screen( true );
	set_raw_mode( true );
}
terminal::~terminal( ) { clear_screen( true ); }

void terminal::set_raw_mode( bool enable )
{
	set_cursor( !enable );
	if( enable )
	{
		auto raw = m_original_termios;
		raw.c_lflag &= ~( ECHO | ICANON ); // Disable echo and canonical mode
		raw.c_cc[VMIN] = 0; // Minimum number of characters for non-canonical read
		raw.c_cc[VTIME] = 0; // Timeout in deciseconds for non-canonical read

		assert( tcsetattr( STDIN_FILENO, TCSAFLUSH, &raw ) != 1 );
	}
	else
		assert( tcsetattr( STDIN_FILENO, TCSAFLUSH, &m_original_termios ) != 1 );
}

void terminal::refresh( ) { cout << flush; }
void terminal::clear_screen( bool full_reset )
{
	if( full_reset )
		set_text_style( text_style::reset );
		set_raw_mode( false );
		move_cursor( 0, 0 );
	print( "\033[2J" );
}
void terminal::move_cursor( int row, int column ) { print( format( "\033[{};{}H", column, row ) ); }
void terminal::set_cursor( bool enable ) { print( enable ? "\033[?25h" : "\033[?25l" ); }
void terminal::print( const string &text ) { cout << text; }
void terminal::print( int row, int column, const string& text )
{
	move_cursor( row, column );
	print( text );
}
void terminal::set_text_style( text_style style ) { print( format( "\033[{}m", static_cast<int>( style ) ) ); }
void terminal::set_color( color color, bool background ) { print( format( "\033[{}m", static_cast<int>( color ) + ( background ? 10 : 0 ) ) ); }

const char terminal::get_char( )
{
	char c;
	read( STDIN_FILENO, &c, 1 );
	return static_cast<int>( c );
}

const int terminal::get_width( )  { return m_ws.ws_col; }
const int terminal::get_height( ) { return m_ws.ws_row; }


};


