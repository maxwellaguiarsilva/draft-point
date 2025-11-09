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
using	::std::to_string;
using	color	= 	::tui::terminal::color;


const point direction::up{ 0, -1 };
const point direction::down{ 0, 1 };
const point direction::left{ -1, 0 };
const point direction::right{ 1, 0 };


game::game( )
	:m_terminal{ }
	,m_player{ m_terminal.get_size( ), point{ 10, 10 } }
	,m_fps{ }
{ m_fps.set_limit( 30 ); }

game::~game( ) noexcept { }


auto game::run( ) -> void
{
	bool exit_loop = false;
	const int delta_lower_case	=	( 'a' - 'A' );

	while( true )
	{
		char code = m_terminal.get_char( );
		if( code not_eq 0 )
		{
			switch( between( code, 'A', 'Z' ) ? code - delta_lower_case : code )
			{
				case 'a': m_player.move( direction::left );	break;
				case 'd': m_player.move( direction::right );	break;
				case 'w': m_player.move( direction::up );		break;
				case 's': m_player.move( direction::down );	break;
				case 'q': exit_loop = true; break;
			}
		}
		
		if( exit_loop )
			break;

		m_terminal.clear_screen( );
		m_player.draw( m_terminal );
		m_terminal.print( 1, m_terminal.get_size( )[1],
				" | fps: " + to_string( m_fps.compute( ) )
			+ 	" | width: " + to_string( m_terminal.get_size( )[0] )
			+ 	" | height: " + to_string( m_terminal.get_size( )[1] )
			+ 	" | "
		);
		m_terminal.refresh( );
	}

	m_terminal.clear_screen( );
}


} 


