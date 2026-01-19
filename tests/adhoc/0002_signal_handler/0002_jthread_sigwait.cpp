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
//	File:   tests/adhoc/0002_signal_handler/0002_jthread_sigwait.cpp
//	Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	
//	Created on 2025-12-30 23:45:57
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
#include <thread>


using	::std::println;
using	::sak::ensure;


//	gets and prints the current terminal size ( rows and columns )
void print_terminal_size( )
{
	struct winsize window_size;
	ensure( ioctl( STDOUT_FILENO, TIOCGWINSZ, &window_size ) not_eq -1, "error on get_win_size" );

	println( "Terminal size: {} by {}", window_size.ws_col, window_size.ws_row );
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
		,jthread
		,stop_token
	)

	println( "{}", join_with( vector< string >( argument_values, argument_values + argument_count ), "\n" ) );

	try
	{
		//	signals to block and wait for
		sigset_t signal_set;
		sigemptyset( &signal_set );
		sigaddset( &signal_set, SIGWINCH );
		sigaddset( &signal_set, SIGINT );
		sigaddset( &signal_set, SIGTERM );

		//	block signals in the main thread ( they will be blocked in child threads too )
		ensure( pthread_sigmask( SIG_BLOCK, &signal_set, nullptr ) == 0, "error blocking signals" );

		println( "resize the terminal window: press ctrl+c to exit" );
		print_terminal_size( );

		//	dedicated thread for signal handling
		jthread signal_handler( [ signal_set ]( stop_token stop_token )
		{
			int received_signal = 0;
			while( not stop_token.stop_requested( ) )
			{
				//	synchronously wait for signals in the set
				if( sigwait( &signal_set, &received_signal ) == 0 )
				{
					if( stop_token.stop_requested( ) )
						break;

					switch( received_signal )
					{
						case SIGWINCH:
							print_terminal_size( );
							break;

						case SIGINT:
						case SIGTERM:
							println( "\nSignal {} received. Exiting...", received_signal );
							return;

						default:
							break;
					}
				}
			}
		} );

		//	the main thread just waits for the signal handler thread to finish
		signal_handler.join( );

		println( "all tests passed!" );
	}
	catch( const exception& error )
	{
		println( "test failed: {}", error.what( ) );
		return	exit_failure;
	}

	return	exit_success;
}}


