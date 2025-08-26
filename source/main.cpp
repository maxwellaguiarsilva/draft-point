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
 * File:   main.cpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-08-02 10:00
 */



#include <string>
#include <chrono>
#include <thread>
#include <tui/terminal.hpp>
#include <iostream>
#include <game/player.hpp>
#include <game/fps.hpp>


using	::tui::terminal;
using	::std::string;
using	color		= 	::tui::terminal::color;
using	text_style	= 	::tui::terminal::text_style;
using	::game::player;
using	::game::fps;



int main( )
{{

	terminal terminal;
	terminal.set_color( color::cyan );

	try
	{
		player player{ terminal, 10, 10 };
		player.draw( );

		fps fps{ terminal };
		fps.show( 0, 0 );

		bool exit_loop = false;
		while( true )
		{
			char code = terminal.get_char( );
			if( code != 0 )
			{
				switch( code >= 65 and code <= 90 ? code + 32 : code )
				{
					case 'a': player.move( player::movement::left ); break;
					case 'd': player.move( player::movement::right ); break;
					case 'w': player.move( player::movement::up ); break;
					case 's': player.move( player::movement::down ); break;
					case 'q': exit_loop = true; break;
				}
			}
			
			if( exit_loop )
				break;

			fps.compute( );
			terminal.refresh( );
		}
		fps.hide( );
		terminal.clear_screen( );

	} catch( const ::std::exception& e )
	{
		std::cerr << "error: " << e.what( ) << "\n";
		return	1;
	}

	return	0;

}}



