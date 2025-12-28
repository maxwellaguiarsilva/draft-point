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
 * You should have received a copy of the GNU General License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/* 
 * File:   0002_atomic_flag.cpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-11-23 12:32:14
 */


#include <print>		//	print
#include <string>
#include <vector>
#include <cstdlib>
#include <sak/sak.hpp>	//	ensure
#include <unistd.h>		//	stdout_fileno, pause
#include <sys/ioctl.h>	//	ioctl
#include <csignal>		//	signal
#include <atomic>		//	atomic_flag
#include <thread>		//	jthread, stop_token
#include <game/fps.hpp>	//	fps


using	::std::println;
using	::std::print;
using	::std::atomic_flag;
using	::std::signal;
using	::std::jthread;
using	::std::stop_token;
using	::sak::ensure;


atomic_flag flg_window_changed;


void print_terminal_size( )
{
	struct winsize window_size;
	ensure( ioctl( STDOUT_FILENO, TIOCGWINSZ, &window_size ) not_eq -1, "error on get_win_size" );

	println( "Terminal size: {} by {}", window_size.ws_col, window_size.ws_row );
}


void signal_handler( int signal_number )
{
	if( signal_number == SIGWINCH )
		flg_window_changed.test_and_set( );
}


auto main( const int argument_count, const char* argument_values[ ] ) -> int
{{
	using	::std::string;
	using	::std::vector;

	const vector< string > arguments( argument_values, argument_values + argument_count );
	for( const auto& value : arguments )
		println( "{}", value );

	//	register the action for the sigwinch signal.
	if( signal( SIGWINCH, signal_handler ) == SIG_ERR )
		return	println( "error registering sigwinch signal handler" ), EXIT_FAILURE;
	
	println( "resize the terminal window: press ctrl+c to exit" );
	print_terminal_size( );
	
	jthread worker( []( stop_token stoken ) {
		::game::fps fps(10);

		while( !stoken.stop_requested( ) )
		{
			if( flg_window_changed.test( ) )
			{
				flg_window_changed.clear( );
				print_terminal_size( );
			}

			fps.compute( );
		}
	} );
	worker.join( );

	return	EXIT_SUCCESS;

}};



