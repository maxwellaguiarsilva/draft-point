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
//	File:   tests/adhoc/0002_signal_handler/0002_atomic_flag.cpp
//	Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	
//	Created on 2025-11-23 18:23:58
//


#include <print>
#include <string>
#include <vector>
#include <exception>
#include <sak/ensure.hpp>
#include <sak/using.hpp>
#include <sak/sak.hpp>
#include <sak/ranges/views/join_with.hpp>
#include <unistd.h>
#include <sys/ioctl.h>
#include <csignal>
#include <atomic>
#include <thread>
#include <game/fps.hpp>


using	::std::println;
using	::std::atomic_flag;
using	::sak::ensure;


atomic_flag flag_window_changed;


void print_terminal_size( )
{
	struct winsize window_size;
	ensure( ioctl( STDOUT_FILENO, TIOCGWINSZ, &window_size ) not_eq -1, "error on get_win_size" );

	println( "Terminal size: {} by {}", window_size.ws_col, window_size.ws_row );
}


void signal_handler( int signal_number )
{
	if( signal_number == SIGWINCH )
		flag_window_changed.test_and_set( );
}


auto main( const int argument_count, const char* argument_values[ ] ) -> int
{{
	__using( ::sak::
		,exit_success
		,exit_failure
		,ensure
	)
	__using( ::sak::ranges::views::, join_with )
	__using( ::std::
		,string
		,vector
		,println
		,exception
		,signal
		,jthread
		,stop_token
	)

	println( "{}", join_with( vector< string >( argument_values, argument_values + argument_count ), "\n" ) );

	try
	{
		//	register the action for the sigwinch signal
		if( signal( SIGWINCH, signal_handler ) == SIG_ERR )
		{
			println( "error registering sigwinch signal handler" );
			return	exit_failure;
		}
		
		println( "resize the terminal window: press ctrl+c to exit" );
		print_terminal_size( );
		
		jthread worker( [ ]( stop_token stop_token ) {
			::game::fps fps( 10 );

			while( not stop_token.stop_requested( ) )
			{
				if( flag_window_changed.test( ) )
				{
					flag_window_changed.clear( );
					print_terminal_size( );
				}

				fps.compute( );
			}
		} );
		worker.join( );

		println( "all tests passed!" );
	}
	catch( const exception& error )
	{
		println( "test failed: {}", error.what( ) );
		return	exit_failure;
	}

	return	exit_success;

}}


