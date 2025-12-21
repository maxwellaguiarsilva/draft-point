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
 * File:   tui/terminal.cpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-08-03 14:35
 */

//	ignore-no-comments-rule

#include <tui/terminal.hpp>
#include <format>
#include <iostream>
#include <unistd.h>		//	STDIN_FILENO, read
#include <sys/ioctl.h>	//	TIOCGWINSZ


//	legenda:
//	terminal configuration get attribute
//	tiocgwinsz	=	terminal input output configuration get window size


namespace tui {


using	::std::cout;
using	::std::flush;
using	::std::format;
using	::tui::point;
using	color		=	::tui::terminal::color;
using	text_style	=	::tui::terminal::text_style;



terminal::terminal( )
{
	struct winsize m_ws;
	assert( tcgetattr( STDIN_FILENO, &m_original_termios ) == 0, "terminal: tcgetattr error" );
	assert( ioctl( STDOUT_FILENO, TIOCGWINSZ, &m_ws ) == 0, "terminal: ioctl error" );
	m_bounds.start	=	{ 1, 1 }
	m_bounds.end	=	{ m_ws.ws_col, m_ws.ws_row };
	clear_screen( true );
	set_raw_mode( true );
}
terminal::~terminal( ) noexcept { clear_screen( true ); }

auto terminal::clear_screen( bool full_reset ) -> void
{
	if( full_reset )
	{
		set_text_style( text_style::reset );
		set_raw_mode( false );
		move_cursor( point( 0, 0 ) );
	}
	print( "\033[2J" );
}

auto terminal::read_char( ) -> char
{
	char c = 0;
	read( STDIN_FILENO, &c, 1 );
	return	c;
}

auto terminal::move_cursor( const point& position ) -> void { print( format( "\033[{};{}H", position[1], position[0] ) ); }

auto terminal::print( const string& text ) -> void { cout << text; }
auto terminal::print( const point& position, const string& text ) -> void
{
	move_cursor( position );
	print( text );
}

auto terminal::refresh( ) -> void { cout << flush; }

auto terminal::set_color( color color, bool background ) -> void { print( format( "\033[{}m", static_cast<int>( color ) + ( background ? 40 : 30 ) ) ); }
auto terminal::set_cursor( bool enable ) -> void { print( enable ? "\033[?25h" : "\033[?25l" ); }

auto terminal::set_raw_mode( bool enable ) -> void
{
	set_cursor( not enable );
	if( enable )
	{
		auto raw = m_original_termios;
		//  ignore-logical-operators-rule
		raw.c_lflag		&=	~( ECHO | ICANON );	//	disable echo and canonical mode
		raw.c_cc[VMIN]	=	0;	//	minimum number of characters for non-canonical read
		raw.c_cc[VTIME]	=	0;	//	timeout in deciseconds for non-canonical read
		assert( tcsetattr( STDIN_FILENO, TCSAFLUSH, &raw ) == 0, "terminal: tcsetattr error" );
	}
	else
		assert( tcsetattr( STDIN_FILENO, TCSAFLUSH, &m_original_termios ) == 0, "terminal: tcsetattr error" );
}

auto terminal::set_text_style( text_style style ) -> void { print( format( "\033[{}m", static_cast<int>( style ) ) ); }



};


