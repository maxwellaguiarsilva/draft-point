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


using	::tui::terminal;
using	::game::player;
using	::game::fps;
using	::std::to_string;
using	::sak::to_lower_case;
using	color	= 	::tui::terminal::color;


game::game( )
	:m_terminal{ }
	,m_player{ m_terminal.get_size( ) / 2 }
	,m_fps{ }
{ m_fps.set_limit( 30 ); }


auto game::run( ) -> void
{
	using	direction	=	::game::direction::direction_value;
	bool exit_loop = false;

	while( true )
	{
		char code = m_terminal.get_char( );
		if( code not_eq 0 )
		{
			switch( to_lower_case( code ) )
			{
				case 'w': m_player.set_direction( direction::up );	break;
				case 'a': m_player.set_direction( direction::left );	break;
				case 's': m_player.set_direction( direction::down );	break;
				case 'd': m_player.set_direction( direction::right );break;
				case 'q': exit_loop = true; break;
			}
			if( exit_loop )
				break;
		}


		m_player.step_move( );

		m_terminal.clear_screen( );
		m_player.draw( m_terminal );
		m_terminal.print( point( {1, m_terminal.get_size( )[1]} ),
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


