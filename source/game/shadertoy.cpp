/*
 * Copyright (C) 2026 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
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
 * File:   game/shadertoy.cpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2026-01-12 16:03
 */


#include <game/shadertoy.hpp>
#include <tui/terminal.hpp>
#include <sak/string.hpp>
#include <chrono>


namespace game {


using	::std::function;
using	::sak::byte;
using	::std::to_string;
using	::std::chrono::high_resolution_clock;
using	::std::chrono::duration;


shadertoy::shadertoy( renderer& renderer )
	:m_renderer{ renderer }
	,m_fps{ }
{
	m_fps.set_limit( 60 );
}


auto shadertoy::run( const function< g3f::point( g3f::point ) >& shader ) -> void
{
	auto last_time = high_resolution_clock::now( );

	while( true )
	{
		const auto current_time = high_resolution_clock::now( );
		const duration< float > delta = current_time - last_time;
		last_time = current_time;

		char code = ::tui::terminal::read_char( );
		if( code == 'q' or code == 'Q' or code == 27 )
			break;

		if( code == 'p' or code == 'P' )
			m_is_paused = not m_is_paused;

		if( not m_is_paused )
			m_time += delta.count( );

		m_renderer.fill_with( [ &shader, time = m_time ]( g2f::point coord ) -> g3f::point
		{
			return	shader( { coord[ 0 ], coord[ 1 ], time } );
		} );

		m_renderer.print( { 1, 1 }, " | fps: " + to_string( m_fps.compute( ) ) + " | time: " + to_string( m_time ) + ( m_is_paused ? " [PAUSED] | " : " | " ) );
		m_renderer.refresh( );
	}

	m_renderer.clear( );
	m_renderer.refresh( );
}


}


