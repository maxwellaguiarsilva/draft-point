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
 * File:   main.c++
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-08-02 10:00
 */



#include <string>
#include <chrono>
#include <thread>
#include <ncurses.h>
#include <tui/screen.hpp>
#include <tui/window.hpp>
#include <tui/terminal.hpp>


int main( )
{{
	using	::tui::window;
	using	::tui::screen;
	using	::tui::terminal;
	using	::std::string;
	using	::std::chrono::milliseconds;
	using	::std::chrono::high_resolution_clock;
	using	::std::this_thread::sleep_for;
	using	color = ::tui::terminal::color;
	using	text_style = ::tui::terminal::text_style;

	screen main_screen;
	terminal terminal;

	try
	{
		window main_window;

		int char_y = 10;
		int char_x = 10;

		bool exit_loop = false;
		while( true )
		{
			auto start_time = high_resolution_clock::now( );

			switch( main_window.get_char( ) )
			{
				case KEY_LEFT:
					--char_x;
					break;
				case KEY_RIGHT:
					++char_x;
					break;
				case KEY_UP:
					--char_y;
					break;
				case KEY_DOWN:
					++char_y;
					break;
				case 'q':
				case 'Q':
					exit_loop = true;
					break;
			}
			if( exit_loop )
				break;

			//	terminal.set_text_style( text_style::underline );
			terminal.move_cursor( char_y, char_x );
			terminal.print( "\u2588" );
			terminal.refresh( );
			//	main_window.print( char_y, char_x, "#" );
			//	main_window.refresh( );

			auto end_time = high_resolution_clock::now( );
			auto frame_time = ::std::chrono::duration_cast<milliseconds>( end_time - start_time );
			milliseconds target_frame_time = milliseconds{ 1000 } / 60;

			if( frame_time < target_frame_time )
				sleep_for( target_frame_time - frame_time );
		}

		return 0;
	} catch( const ::std::exception& e ) {
		if( ! isendwin( ) )
			fprintf( stderr, "error: %s\n", e.what( ) );
		return 1;
	}
}}
