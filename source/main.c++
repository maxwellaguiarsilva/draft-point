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
#include <tui/terminal.hpp>
#include <iostream>



using	::tui::terminal;
using	::std::string;
using	::std::chrono::milliseconds;
using	::std::chrono::high_resolution_clock;
using	::std::this_thread::sleep_for;
using	color		=	::tui::terminal::color;
using	text_style	=	::tui::terminal::text_style;



int main( )
{{

	terminal terminal;
	terminal.set_color( color::cyan );

	try
	{
		int char_y = 10;
		int char_x = 10;

		bool exit_loop = false;
		while( true )
		{
			auto start_time = high_resolution_clock::now( );

			int code = terminal.get_char( );
			if( code != 0 )
			{
				switch( code >= 65 and code <= 90 ? code + 32 : code )
				{
					case 'a':
						--char_x;
						break;
					case 'd':
						++char_x;
						break;
					case 'w':
						--char_y;
						break;
					case 's':
						++char_y;
						break;
					case 'q':
						exit_loop = true;
						break;
				}
			}
			
			if( exit_loop )
				break;

			terminal.print( char_x, char_y, "\u2588" );
			terminal.refresh( );

			auto end_time = high_resolution_clock::now( );
			auto frame_time = ::std::chrono::duration_cast<milliseconds>( end_time - start_time );
			milliseconds target_frame_time = milliseconds{ 1000 } / 60;

			if( frame_time < target_frame_time )
				sleep_for( target_frame_time - frame_time );
		}
		terminal.clear_screen( );

		return 0;
	} catch( const ::std::exception& e )
	{
		std::cerr << "error: " << e.what( ) << "\n";
		return 1;
	}

}}

