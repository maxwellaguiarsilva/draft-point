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
 * File:   tui/terminal.hpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-08-03 14:35
 */



#pragma once
#ifndef header_guard_278153203
#define header_guard_278153203


#include <sak/sak.hpp>
#include <geometry/geometry.hpp>
#include <string>
#include <unordered_map>
#include <expected>


namespace tui {


using	::geometry::point;
using	::geometry::rectangle;
using	::std::string;
using	::std::expected;
using	::std::unexpected;

class terminal final
{
public:
	enum class color
	{
		 black			=	0
		,red			=	1
		,green			=	2
		,yellow			=	3
		,blue			=	4
		,magenta		=	5
		,cyan			=	6
		,white			=	7
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

	enum class error
	{
		 out_of_bounds
		,tcgetattr_failed
		,tcsetattr_failed
		,ioctl_failed
	};


	using	error_messages_type = ::std::unordered_map< error, string >;
	static const error_messages_type	error_messages;


	terminal( );
	~terminal( ) noexcept;

	delete_copy_move_ctc( terminal );

	auto clear_screen( bool full_reset = false ) -> expected< void, error >;
	auto read_char( ) -> char;
	auto move_cursor( const point& position ) -> expected< void, error >;
	auto print( const string& text ) -> void;
	auto print( const point& position, const string& text ) -> void;
	auto refresh( ) -> void;
	auto set_color( color color_code, bool flg_background = false ) -> void;
	auto set_cursor( bool enable ) -> void;
	auto set_raw_mode( bool enable ) -> expected< void, error >;
	auto set_text_style( text_style style ) -> void;

private:
	auto print_error( error error_code ) const noexcept -> void;

	struct termios m_original_termios;
	rectangle	m_bounds;
	point&		m_size	=	m_bounds.end;
public:
	const point& size = m_size;
};


}


#endif


