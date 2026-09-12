//	
//	SPDX-FileCopyrightText: 2026 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	SPDX-License-Identifier: GPL-3.0-or-later
//	


#include <game/player.hpp>


namespace game {


using	enum	::game::direction;
using	::game::use_direction;

player::player( const g2i::point& position ) noexcept
	:position{ position }
	,m_direction{ right }
{ }


auto player::direction( ) const noexcept -> const ::game::direction& { return m_direction; }
auto player::direction( const ::game::direction& heading ) noexcept -> void { m_direction = heading; }

auto player::step_move( ) noexcept -> void { position += use_direction( m_direction ).point; }

auto player::draw( renderer& renderer ) const noexcept -> void
{
	renderer.color( 15 );
	renderer.draw( position );
}


}


