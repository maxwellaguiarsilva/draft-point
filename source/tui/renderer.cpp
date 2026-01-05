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
 * File:   tui/renderer.cpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2026-01-03 13:03
 */


#include <tui/renderer.hpp>
#include <tui/terminal.hpp>
#include <sak/using.hpp>
#include <sak/math/math.hpp>
#include <cmath>
#include <ranges>


namespace tui {


__using( ::std::, lock_guard, make_shared, try_to_lock, uint8_t, unique_lock, vector, ranges::fill )
__using( ::sak::math, ::abs, ::sign )
__using( ::tui::, line, point, rectangle )


struct renderer::terminal_listener final : public terminal::listener
{
	explicit terminal_listener( renderer& parent ) : m_renderer( parent ) { }
	void on_resize( const point& size ) override { m_renderer.on_resize( size ); }
	renderer& m_renderer;
};


renderer::renderer( terminal& terminal )
	:m_terminal( terminal )
	,m_color( 15 )
	,m_terminal_listener( make_shared< terminal_listener >( *this ) )
{
	m_terminal += m_terminal_listener;
	on_resize( m_terminal.size( ) );
}

void renderer::clear( ) noexcept
{
	auto lock = lock_guard( m_mutex );
	fill( m_back, 0 );
}

void renderer::refresh( )
{
	unique_lock lock( m_mutex, try_to_lock );
	if( not lock.owns_lock( ) ) return;

	uint8_t current_foreground = 255;
	uint8_t current_background = 255;
	bool force_update = true;
	point cursor_position = { 0, 0 };

	const int width = m_terminal_size[ 0 ];
	const int height = m_terminal_size[ 1 ];

	for( int y = 0; y < height; ++y )
	{
		const int up_offset = ( 2 * y ) * width;
		const int down_offset = ( 2 * y + 1 ) * width;

		for( int x = 0; x < width; ++x )
		{
			const uint8_t b_up = m_back[ up_offset + x ];
			const uint8_t b_down = m_back[ down_offset + x ];
			uint8_t& f_up = m_front[ up_offset + x ];
			uint8_t& f_down = m_front[ down_offset + x ];

			if( b_up not_eq f_up or b_down not_eq f_down )
			{
				const point current = { x + 1, y + 1 };
				if( cursor_position not_eq current )
					m_terminal.move_cursor( current );

				if( b_up not_eq current_foreground or force_update )
				{
					current_foreground = b_up;
					m_terminal.set_color( current_foreground, false );
				}
				if( b_down not_eq current_background or force_update )
				{
					current_background = b_down;
					m_terminal.set_color( current_background, true );
				}

				m_terminal.print( "\xe2\x96\x80" );
				f_up = b_up;
				f_down = b_down;
				cursor_position = { current[ 0 ] + 1, current[ 1 ] };
				force_update = false;
			}
		}
	}
}

void renderer::set_color( const uint8_t color ) noexcept { m_color = color; }

void renderer::draw( const line& data )
{
	point difference = ( data.end - data.start ).map( abs );
	point step = ( data.end - data.start ).map( sign );

	int error = difference[ 0 ] - difference[ 1 ];
	point current = data.start;

	while( true )
	{
		draw( current );
		if( current == data.end ) break;

		int error_doubled = 2 * error;
		if( error_doubled > -difference[ 1 ] )
		{
			error -= difference[ 1 ];
			current[ 0 ] += step[ 0 ];
		}
		if( error_doubled < difference[ 0 ] )
		{
			error += difference[ 0 ];
			current[ 1 ] += step[ 1 ];
		}
	}
}

void renderer::draw( const rectangle& data, bool fill )
{
	if( fill )
	{
		for( int y = data.start[ 1 ]; y <= data.end[ 1 ]; ++y )
			for( int x = data.start[ 0 ]; x <= data.end[ 0 ]; ++x )
				draw( point{ x, y } );
	}
	else
	{
		for( int x = data.start[ 0 ]; x <= data.end[ 0 ]; ++x )
		{
			draw( point{ x, data.start[ 1 ] } );
			draw( point{ x, data.end[ 1 ] } );
		}
		for( int y = data.start[ 1 ]; y <= data.end[ 1 ]; ++y )
		{
			draw( point{ data.start[ 0 ], y } );
			draw( point{ data.end[ 0 ], y } );
		}
	}
}

void renderer::draw( const point& pixel ) noexcept
{
	auto lock = lock_guard( m_mutex );

	if( not m_screen_bounds.contains( pixel ) ) return;

	m_back[ ( pixel[ 1 ] - 1 ) * m_screen_size[ 0 ] + ( pixel[ 0 ] - 1 ) ] = m_color;
}

void renderer::on_resize( const point& size )
{
	{
		auto lock = lock_guard( m_mutex );
		m_terminal_size = size;
		m_screen_size = { m_terminal_size[ 0 ], 2 * m_terminal_size[ 1 ] };
		m_screen_bounds = { { 1, 1 }, m_screen_size };
		size_t count = m_screen_size[ 0 ] * m_screen_size[ 1 ];
		if( m_back.size( ) not_eq count )
		{
			m_back.resize( count );
			m_front.resize( count );
		}
	}
	clear( );
	refresh( );
	m_terminal.refresh( );
}


}


