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
using	::std::lock_guard;
using	::std::unique_lock;
using	::std::try_to_lock;


renderer::renderer( terminal& parent )
	:m_parent( parent )
	,m_color( 15 )
	,m_is_resizing( false )
{
	point size = m_parent.size( );
	m_front.resize( size[ 0 ] * size[ 1 ] );
	m_back.resize( size[ 0 ] * size[ 1 ] );
}

void renderer::clear( ) noexcept
{
	auto lock = lock_guard( m_mutex );
	for( auto& cell : m_back )
	{
		cell.up = 0;
		cell.down = 0;
	}
}

void renderer::refresh( )
{
	if( m_is_resizing.load( ) ) return;
	unique_lock lock( m_mutex, try_to_lock );
	if( not lock.owns_lock( ) ) return;

	point size = m_parent.size( );
	int width = size[ 0 ];
	int height = size[ 1 ];
	int count = width * height;

	if( m_back.size( ) not_eq static_cast< size_t >( count ) )
	{
		m_back.resize( static_cast< size_t >( count ) );
		m_front.resize( static_cast< size_t >( count ) );
	}

	uint8_t current_foreground = 255;
	uint8_t current_background = 255;
	bool force_update = true;
	point current_position = { 0, 0 };

	for( int row = 1; row <= height; ++row )
		for( int column = 1; column <= width; ++column )
		{
			int index = ( row - 1 ) * width + ( column - 1 );
			if( m_back[ index ] not_eq m_front[ index ] )
			{
				if( current_position[ 0 ] not_eq column or current_position[ 1 ] not_eq row )
					m_parent.move_cursor( { column, row } );

				if( m_back[ index ].up not_eq current_foreground or force_update )
				{
					current_foreground = m_back[ index ].up;
					m_parent.m_output << "\033[38;5;" << static_cast< int >( current_foreground ) << "m";
				}
				if( m_back[ index ].down not_eq current_background or force_update )
				{
					current_background = m_back[ index ].down;
					m_parent.m_output << "\033[48;5;" << static_cast< int >( current_background ) << "m";
				}

				m_parent.m_output << "\xe2\x96\x80";
				m_front[ index ] = m_back[ index ];
				current_position = { column + 1, row };
				force_update = false;
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

	int delta_x = abs( x2 - x1 );
	int delta_y = -abs( y2 - y1 );
	int step_x = x1 < x2 ? 1 : -1;
	int step_y = y1 < y2 ? 1 : -1;
	int error = delta_x + delta_y;

	while( true )
	{
		draw( { x1, y1 } );
		if( x1 == x2 and y1 == y2 ) break;
		int error_doubled = 2 * error;
		if( error_doubled >= delta_y )
		{
			error += delta_y;
			x1 += step_x;
		}
		if( error_doubled <= delta_x )
		{
			error += delta_x;
			y1 += step_y;
		}
	}
}

void renderer::draw( const rectangle& data, bool fill )
{
	if( fill )
	{
		for( int y = data.start[ 1 ]; y <= data.end[ 1 ]; ++y )
			for( int x = data.start[ 0 ]; x <= data.end[ 0 ]; ++x )
				draw( { x, y } );
	}
	else
	{
		for( int x = data.start[ 0 ]; x <= data.end[ 0 ]; ++x )
		{
			draw( { x, data.start[ 1 ] } );
			draw( { x, data.end[ 1 ] } );
		}
		for( int y = data.start[ 1 ]; y <= data.end[ 1 ]; ++y )
		{
			draw( { data.start[ 0 ], y } );
			draw( { data.end[ 0 ], y } );
		}
	}
}

void renderer::draw( const point& data ) noexcept
{
	int x = data[ 0 ];
	int y = data[ 1 ];

	point size = m_parent.size( );
	int width = size[ 0 ];
	int height = size[ 1 ];

	if( x < 1 or x > width or y < 1 or y > 2 * height ) return;

	int row = ( y + 1 ) / 2;
	int column = x;
	int index = ( row - 1 ) * width + ( column - 1 );

	auto lock = lock_guard( m_mutex );
	if( y % 2 not_eq 0 )
		m_back[ index ].up = m_color;
	else
		m_back[ index ].down = m_color;
}

void renderer::on_resize( const point& size )
{
	m_is_resizing.store( true );
	{
		auto lock = lock_guard( m_mutex );
		m_front.assign( size[ 0 ] * size[ 1 ], { 0, 0 } );
		m_back.assign( size[ 0 ] * size[ 1 ], { 0, 0 } );
	}
	m_is_resizing.store( false );
}


}


