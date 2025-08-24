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
 * File:   tui/terminal.h++
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-08-03 14:35
 */



#pragma once
#ifndef header_guard_278153203
#define header_guard_278153203


#include <sak.h++>
#include <termios.h>


namespace tui
{



class terminal
{
public:
	enum class color
	{
		 black			=	30
		,red			=	31
		,green			=	32
		,yellow			=	33
		,blue			=	34
		,magenta		=	35
		,cyan			=	36
		,white			=	37
		,default_color	=	39
	};

	enum class text_style
	{
		 reset			=	0
		,bold			=	1
		,faint			=	2
		,italic			=	3
		,underline		=	4
		,slow_blink		=	5
		,rapid_blink	=	6
		,reverse		=	7
		,hide			=	8
		,crossed_out	=	9
	};

	terminal( );
	virtual ~terminal( );

	disable_copy_move_ctc( terminal );

	void refresh( );
	void clear_screen( bool full_reset = false );
	void move_cursor( int row, int column );
	void set_cursor( bool enable );
	void set_color( color color_code, bool flg_background = false );
	void set_text_style( text_style style );
	void set_raw_mode( bool enable );
	void print( const string& text );
	void print( int row, int column, const string &text );
	const char get_char( );
	const int get_width( );
	const int get_height( );

private:
	struct termios m_original_termios;
	struct winsize m_ws;

};

}

#endif


