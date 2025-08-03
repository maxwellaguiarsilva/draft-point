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
#include <unistd.h> // For STDIN_FILENO, read
#include <sys/ioctl.h> // For TIOCGWINSZ
#include <cstdio> // For perror

namespace tui
{

using ::std::cout;
using ::std::flush;
using ::std::format;
using	color		=	::tui::terminal::color;
using	text_style	=	::tui::terminal::text_style;



terminal::terminal( )
{
	set_raw_mode( true );
	clear_screen( true );
}
terminal::~terminal( )
{
	set_raw_mode( false );
	clear_screen( true );
}


void terminal::set_raw_mode( bool enable )
{
	if( enable )
	{
		if( tcgetattr( STDIN_FILENO, &m_original_termios ) == -1 )
			perror( "tcgetattr" );

		struct termios raw = m_original_termios;
		raw.c_lflag &= ~( ECHO | ICANON ); // Disable echo and canonical mode
		raw.c_cc[VMIN] = 0; // Minimum number of characters for non-canonical read
		raw.c_cc[VTIME] = 0; // Timeout in deciseconds for non-canonical read

		if ( tcsetattr( STDIN_FILENO, TCSAFLUSH, &raw ) == -1 )
			perror( "tcsetattr" );
	}
	else
		if ( tcsetattr( STDIN_FILENO, TCSAFLUSH, &m_original_termios ) == -1 )
			perror( "tcsetattr" );
}

void terminal::refresh( ) { cout << flush; }
void terminal::clear_screen( bool full_reset ) {
	cout << "\033[2J";
	if( !full_reset )
		return;
	set_text_style( text_style::reset );
	move_cursor( 0, 0 );
}
void terminal::move_cursor( int row, int column ) { cout << format( "\033[{};{}H", column, row ); }
void terminal::print( const string &text ) { cout << text; }
void terminal::print( int row, int column, const string &text )
{
	move_cursor( row, column );
	print( text );
}
void terminal::set_text_style( text_style style ) { cout << format( "\033[{}m", static_cast<int>( style ) ); }
void terminal::set_color( color color, bool background ) { cout << format( "\033[{}m", static_cast<int>( color ) + ( background ? 10 : 0 ) ); }

int terminal::get_char( )
{
	char c;
	read( STDIN_FILENO, &c, 1 );
	return static_cast<int>( c );
}

int terminal::get_terminal_width( )
{
	struct winsize ws;
	if ( ioctl( STDOUT_FILENO, TIOCGWINSZ, &ws ) == -1 || ws.ws_col == 0 )
		return 80; // Default to 80 if unable to get width
	return ws.ws_col;
}

int terminal::get_terminal_height( )
{
	struct winsize ws;
	if ( ioctl( STDOUT_FILENO, TIOCGWINSZ, &ws ) == -1 || ws.ws_row == 0 )
		return 24; // Default to 24 if unable to get height
	return ws.ws_row;
}


};


