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
 * Created on 2026-01-03 15:28
 */


#include <tui/renderer.hpp>
#include <cmath>
#include <ranges>
#include <tui/terminal.hpp>
#include <sak/using.hpp>
#include <sak/math/math.hpp>
#include <sak/ranges/chunk.hpp>
#include <sak/ranges/views/cartesian_product.hpp>


namespace tui {


__using( ::std::
	,lock_guard
	,make_shared
	,try_to_lock
	,uint8_t
	,unique_lock
	,vector
	,ranges::fill
	,ranges::max
	,views::iota
	,views::drop
	,views::take
)
__using( ::sak::ranges::views::, cartesian_product )
__using( ::sak::math::, abs, sign, bind_back, greater_equal )
__using( ::sak::, to_point )
__using( ::sak::ranges::, chunk )
__using( ::tui::, line, point, rectangle )


struct renderer::terminal_listener final : public terminal::listener
{
	explicit terminal_listener( renderer& parent ) : m_renderer( parent ) { }
	auto on_resize( const point& size ) -> void override { m_renderer.on_resize( size ); }
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

auto renderer::clear( ) noexcept -> void
{
	auto lock = lock_guard( m_mutex );
	fill( m_back, 0 );
}

auto renderer::set_color( const uint8_t color ) noexcept -> void { m_color = color; }

auto renderer::draw( const line& segment ) noexcept -> void
{
	auto lock = lock_guard( m_mutex );
	const point difference = ( segment.end - segment.start );
	const point walker_step = difference | abs | to_point;
	const point step = difference | sign | to_point;
	const auto total = max( walker_step );
	
	point current = segment.start;
	point walker = walker_step;
	
	auto count = total + 1;
	while( --count > 0 )
	{
		plot_unsafe( current[ 0 ], current[ 1 ] );
		const point direction = walker | bind_back( greater_equal, total ) | to_point;
		current += step * direction;
		walker += walker_step - direction * total;
	}
}

auto renderer::draw( const rectangle& area, bool is_filled ) noexcept -> void
{
	auto lock = lock_guard( m_mutex );
	if( is_filled )
		for( auto row : chunk( m_back, m_screen_size[ 0 ] ) | drop( area.start[ 1 ] ) | take( area.end[ 1 ] - area.start[ 1 ] + 1 ) )
			fill( row | drop( area.start[ 0 ] ) | take( area.end[ 0 ] - area.start[ 0 ] + 1 ), m_color );
	else
	{
		auto rows = chunk( m_back, m_screen_size[ 0 ] );
		auto const column_start = area.start[ 0 ];
		auto const column_count = area.end[ 0 ] - area.start[ 0 ] + 1;

		fill( rows[ area.start[ 1 ] ] | drop( column_start ) | take( column_count ), m_color );
		fill( rows[ area.end[ 1 ] ] | drop( column_start ) | take( column_count ), m_color );

		for( int row = area.start[ 1 ]; row <= area.end[ 1 ]; ++row )
		{
			plot_unsafe( area.start[ 0 ], row );
			plot_unsafe( area.end[ 0 ], row );
		}
	}
}

auto renderer::draw( const point& pixel ) noexcept -> void
{
	auto lock = lock_guard( m_mutex );
	plot_unsafe( pixel[ 0 ], pixel[ 1 ] );
}

auto renderer::plot_unsafe( int column, int row ) noexcept -> void
{
	const size_t index = row * m_screen_size[ 0 ] + column;
	if( index < m_back.size( ) )
		m_back[ index ] = m_color;
}

auto renderer::on_resize( const point& size ) -> void
{
	{
		auto lock = lock_guard( m_mutex );
		m_terminal_size = size;
		m_screen_size = { m_terminal_size[ 0 ], 2 * m_terminal_size[ 1 ] };
		size_t total_pixel_count = m_screen_size[ 0 ] * m_screen_size[ 1 ];
		if( m_back.size( ) not_eq total_pixel_count )
		{
			m_back.resize( total_pixel_count );
			m_front.resize( total_pixel_count );
		}
	}
	clear( );
	refresh( );
	m_terminal.refresh( );
}

auto renderer::refresh( ) -> void
{
	unique_lock lock( m_mutex, try_to_lock );
	if( not lock.owns_lock( ) ) return;

	uint8_t current_foreground = 255;
	uint8_t current_background = 255;
	bool force_update = true;
	point cursor_position = { 0, 0 };

	const int terminal_width = m_terminal_size[ 0 ];
	const int terminal_height = m_terminal_size[ 1 ];

	auto back_rows = chunk( m_back, terminal_width );
	auto front_rows = chunk( m_front, terminal_width );

	for( int row = 0; row < terminal_height; ++row )
	{
		auto back_upper_row = back_rows[ 2 * row ];
		auto back_lower_row = back_rows[ 2 * row + 1 ];
		auto front_upper_row = front_rows[ 2 * row ];
		auto front_lower_row = front_rows[ 2 * row + 1 ];

		for( int column = 0; column < terminal_width; ++column )
		{
			const uint8_t back_upper = back_upper_row[ column ];
			const uint8_t back_lower = back_lower_row[ column ];
			uint8_t& front_upper = front_upper_row[ column ];
			uint8_t& front_lower = front_lower_row[ column ];

			if( back_upper not_eq front_upper or back_lower not_eq front_lower )
			{
				const point current = { column + 1, row + 1 };
				if( cursor_position not_eq current )
					m_terminal.move_cursor( current );

				if( back_upper not_eq current_foreground or force_update )
				{
					current_foreground = back_upper;
					m_terminal.set_color( current_foreground, false );
				}
				if( back_lower not_eq current_background or force_update )
				{
					current_background = back_lower;
					m_terminal.set_color( current_background, true );
				}

				m_terminal.print( "\xe2\x96\x80" );
				front_upper = back_upper;
				front_lower = back_lower;
				cursor_position = { current[ 0 ] + 1, current[ 1 ] };
				force_update = false;
			}
		}
	}
}


}


