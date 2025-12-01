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
#include <sak/sak.hpp>	//	assert
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


atomic_flag flg_window_changed;


void print_terminal_size( )
{
	struct winsize window_size;
	assert( ioctl( STDOUT_FILENO, TIOCGWINSZ, &window_size ) not_eq -1, "erro no get_win_size" );

	println( "Tamanho do terminal: {} por {}", window_size.ws_col, window_size.ws_row );
}


void signal_handler( int signal_number )
{
	if( signal_number == SIGWINCH )
		flg_window_changed.test_and_set( );
}


int main( )
{{

	//	registra a ação para o sinal sigwinch.
	if( signal( SIGWINCH, signal_handler ) == SIG_ERR )
		return	println( "erro ao registrar o manipulador de sinal sigwinch" ), EXIT_FAILURE;
	
	println( "redimensione a janela do terminal: pressione ctrl+c para sair" );
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



