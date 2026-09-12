//	
//	SPDX-FileCopyrightText: 2025 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	SPDX-License-Identifier: GPL-3.0-or-later
//	


#include <print>
#include <ranges>
#include <string>
#include <vector>
#include <exception>
#include <sak/ensure.hpp>
#include <sak/using.hpp>
#include <sak/sak.hpp>
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
{
	__using( ::sak::
		,exit_success
		,exit_failure
		,ensure
	)
	__using( ::std::
		,string
		,vector
		,println
		,exception
		,signal
		,jthread
		,stop_token
	)
	__using( ::std::views::, join_with )
	__using( ::std::ranges::, to )

	println( "{}", vector< string >( argument_values, argument_values + argument_count ) | join_with( '\n' ) | to< string >( ) );

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

}


