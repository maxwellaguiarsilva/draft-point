//	
//	Copyright (C) 2025 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	
//	This program is free software: you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//	
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//	
//	You should have received a copy of the GNU General Public License
//	along with this program.  If not, see <http://www.gnu.org/licenses/>.
//	
//	
//	File:   include/tui/terminal.hpp
//	Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	
//	Created on 2025-08-03 16:17:10
//	

#pragma once
#ifndef header_guard_278153203
#define header_guard_278153203


#include <sak/pattern/dispatcher.hpp>
#include <tui/geometry.hpp>
#include <termios.h>
#include <ostream>
#include <sstream>
#include <thread>
#include <unordered_map>


namespace tui {


__using( ::std::
	,array
	,atomic
	,expected
	,jthread
	,lock_guard
	,mutex
	,ostream
	,ostringstream
	,shared_ptr
	,stop_token
	,string
	,unexpected
	,unordered_map
)
using	::sak::byte;
using	::sak::pattern::dispatcher;
using	::termios;


class terminal final
{
public:
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
		 tcgetattr_failed
		,tcsetattr_failed
		,ioctl_failed
		,unknown
	};

	using	result	=	expected< void, error >;
	using	error_messages	=	unordered_map< error, string >;


	terminal( );
	~terminal( ) noexcept;

	delete_copy_move_ctc( terminal );

	auto clear_screen( bool full_reset = false ) -> void;
	auto move_cursor( const g2i::point& position ) -> void;
	auto print( const g2i::point& position, const string& text ) -> void;
	auto print( const string& text ) -> void;
	auto refresh( ) -> void;
	auto set_color( byte code, bool is_background = false ) -> void;
	auto set_cursor( bool enable ) -> void;
	auto set_raw_mode( bool enable ) -> result;
	auto set_text_style( text_style style ) -> void;
	auto size( ) const noexcept -> g2i::point;

	static auto get_error_message( const error& error_code ) noexcept -> const string&;
	static auto query_size( ) -> g2i::point;
	static auto read_char( ) -> char;


	class listener
	{
	public:
		virtual ~listener( ) = default;
		virtual void on_resize( const g2i::point& size ) = 0;
	};
	void operator +=( const shared_ptr< listener >& instance );

private:
	static const error_messages			m_error_messages;
	static const string					m_unknown_error_message;
	static const array< string, 256 >	m_foreground_colors;
	static const array< string, 256 >	m_background_colors;
	static const array< string, 10 >	m_text_styles;

	auto print( const error& error_code ) const noexcept -> void;

	ostream&		m_output;
	ostream&		m_error_output;
	ostringstream	m_buffer;
	termios			m_original_termios;
	mutable mutex	m_mutex;
	g2i::rectangle	m_bounds;
	jthread			m_resize_thread;
	dispatcher< listener >	m_dispatcher;
};


}


#endif


