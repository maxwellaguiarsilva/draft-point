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
#include <cmath>


namespace tui {


using	::std::vector;
using	::std::uint8_t;
using	::std::abs;
using	::tui::point;
using	::tui::line;
using	::tui::rectangle;


renderer::renderer( terminal& parent )
	:	m_parent( parent )
	,	m_color( 15 )
{
	point size = m_parent.size( );
	m_front.resize( size[ 0 ] * size[ 1 ] );
	m_back.resize( size[ 0 ] * size[ 1 ] );
}

void renderer::clear( ) noexcept
{
	for( auto& cell : m_back )
	{
		cell.up = 0;
		cell.down = 0;
	}
}

void renderer::refresh( )
{
	point size = m_parent.size( );
	size_t width = static_cast< size_t >( size[ 0 ] );
	size_t height = static_cast< size_t >( size[ 1 ] );
	size_t count = width * height;

	if( m_back.size( ) not_eq count )
	{
		m_back.resize( count );
		m_front.resize( count );
	}

	uint8_t current_fg = 255;
	uint8_t current_bg = 255;
	bool force_update = true;
	point current_pos = { 0, 0 };

	for( size_t row = 1; row <= height; ++row )
	{
		for( size_t col = 1; col <= width; ++col )
		{
			size_t idx = ( row - 1 ) * width + ( col - 1 );
			if( m_back[ idx ] not_eq m_front[ idx ] )
			{
				if( current_pos[ 0 ] not_eq static_cast< int >( col ) or current_pos[ 1 ] not_eq static_cast< int >( row ) )
					m_parent.move_cursor( { static_cast< int >( col ), static_cast< int >( row ) } );

				if( m_back[ idx ].up not_eq current_fg or force_update )
				{
					current_fg = m_back[ idx ].up;
					m_parent.m_output << "\033[38;5;" << static_cast< int >( current_fg ) << "m";
				}
				if( m_back[ idx ].down not_eq current_bg or force_update )
				{
					current_bg = m_back[ idx ].down;
					m_parent.m_output << "\033[48;5;" << static_cast< int >( current_bg ) << "m";
				}

				m_parent.m_output << "\xe2\x96\x80";
				m_front[ idx ] = m_back[ idx ];
				current_pos = { static_cast< int >( col + 1 ), static_cast< int >( row ) };
				force_update = false;
			}
		}
	}
}

void renderer::set_color( const uint8_t color ) noexcept { m_color = color; }

void renderer::draw( const line& data )
{
	int x1 = data.start[ 0 ];
	int y1 = data.start[ 1 ];
	int x2 = data.end[ 0 ];
	int y2 = data.end[ 1 ];

	int dx = abs( x2 - x1 );
	int dy = -abs( y2 - y1 );
	int sx = x1 < x2 ? 1 : -1;
	int sy = y1 < y2 ? 1 : -1;
	int err = dx + dy;

	while( true )
	{
		draw_point( x1, y1 );
		if( x1 == x2 and y1 == y2 ) break;
		int e2 = 2 * err;
		if( e2 >= dy )
		{
			err += dy;
			x1 += sx;
		}
		if( e2 <= dx )
		{
			err += dx;
			y1 += sy;
		}
	}
}

void renderer::draw( const rectangle& data, bool fill )
{
	if( fill )
	{
		for( int y = data.start[ 1 ]; y <= data.end[ 1 ]; ++y )
			for( int x = data.start[ 0 ]; x <= data.end[ 0 ]; ++x )
				draw_point( x, y );
	}
	else
	{
		for( int x = data.start[ 0 ]; x <= data.end[ 0 ]; ++x )
		{
			draw_point( x, data.start[ 1 ] );
			draw_point( x, data.end[ 1 ] );
		}
		for( int y = data.start[ 1 ]; y <= data.end[ 1 ]; ++y )
		{
			draw_point( data.start[ 0 ], y );
			draw_point( data.end[ 0 ], y );
		}
	}
}

void renderer::draw_point( int x, int y ) noexcept
{
	point size = m_parent.size( );
	int width = size[ 0 ];
	int height = size[ 1 ];

	if( x < 1 or x > width or y < 1 or y > 2 * height ) return;

	int r = ( y + 1 ) / 2;
	int c = x;
	size_t idx = static_cast< size_t >( ( r - 1 ) * width + ( c - 1 ) );

	if( y % 2 not_eq 0 )
		m_back[ idx ].up = m_color;
	else
		m_back[ idx ].down = m_color;
}

}


