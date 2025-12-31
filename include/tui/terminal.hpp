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
#include <sak/geometry/geometry.hpp>
#include <sak/pattern/dispatcher.hpp>
#include <termios.h>
#include <string>
#include <unordered_map>
#include <expected>
#include <ostream>
#include <memory>
#include <thread>
#include <csignal>


namespace tui {


__using( ::std::
	,string
	,ostream
	,expected
	,unexpected
	,shared_ptr
	,jthread
	,stop_token
)
using	::sak::pattern::dispatcher;
using	point	=	::sak::geometry::coordinate< int, 2 >;
using	rectangle	=	::sak::geometry::shapes< point >::rectangle;
using	::termios;


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
		,unknown
	};

	using	result	=	expected< void, error >;
	using	error_messages	=	::std::unordered_map< error, string >;


	terminal( );
	~terminal( ) noexcept;

	delete_copy_move_ctc( terminal );

	auto clear_screen( bool full_reset = false ) -> void;
	static auto read_char( ) -> char;
	auto move_cursor( const point& position ) -> result;
	auto print( const string& text ) -> void;
	auto print( const point& position, const string& text ) -> result;
	auto refresh( ) -> void;
	auto set_color( color color_code, bool flg_background = false ) -> void;
	auto set_cursor( bool enable ) -> void;
	auto set_raw_mode( bool enable ) -> result;
	auto set_text_style( text_style style ) -> void;

	static auto get_error_message( const error& error_code ) noexcept -> const string&;

	class listener
	{
	public:
		virtual ~listener( ) = default;
		virtual void on_resize( const point& size ) = 0;
	};
	void operator +=( const shared_ptr< listener >& instance );

private:
	static const error_messages m_error_messages;
	static const string			m_unknown_error_message;
	auto print( const error& error_code ) const noexcept -> void;

	ostream&	m_output;
	ostream&	m_error_output;
	termios		m_original_termios;
	rectangle	m_bounds;
	jthread		m_resize_thread;
	dispatcher< listener >	m_dispatcher;

public:
	constexpr auto size( ) const noexcept -> const point& { return m_bounds.to( ); }

};


}


#endif


