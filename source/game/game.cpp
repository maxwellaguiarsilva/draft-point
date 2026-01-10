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
 * Created on 2025-08-26 19:31
 */


#include <game/game.hpp>
#include <sak/string.hpp>


namespace game {


using	::game::point;
using	::std::to_string;
using	::sak::to_lower_case;
__using( ::game::, line, point, rectangle )


game::renderer_listener::renderer_listener( const point& new_size )
{ on_resize( new_size ); }


void game::renderer_listener::on_resize( const point& new_size )
{
	start			=	{ 0, 0 };
	end				=	{ new_size[ 0 ] - 1, new_size[ 1 ] - 1 };
	size			=	end - start + 1;
	label_position	=	{ 1, new_size[ 1 ] / 2 - 1 };
}


game::game( renderer& renderer )
	:m_renderer{ renderer }
	,m_player{ { m_renderer.size( )[ 0 ] / 2, m_renderer.size( )[ 1 ] } }
	,m_fps{ }
	,m_renderer_listener{ make_shared< renderer_listener >( m_renderer.size( ) ) }
{
	m_fps.set_limit( 60 );
	m_renderer += m_renderer_listener;
}


auto game::run( ) -> void
{
	using enum direction;
	const point& frame_size	=	m_renderer.size( );
	auto& game_renderer		=	m_renderer;
	point&	position		=	m_player.position;
	
	const auto& l_listener		=	*m_renderer_listener;
	const point& label_position	=	l_listener.label_position;
	const point& bounds_start	=	l_listener.start;
	const point& bounds_end		=	l_listener.end;
	const point& bounds_size	=	l_listener.size;

	bool exit_loop = false;

	while( true )
	{
		//	this coupling between the game and the terminal is temporary, and read_char is used only once
		//	for this reason, the violation of the `__using` rule was permitted
		//	any bugs caused by read_char are not important in this project and should be ignored
		char code	=	::tui::terminal::read_char( );

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
		if( not bounds_start.is_inside( position ) )
			position += bounds_size;
		if( not position.is_inside( bounds_end ) )
			position = ( position - bounds_start ) % bounds_size + bounds_start;

		game_renderer.clear( );
		m_player.draw( game_renderer );

		game_renderer.draw( line{ { 0, 0 }, position - 2 } );

		game_renderer.print( label_position,
				" | fps: " + to_string( m_fps.compute( ) )
			+ 	" | size: " + to_string( frame_size[ 0 ] ) + " x " + to_string( frame_size[ 1 ] )
			+ 	" | player: " + to_string( position[ 0 ] ) + " x " + to_string( position[ 1 ] )
			+ 	" | direction: " + use_direction( m_player.get_direction( ) ).name
			+ 	" | length: " + to_string( position.get_length( ) )
			+ 	" | "
		);
		game_renderer.refresh( );
	}

	m_renderer.clear_screen( );
}


}


