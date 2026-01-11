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
 * File:   tui/renderer.hpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2026-01-03 15:28
 */


#pragma once
#ifndef header_guard_794521060
#define header_guard_794521060


#include <sak/sak.hpp>
#include <sak/pattern/dispatcher.hpp>
#include <tui/geometry.hpp>
#include <game/renderer.hpp>
#include <vector>
#include <mutex>


namespace tui {


__using( ::std::
	,lock_guard
	,mutex
	,shared_ptr
	,size_t
	,string
	,vector
)
using	::sak::pattern::dispatcher;
using	::sak::byte;


class terminal;


class renderer : public ::game::renderer
{
public:
	using	buffer	=	vector< byte >;
	using	listener =	::game::renderer::listener;

	explicit renderer( terminal& parent );
	~renderer( ) noexcept override = default;

	void clear( ) noexcept override;
	void clear_screen( ) noexcept override;
	void refresh( ) override;
	void set_color( const byte color ) noexcept override;
	void draw( const point& pixel ) noexcept override;
	void draw( const line& segment ) noexcept override;
	void draw( const rectangle& area, bool is_filled = true ) noexcept override;
	void print( const point& position, const string& text ) noexcept override;

	auto size( ) const noexcept -> point override;

	static auto read_char( ) -> char;

	void operator +=( const shared_ptr< listener >& subject ) override;

private:
	struct terminal_listener;

	void on_resize( const point& size );
	void plot_unsafe( int column, int row ) noexcept;

	terminal&	m_terminal;
	buffer		m_front;
	buffer		m_back;
	point		m_terminal_size;
	point		m_screen_size;
	byte		m_color;
	mutable mutex	m_mutex;
	shared_ptr< terminal_listener > m_terminal_listener;
	dispatcher< listener >	m_dispatcher;
};


}


#endif


