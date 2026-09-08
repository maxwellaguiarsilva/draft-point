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
//	File:   source/tui/terminal.cpp
//	Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	
//	Created on 2025-08-24 11:29:53
//	


#include <sak/sak.hpp>
#include <sak/format.hpp>
#include <sak/pattern/value_or.hpp>
#include <sak/ranges/to.hpp>
#include <sak/ranges/transform.hpp>
#include <tui/terminal.hpp>
#include <iostream>
#include <functional>
#include <format>
#include <termios.h>
#include <unistd.h>		//	stdin_fileno, read
#include <sys/ioctl.h>	//	tiocgwinsz
#include <csignal>


//	glossary:
//	tcgetattr	=	terminal configuration get attribute
//	tiocgwinsz	=	terminal input output configuration get window size


namespace tui {

__using( ::std::
	,bind_front
	,cout
	,cerr
	,endl
	,flush
	,make_shared
	,to_string
)
__using( ::std::views::
	,iota
)
__using( ::sak::ranges::
	,lazy_transform
)


__using_constexpr( geometry::, left, top )
__using( ::sak::, ensure, format )
using	::sak::pattern::value_or;
using	::sak::ranges::to;
using	text_style	=	::tui::terminal::text_style;
using	position	=	geometry::position;
using	enum	::tui::terminal::error;


const terminal::error_messages terminal::m_error_messages	=
{
	 { tcgetattr_failed	,"terminal: tcgetattr error" }
	,{ tcsetattr_failed	,"terminal: tcsetattr error" }
	,{ ioctl_failed		,"terminal: ioctl error" }
};
const string terminal::m_unknown_error_message	=	"terminal: unknown error";


const array< string, 256 > terminal::m_foreground_colors	=	iota( 0, 256 )
	|	lazy_transform( bind_front( format, "\033[{};5;{}m", 38 ) )
	|	to;
const array< string, 256 > terminal::m_background_colors	=	iota( 0, 256 )
	|	lazy_transform( bind_front( format, "\033[{};5;{}m", 48 ) )
	|	to;
const array< string, 10 > terminal::m_text_styles	=	iota( 0, 10 )
	|	lazy_transform( bind_front( format, "\033[{}m" ) )
	|	to;
constexpr auto zero = position( 0, 0 );


terminal::terminal( )
	:m_output( cout )
	,m_error_output( cerr )
	,m_bounds( { 1, 1 }, query_size( ) )
	,m_foreground( 15 )
	,m_background( 0 )
{
	ensure( tcgetattr( STDIN_FILENO, &m_original_termios ) == 0, error_message( tcgetattr_failed ) );
	ensure( m_bounds.end not_eq zero, error_message( ioctl_failed ) );
	ensure( m_bounds.start.is_inside( m_bounds.end ), "invalid terminal size" );

	clear_screen( true );
	( void )raw_mode( true );
	refresh( );

	sigset_t set;
	sigemptyset( &set );
	sigaddset( &set, SIGWINCH );
	pthread_sigmask( SIG_BLOCK, &set, nullptr );

	m_resize_thread	=	jthread( [ this, set ]( stop_token token )
	{
		int sig = 0;
		while( not token.stop_requested( ) )
		{
			if( sigwait( &set, &sig ) == 0 )
			{
				if( token.stop_requested( ) )
					break;

				if( sig == SIGWINCH )
				{
					auto current_size = query_size( );
					if( current_size not_eq zero )
					{
						{
							auto lock = lock_guard( m_mutex );
							m_bounds.end	=	current_size;
						}
						( void )m_dispatcher( &listener::resize, size( ) );
					}
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
	refresh( );
}


auto terminal::clear_screen( bool full_reset ) -> void
{
	if( full_reset )
	{
		style( text_style::reset );
		if( auto result = raw_mode( false ); not result )
			print( result.error( ) );
		
		geometry::position start_position;
		{
			auto lock = lock_guard( m_mutex );
			start_position = m_bounds.start;
		}
		move_cursor( start_position );
	}
	print( "\033[2J" );
}

auto terminal::read_char( ) -> char
{
	char character = 0;
	read( STDIN_FILENO, &character, 1 );
	return	character;
}

auto terminal::move_cursor( const geometry::position& position ) -> void
{ 
	m_buffer << "\033[" << top( position ) << ';' << left( position ) << 'H';
}

auto terminal::print( const string& text ) -> void { m_buffer << text; }

auto terminal::print( const geometry::position& position, const string& text ) -> void
{
	move_cursor( position );
	print( text );
}

auto terminal::refresh( ) -> void
{
	m_output << m_buffer.str( ) << flush;
	m_buffer.str( "" );
	m_buffer.clear( );
}

auto terminal::color( byte code, bool background ) -> void
{
	byte& current = background ? m_background : m_foreground;
	if( current == code ) return;
	current = code;
	m_buffer << ( background ? m_background_colors[ code ] : m_foreground_colors[ code ] );
}

auto terminal::color( byte foreground, byte background ) -> void
{
	color( foreground, false );
	color( background, true );
}

auto terminal::cursor( bool enable ) -> void { print( enable ? "\033[?25h" : "\033[?25l" ); }

auto terminal::raw_mode( bool enable ) -> result
{
	cursor( not enable );
	if( enable )
	{
		auto raw = m_original_termios;
		//	ignore-logical-operators-rule
		raw.c_lflag		&=	~( ECHO | ICANON );	//	disable echo and canonical mode
		raw.c_cc[ VMIN ]	=	0;	//	minimum number of characters for non-canonical read
		raw.c_cc[ VTIME ]	=	0;	//	timeout in deciseconds for non-canonical read
		if( tcsetattr( STDIN_FILENO, TCSAFLUSH, &raw ) not_eq 0 )
			return	unexpected( tcsetattr_failed );
	}
	else if( tcsetattr( STDIN_FILENO, TCSAFLUSH, &m_original_termios ) not_eq 0 )
		return	unexpected( tcsetattr_failed );

	return	{ };
}

auto terminal::style( text_style new_style ) -> void
{
	if( new_style == text_style::reset )
	{
		m_foreground = 15;
		m_background = 0;
	}

	m_buffer << m_text_styles[ static_cast< size_t >( new_style ) ];
}

auto terminal::query_size( ) -> geometry::size
{
	winsize window_size;
	if( ioctl( STDOUT_FILENO, TIOCGWINSZ, &window_size ) not_eq 0 )
		return	{ 0, 0 };
	return	{ window_size.ws_col, window_size.ws_row };
}

auto terminal::size( ) const noexcept -> geometry::size
{
	auto lock = lock_guard( m_mutex );
	return	m_bounds.end;
}

auto terminal::error_message( const error& error_code ) noexcept -> const string&
{
	return	value_or( m_error_messages, error_code, m_unknown_error_message );	//	"terminal: unknown error"
}

void terminal::operator +=( const shared_ptr< listener >& instance ) { m_dispatcher += instance; }

auto terminal::print( const error& error_code ) const noexcept -> void { m_error_output << error_message( error_code ) << endl; }


}


