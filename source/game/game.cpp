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
 * File:   game/game.cpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-08-26 19:41
 */



#include <game/game.hpp>


namespace game {


using	::std::string;
using	::tui::terminal;
using	::game::player;
using	::game::fps;
using	::std::vector;
using	::std::to_string;
using	color	= 	::tui::terminal::color;


game::game( )
	:m_terminal{}
	,m_player{ m_terminal, 10, 10 }
	,m_fps{}
{ m_fps.set_limit( 30 );}
game::~game( ) { }


auto game::run( ) -> void
{
	bool exit_loop = false;
	while( true )
	{
		char code = m_terminal.get_char( );
		if( code != 0 )
		{
			switch( code >= 65 and code <= 90 ? code + 32 : code )
			{
				case 'a': m_player.move( player::movement::left );	break;
				case 'd': m_player.move( player::movement::right );	break;
				case 'w': m_player.move( player::movement::up );	break;
				case 's': m_player.move( player::movement::down );	break;
				case 'v': m_fps.enable_limit( not m_fps.is_enable( ) ); break;
				case 'q': exit_loop = true; break;
			}
		}
		
		if( exit_loop )
			break;

		m_terminal.clear_screen( );
		m_player.draw( );
		if( m_show_fps )
			m_terminal.print( 1, 1, "fps: " + to_string( m_fps.compute( ) ) );
		m_terminal.refresh( );
	}

	m_terminal.print( 0, 0, "           " );
	m_terminal.clear_screen( );
}


} 



