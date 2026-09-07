//	
//	Copyright (C) 2026 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	
//	This program is free software: you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//	
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//	
//	You should have received a copy of the GNU General Public License
//	along with this program.  If not, see <http://www.gnu.org/licenses/>.
//	
//	
//	File:   source/tui/renderer.cpp
//	Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	
//	Created on 2026-01-03 15:28:53
//	


#include <tui/renderer.hpp>
#include <ranges>
#include <tui/terminal.hpp>
#include <sak/using.hpp>
#include <sak/ranges/transform.hpp>


namespace tui {


__using( ::std::
	,lock_guard
	,make_shared
	,try_to_lock
	,unique_lock
	,vector
	,ranges::equal
	,ranges::fill
	,ranges::max
	,views::cartesian_product
	,views::chunk
	,views::iota
	,views::drop
	,views::take
	,views::zip
)
__using( ::sak::ranges::, lazy_transform )
__using( ::sak::, line_to )


constexpr int width_index = 0, left_index = 0;
constexpr int height_index = 1, top_index = 1;


struct renderer::terminal_listener final : public terminal::listener
{
	explicit terminal_listener( renderer& parent ) : m_renderer( parent ) { }
	auto resize( const g2i::point& new_size ) -> void override { m_renderer.resize( new_size ); }
	renderer& m_renderer;
};


renderer::renderer( terminal& terminal )
	:m_terminal( terminal )
	,m_margin( { 0, 1 } )
	,m_color( 15 )
	,m_terminal_listener( make_shared< terminal_listener >( *this ) )
{
	m_terminal += m_terminal_listener;
	resize( m_terminal.size( ) );
}


auto renderer::clear( const byte value ) noexcept -> void
{
	auto lock = lock_guard( m_mutex );
	fill( m_main, value );
}


auto renderer::color( const byte value ) noexcept -> void { m_color = value; }


auto renderer::draw( const g2i::line& line ) noexcept -> void
{
	auto lock = lock_guard( m_mutex );
	for( const auto& pixel : line.start | line_to( line.end ) )
		plot_unsafe( pixel[ width_index ], pixel[ height_index ] );
}


auto renderer::draw( const g2i::rectangle& area, bool is_filled ) noexcept -> void
{
	auto lock = lock_guard( m_mutex );
	auto const area_bound = area.end - area.start + 1;
	const auto crop_width = drop( area.start[ left_index ] ) | take( area_bound[ width_index ] );
	auto rows = m_main | chunk( m_screen_size[ width_index ] )
		| drop( area.start[ top_index ] )
		| take( area_bound[ height_index ] )
		| lazy_transform( crop_width );

	if( is_filled )
	{
		for( auto row : rows )
			fill( row, m_color );
		return;
	}

	fill( rows.front( ), m_color );
	fill( rows.back( ),  m_color );

	for( auto row : iota( area.start[ top_index ], area.end[ top_index ] + 1 ) )
	{
		plot_unsafe( area.start[ left_index ], row );
		plot_unsafe( area.end[ left_index ],   row );
	}
}

auto renderer::draw( const g2i::point& pixel ) noexcept -> void
{
	auto lock = lock_guard( m_mutex );
	plot_unsafe( pixel[ left_index ], pixel[ top_index ] );
}

auto renderer::print( const g2i::point& position, const string& text ) noexcept -> void
{
	using	enum	::tui::terminal::text_style;
	auto lock = lock_guard( m_mutex );
	m_terminal.style( reset );
	m_terminal.print( position, text );
}

auto renderer::fill_with( const function< byte( g2i::point ) >& shader ) noexcept -> void
{
	auto lock = lock_guard( m_mutex );
	const int width = m_screen_size[ width_index ];
	const int height = m_screen_size[ height_index ];
	for( auto [ row, column ] : cartesian_product( iota( 0, height ), iota( 0, width ) ) )
		m_main[ row * width + column ] = shader( g2i::point{ column, row } );
}

auto renderer::size( ) const noexcept -> g2i::point
{
	auto lock = lock_guard( m_mutex );
	return	m_screen_size;
}

auto renderer::plot_unsafe( int column, int row ) noexcept -> void
{
	const size_t index = row * m_screen_size[ width_index ] + column;
	if( index < m_main.size( ) )
		m_main[ index ] = m_color;
}

auto renderer::resize( const g2i::point& new_size ) -> void
{
	{
		auto lock = lock_guard( m_mutex );
		m_terminal_size = new_size;
		m_screen_size = ( m_terminal_size - m_margin * 2 ) * g2i::point{ 1, 2 };
		const size_t total_pixel_count = m_screen_size.product( );
		if( m_main.size( ) not_eq total_pixel_count )
		{
			m_main.resize( total_pixel_count );
			m_copy.resize( total_pixel_count );
		}
	}
	renderer::clear( );
	renderer::refresh( );
	( void )m_dispatcher( &listener::resize, m_screen_size );
}

auto renderer::refresh( ) -> void
{
	using	enum	::tui::terminal::text_style;
	using	point	=	g2i::point;
	unique_lock lock( m_mutex, try_to_lock );
	if( not lock.owns_lock( ) ) return;

	m_terminal.style( reset );
	point cursor_position	=	{ 0, 0 };
	auto grid_view = chunk( m_screen_size[ width_index ] ) | chunk( 2 );

	for( auto [ row, main_row, copy_row ] : zip( iota( m_margin[ top_index ] + 1 ), m_main | grid_view, m_copy | grid_view ) )
		for( auto [ column, main_upper, main_lower, copy_upper, copy_lower ] : zip(
			 iota( m_margin[ left_index ] + 1 )
			,main_row.front( )
			,main_row.back( )
			,copy_row.front( )
			,copy_row.back( )
		) )
			if( main_upper not_eq copy_upper or main_lower not_eq copy_lower )
			{
				copy_upper = main_upper;
				copy_lower = main_lower;
				
				const point current = point{ column, row };
				if( cursor_position not_eq current )
					m_terminal.move_cursor( current );

				m_terminal.color( main_upper, main_lower );
				m_terminal.print( "\xe2\x96\x80" );
				cursor_position = { current[ width_index ] + 1, current[ height_index ] };
			}
	
	m_terminal.refresh( );
}

void renderer::operator +=( const shared_ptr< listener >& subject ) { m_dispatcher += subject; }


}


