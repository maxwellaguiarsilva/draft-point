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
	,unique_lock
	,vector
	,ranges::fill
	,ranges::max
	,views::iota
	,views::drop
	,views::take
	,views::transform
)
__using( ::sak::ranges::views::, cartesian_product )
__using( ::sak::math::, abs, sign, bind_back, greater_equal, between )
__using( ::sak::, to_point )
__using( ::sak::ranges::, chunk )


constexpr int width_index = 0, left_index = 0;
constexpr int height_index = 1, top_index = 1;


struct renderer::terminal_listener final : public terminal::listener
{
	explicit terminal_listener( renderer& parent ) : m_renderer( parent ) { }
	auto on_resize( const g2i::point& new_size ) -> void override { m_renderer.on_resize( new_size ); }
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


auto renderer::set_color( const byte color ) noexcept -> void { m_color = color; }


auto renderer::draw( const g2i::line& line ) noexcept -> void
{
	using	point	=	g2i::point;
	auto lock = lock_guard( m_mutex );
	const point difference = ( line.end - line.start );
	const point walker_step = difference | abs | to_point;
	const point step = difference | sign | to_point;
	const int total = max( walker_step );
	
	auto current = line.start;
	auto walker = walker_step;

	for( [[maybe_unused]] auto const iteration : iota( 0, total ) )
	{
		plot_unsafe( current[ width_index ], current[ height_index ] );
		const point direction = walker | bind_back( greater_equal, total ) | to_point;
		current += step * direction;
		walker += walker_step - direction * total;
	}
}


auto renderer::draw( const g2i::rectangle& area, bool is_filled ) noexcept -> void
{
	auto lock = lock_guard( m_mutex );
	auto const area_bound = area.end - area.start + 1;
	const auto crop_width = drop( area.start[ left_index ] ) | take( area_bound[ width_index ] );
	auto rows = chunk( m_back, m_screen_size[ width_index ] )
		| drop( area.start[ top_index ] )
		| take( area_bound[ height_index ] )
		| transform( crop_width );

	if( is_filled )
		for( auto row : rows )
			fill( row, m_color );
	else
	{
		fill( rows.front( ), m_color );
		fill( rows.back( ),  m_color );

		for( auto row : iota( area.start[ top_index ], area.end[ top_index ] + 1 ) )
		{
			plot_unsafe( area.start[ left_index ], row );
			plot_unsafe( area.end[ left_index ],   row );
		}
	}
}

auto renderer::draw( const g2i::point& pixel ) noexcept -> void
{
	auto lock = lock_guard( m_mutex );
	plot_unsafe( pixel[ left_index ], pixel[ top_index ] );
}

auto renderer::print( const g2i::point& position, const string& text ) noexcept -> void
{
	auto lock = lock_guard( m_mutex );
	m_terminal.print( position, text );
}

auto renderer::size( ) const noexcept -> g2i::point
{
	auto lock = lock_guard( m_mutex );
	return	m_screen_size;
}

auto renderer::plot_unsafe( int column, int row ) noexcept -> void
{
	const size_t index = row * m_screen_size[ width_index ] + column;
	if( index < m_back.size( ) )
		m_back[ index ] = m_color;
}

auto renderer::on_resize( const g2i::point& new_size ) -> void
{
	{
		auto lock = lock_guard( m_mutex );
		m_terminal_size = new_size;
		m_screen_size = { m_terminal_size[ width_index ], 2 * m_terminal_size[ height_index ] };
		size_t total_pixel_count = m_screen_size[ width_index ] * m_screen_size[ height_index ];
		if( m_back.size( ) not_eq total_pixel_count )
		{
			m_back.resize( total_pixel_count );
			m_front.resize( total_pixel_count );
		}
	}
	renderer::clear( );
	renderer::refresh( );
	m_dispatcher( &listener::on_resize, m_screen_size );
}

auto renderer::refresh( ) -> void
{
	using	point	=	g2i::point;
	unique_lock lock( m_mutex, try_to_lock );
	if( not lock.owns_lock( ) ) return;

	byte current_foreground = 255;
	byte current_background = 255;
	bool force_update = true;
	point cursor_position = { 0, 0 };

	const int terminal_width = m_terminal_size[ width_index ];
	const int terminal_height = m_terminal_size[ height_index ];

	auto back_rows = chunk( m_back, terminal_width );
	auto front_rows = chunk( m_front, terminal_width );

	for( auto [ row, column ] : cartesian_product( iota( 0, terminal_height ), iota( 0, terminal_width ) ) )
	{
		const byte back_upper = back_rows[ 2 * row ][ column ];
		const byte back_lower = back_rows[ 2 * row + 1 ][ column ];
		byte& front_upper = front_rows[ 2 * row ][ column ];
		byte& front_lower = front_rows[ 2 * row + 1 ][ column ];

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
	m_terminal.refresh( );
}

void renderer::operator +=( const shared_ptr< listener >& subject ) { m_dispatcher += subject; }


}


