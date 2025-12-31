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
 * File:   0002_volatile_sig_atomic_t.cpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-11-23 12:32:14
 */


#include <print>		//	print
#include <string>
#include <vector>
#include <cstdlib>
#include <sak/sak.hpp>	//	ensure
#include <unistd.h>		//	stdout_fileno
#include <sys/ioctl.h>	//	ioctl
#include <signal.h>		//	signal handling (sigaction, siginfo_t, etc.)
#include <game/fps.hpp>	//	fps


using	::std::println;
using	::sak::ensure;


//	needs to be `volatile` to be `signal_thread_safe`
volatile sig_atomic_t flg_window_changed = 0;


//	gets and prints the current terminal size (rows and columns)
void print_terminal_size( )
{
	struct winsize window_size;
	ensure( ioctl( STDOUT_FILENO, TIOCGWINSZ, &window_size ) not_eq -1, "error on get_win_size" );

	println( "Terminal size: {} by {}", window_size.ws_col, window_size.ws_row );
}


//	signal handler for sigwinch
void signal_window_change_handler( int signal_number ) { if( signal_number == SIGWINCH ) flg_window_changed = 1; }


//	configures the handler for the sigwinch signal
void setup_signal_handler( )
{
	struct sigaction signal_action;

	//	assign the function to be called
	signal_action.sa_handler = signal_window_change_handler;

	//	resets any flags that may be set
	sigemptyset( &signal_action.sa_mask );

	//	flags: sa_restart restarts interrupted system calls
	signal_action.sa_flags = SA_RESTART;

	//	register the action for the sigwinch signal
	ensure( sigaction( SIGWINCH, &signal_action, nullptr ) not_eq -1, "error registering sigwinch signal handler" );
}


auto main( const int argument_count, const char* argument_values[ ] ) -> int
{{
	using	::std::string;
	using	::std::vector;

	const vector< string > arguments( argument_values, argument_values + argument_count );
	for( const auto& value : arguments )
		println( "{}", value );

	::game::fps fps( 10 );

	setup_signal_handler( );

	println( "resize the terminal window: press ctrl+c to exit" );
	print_terminal_size( );

	//	main program loop
	while( true )
	{
		if( flg_window_changed )
		{
			flg_window_changed = 0;
			print_terminal_size( );
		}

		fps.compute( );
	}

	return	EXIT_SUCCESS;
}};


