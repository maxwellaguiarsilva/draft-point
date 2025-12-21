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


using	::geometry::point;
using	::std::to_string;
using	::sak::to_lower_case;


game::game( )
	:m_terminal{ }
	,m_player{ m_terminal.size / 2 }
	,m_fps{ }
{ m_fps.set_limit( 30 ); }


auto game::run( ) -> void
{
	using enum direction	=	::game::direction::direction;
	const point& frame_size	=	m_terminal.size;
	point&	position	=	m_player.position;
	point	zero		=	{ 0, 0 };
	//	the screen doesn't scroll because it doesn't have a line break, i've already validated it
	point	label_position	=	{ 1, frame_size[1] };

	bool exit_loop = false;

	while( true )
	{
		//	the bug with the special keys is not important to the purpose of this project
		char code = m_terminal.read_char( );
		if( code not_eq 0 )
		{
			switch( to_lower_case( code ) )
			{
				case 'w': m_player.set_direction( up );		break;
				case 'a': m_player.set_direction( left );	break;
				case 's': m_player.set_direction( down );	break;
				case 'd': m_player.set_direction( right );	break;
				case 'q': exit_loop = true; break;
			}
			if( exit_loop )
				break;
		}

		m_player.step_move( );
		if( not zero.is_all_less_equal( position ) )
			position += frame_size;
		position %= frame_size;

		m_terminal.clear_screen( );
		m_player.draw( m_terminal );
		m_terminal.print( label_position,
				" | fps: " + to_string( m_fps.compute( ) )
			+ 	" | width: " + to_string( frame_size[0] )
			+ 	" | height: " + to_string( frame_size[1] )
			+ 	" | direction: " + use_direction( player.get_direction( ) )
			+ 	" | length: " + to_string( position.get_length( ) )
			+ 	" | "
		);
		m_terminal.refresh( );
	}

	m_terminal.clear_screen( );
}


} 


