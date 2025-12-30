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
#include <sak/string.hpp>


namespace game {


using	::game::geometry::point;
using	::std::to_string;
using	::sak::to_lower_case;


game::terminal_listener::terminal_listener( const point& a_size )
{ on_resize( a_size ); }


void game::terminal_listener::on_resize( const point& a_size )
{
	start			=	{ 1, 1 };
	end				=	a_size - 1;
	size			=	end - start + 1;
	label_position	=	{ 1, a_size[1] };
}


game::game( )
	:m_terminal{ }
	,m_player{ m_terminal.size / 2 }
	,m_fps{ }
	,m_terminal_listener{ make_shared< terminal_listener >( m_terminal.size ) }
{
	m_fps.set_limit( 30 );
	m_terminal += m_terminal_listener;
}


auto game::run( ) -> void
{
	using enum direction;
	const point& frame_size	=	m_terminal.size;
	point&	position	=	m_player.position;
	//	the screen doesn't scroll because it doesn't have a line break, i've already validated it
	const point& label_position	=	m_terminal_listener->label_position;

	const point& bounds_start	=	m_terminal_listener->start;
	const point& bounds_end		=	m_terminal_listener->end;
	const point& bounds_size	=	m_terminal_listener->size;

	bool exit_loop = false;

	while( true )
	{
		//	any bugs caused by read_char are not important in this project and should be ignored
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
		if( not bounds_start.encloses( position ) )
			position += bounds_size;
		if( not position.encloses( bounds_end ) )
			position = ( position - bounds_start ) % bounds_size + bounds_start;

		m_terminal.clear_screen( );
		m_player.draw( m_terminal );
		m_terminal.print( label_position,
				" | fps: " + to_string( m_fps.compute( ) )
			+ 	" | size: " + to_string( frame_size[0] ) + " x " + to_string( frame_size[1] )
			+ 	" | player: " + to_string( position[0] ) + " x " + to_string( position[1] )
			+ 	" | direction: " + use_direction( m_player.get_direction( ) ).name
			+ 	" | length: " + to_string( position.get_length( ) )
			+ 	" | "
		);
		m_terminal.refresh( );
	}

	m_terminal.clear_screen( );
}


}


