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
#include <sak/sak.hpp>	//	assert
#include <unistd.h>		//	stdout_fileno
#include <sys/ioctl.h>	//	ioctl
#include <signal.h>		//	signal handling (sigaction, siginfo_t, etc.)
#include <game/fps.hpp>	//	fps


using	::std::println;
using	::std::print;


//	precisa ser `volatile` para ser `signal_thread_safe`
volatile sig_atomic_t flg_window_changed = 0;

/**
 * @brief Obtém e imprime o tamanho atual do terminal (linhas e colunas).
 */
void print_terminal_size( )
{
	struct winsize window_size;
	assert( ioctl( STDOUT_FILENO, TIOCGWINSZ, &window_size ) not_eq -1, "erro no get_win_size" );

	println( "Tamanho do terminal: {} por {}", window_size.ws_col, window_size.ws_row );
}

/**
 * @brief Manipulador de sinal (Signal Handler) para SIGWINCH.
 * @param signal_number O número do sinal recebido (deve ser SIGWINCH).
 */
void signal_window_change_handler( int signal_number ) {
	if( signal_number == SIGWINCH )
		flg_window_changed = 1;
}

/**
 * @brief Configura o manipulador para o sinal SIGWINCH.
 */
void setup_signal_handler( )
{
	struct sigaction signal_action;

	//	atribuição a função a ser chamada
	signal_action.sa_handler = signal_window_change_handler;
	//	reinicia quaisquer flags que possam ser definidas
	sigemptyset(&signal_action.sa_mask);
	//	flags: sa_restart reinicia chamadas de sistema interrompidas.
	signal_action.sa_flags = SA_RESTART;

	//	registra a ação para o sinal sigwinch.
	assert( sigaction( SIGWINCH, &signal_action, NULL ) not_eq -1, "erro ao registrar o manipulador de sinal sigwinch" );
}

int main( )
{{
	::game::fps fps(10);

	setup_signal_handler( );

	println( "redimensione a janela do terminal: pressione ctrl+c para sair" );
	print_terminal_size( );

	// loop principal do programa
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



