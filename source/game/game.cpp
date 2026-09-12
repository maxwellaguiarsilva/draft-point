//	
//	SPDX-FileCopyrightText: 2025 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	SPDX-License-Identifier: GPL-3.0-or-later
//	


#include <game/game.hpp>
#include <tui/terminal.hpp>
#include <sak/string.hpp>


namespace game {


using	::std::to_string;
using	::sak::to_lower_case;


game::renderer_listener::renderer_listener( const g2i::point& new_size )
{ resize( new_size ); }


void game::renderer_listener::resize( const g2i::point& new_size )
{
	start			=	{ 0, 0 };
	end				=	{ g2i::width( new_size ) - 1, g2i::height( new_size ) - 1 };
	size			=	end - start + 1;
	label_position	=	{ 1, 1 };
}


game::game( renderer& renderer )
	:m_renderer{ renderer }
	,m_player{ { g2i::width( m_renderer.size( ) ) / 2, g2i::height( m_renderer.size( ) ) } }
	,m_fps{ }
	,m_renderer_listener{ make_shared< renderer_listener >( m_renderer.size( ) ) }
{
	m_fps.limit( 60 );
	m_renderer += m_renderer_listener;
}


auto game::run( ) -> void
{
	using enum direction;
	const auto& frame_size	=	m_renderer.size( );
	auto&	position		=	m_player.position;
	
	const auto& listener		=	*m_renderer_listener;
	const auto& label_position	=	listener.label_position;
	const auto& bounds_start	=	listener.start;
	const auto& bounds_end		=	listener.end;
	const auto& bounds_size		=	listener.size;

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
				case 'w': m_player.direction( up );		break;
				case 'a': m_player.direction( left );	break;
				case 's': m_player.direction( down );	break;
				case 'd': m_player.direction( right );	break;
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

		m_renderer.clear( );
		m_player.draw( m_renderer );

		m_renderer.print( label_position,
				" | fps: " + to_string( m_fps.compute( ) )
			+ 	" | size: " + to_string( g2i::width( frame_size ) ) + " x " + to_string( g2i::height( frame_size ) )
			+ 	" | player: " + to_string( g2i::left( position ) ) + " x " + to_string( g2i::top( position ) )
			+ 	" | direction: " + use_direction( m_player.direction( ) ).name
			+ 	" | length: " + to_string( position.length( ) )
			+ 	" | "
		);
		m_renderer.refresh( );
	}

	m_renderer.clear( );
	m_renderer.refresh( );
}


}


