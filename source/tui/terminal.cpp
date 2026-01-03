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

#include <sak/sak.hpp>
#include <sak/pattern/value_or.hpp>
#include <tui/terminal.hpp>
#include <tui/renderer.hpp>
#include <iostream>
#include <termios.h>
#include <unistd.h>		//	STDIN_FILENO, read
#include <sys/ioctl.h>	//	TIOCGWINSZ


//	glossary:
//	tcgetattr	=	terminal configuration get attribute
//	tiocgwinsz	=	terminal input output configuration get window size


namespace tui {

__using( ::std::
	,cout
	,cerr
	,endl
	,flush
	,make_shared
)


using	::sak::ensure;
using	::sak::pattern::value_or;
using	::tui::point;
using	color		=	::tui::terminal::color;
using	text_style	=	::tui::terminal::text_style;
using	enum	::tui::terminal::error;


const terminal::error_messages terminal::m_error_messages	=
{
	 { out_of_bounds	,"terminal: cursor position out of bounds" }
	,{ tcgetattr_failed	,"terminal: tcgetattr error" }
	,{ tcsetattr_failed	,"terminal: tcsetattr error" }
	,{ ioctl_failed		,"terminal: ioctl error" }
};
const string terminal::m_unknown_error_message	=	"terminal: unknown error";


terminal::terminal( )
	:	m_output( cout )
	,	m_error_output( cerr )
{
	winsize m_ws;
	ensure( tcgetattr( STDIN_FILENO, &m_original_termios ) == 0, get_error_message( tcgetattr_failed ) );
	ensure( ioctl( STDOUT_FILENO, TIOCGWINSZ, &m_ws ) == 0, get_error_message( ioctl_failed ) );
	{
		auto lock = lock_guard( m_mutex );
		m_bounds	=	{ { 1, 1 }, { m_ws.ws_col, m_ws.ws_row } };
		ensure( m_bounds.end.encloses( m_bounds.start ), "error: invalid terminal size" );
	}
	clear_screen( true );
	set_raw_mode( true );

	m_renderer	=	make_shared< renderer >( *this );
	*this += m_renderer->m_terminal_listener;

	sigset_t set;
	sigemptyset( &set );
	sigaddset( &set, SIGWINCH );
	pthread_sigmask( SIG_BLOCK, &set, nullptr );

	m_resize_thread	=	jthread( [ this, set ]( stop_token token )
	{
		winsize ws;
		int sig = 0;
		while( not token.stop_requested( ) )
		{
			if( sigwait( &set, &sig ) == 0 )
			{
				if( token.stop_requested( ) )
					break;

				if( sig == SIGWINCH and ioctl( STDOUT_FILENO, TIOCGWINSZ, &ws ) == 0 )
				{
					{
						auto lock = lock_guard( m_mutex );
						m_bounds.end	=	{ ws.ws_col, ws.ws_row };
					}
					m_dispatcher( &listener::on_resize, size( ) );
				}
			}
		}
	} );
}
terminal::~terminal( ) noexcept
{
	m_resize_thread.request_stop( );
	pthread_kill( m_resize_thread.native_handle( ), SIGWINCH );
	clear_screen( true );
}


auto terminal::clear_screen( bool full_reset ) -> void
{
	if( full_reset )
	{
		set_text_style( text_style::reset );
		if( auto result = set_raw_mode( false ); not result )
			print( result.error( ) );
		
		point l_start;
		{
			auto lock = lock_guard( m_mutex );
			l_start = m_bounds.start;
		}
		move_cursor( l_start );
	}
	print( "\033[2J" );
}

auto terminal::read_char( ) -> char
{
	char c = 0;
	read( STDIN_FILENO, &c, 1 );
	return	c;
}

auto terminal::move_cursor( const point& position ) -> result 
{ 
	{
		auto lock = lock_guard( m_mutex );
		if( not m_bounds.contains( position ) )
			return	unexpected( out_of_bounds );
	}
	m_output << "\033[" << position[1] << ';' << position[0] << 'H';
	return	{ };
}

auto terminal::print( const string& text ) -> void { m_output << text; }
auto terminal::print( const point& position, const string& text ) -> result
{
	if( auto status = move_cursor( position ); not status )
		return	status;
	print( text );
	return	{ };
}

auto terminal::refresh( ) -> void { m_output << flush; }

auto terminal::set_color( color color, bool background ) -> void { m_output << "\033[" << ( static_cast<int>( color ) + ( background ? 40 : 30 ) ) << 'm'; }
auto terminal::set_cursor( bool enable ) -> void { print( enable ? "\033[?25h" : "\033[?25l" ); }

auto terminal::set_raw_mode( bool enable ) -> result
{
	set_cursor( not enable );
	if( enable )
	{
		auto raw = m_original_termios;
		//  ignore-logical-operators-rule
		raw.c_lflag		&=	~( ECHO | ICANON );	//	disable echo and canonical mode
		raw.c_cc[VMIN]	=	0;	//	minimum number of characters for non-canonical read
		raw.c_cc[VTIME]	=	0;	//	timeout in deciseconds for non-canonical read
		if( tcsetattr( STDIN_FILENO, TCSAFLUSH, &raw ) not_eq 0 )
			return	unexpected( tcsetattr_failed );
	}
	else if( tcsetattr( STDIN_FILENO, TCSAFLUSH, &m_original_termios ) not_eq 0 )
		return	unexpected( tcsetattr_failed );

	return	{ };
}

auto terminal::set_text_style( text_style style ) -> void { m_output << "\033[" << static_cast<int>( style ) << 'm'; }

auto terminal::size( ) const noexcept -> point
{
	auto lock = lock_guard( m_mutex );
	return	m_bounds.end;
}

auto terminal::get_renderer( ) noexcept -> renderer& { return *m_renderer; }

auto terminal::get_error_message( const error& error_code ) noexcept -> const string&


{
	return	value_or( m_error_messages, error_code, m_unknown_error_message ); //	"terminal: unknown error"
}

void terminal::operator +=( const shared_ptr< listener >& instance ) { m_dispatcher += instance; }

auto terminal::print( const error& error_code ) const noexcept -> void { m_error_output << get_error_message( error_code ) << endl; }


}


