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
#include <sak/geometry/geometry.hpp>
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
	using	geometry	=	::sak::g2i;


	terminal( );
	~terminal( ) noexcept;

	delete_copy_move_ctc( terminal );

	auto clear_screen( bool full_reset = false ) -> void;
	auto move_cursor( const geometry::position& position ) -> void;
	auto print( const geometry::position& position, const string& text ) -> void;
	auto print( const string& text ) -> void;
	auto refresh( ) -> void;
	auto color( byte code, bool is_background = false ) -> void;
	auto color( byte foreground, byte background ) -> void;
	auto cursor( bool enable ) -> void;
	auto raw_mode( bool enable ) -> result;
	auto style( text_style new_style ) -> void;
	auto size( ) const noexcept -> geometry::size;

	static auto error_message( const error& error_code ) noexcept -> const string&;
	static auto query_size( ) -> geometry::size;
	static auto read_char( ) -> char;


	class listener
	{
	public:
		virtual ~listener( ) = default;
		virtual void resize( const geometry::size& size ) = 0;
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
	geometry::rectangle	m_bounds;
	jthread			m_resize_thread;
	dispatcher< listener >	m_dispatcher;
	byte			m_foreground;
	byte			m_background;
};


}


#endif


