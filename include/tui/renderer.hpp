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
#include <tui/geometry.hpp>
#include <vector>
#include <cstdint>
#include <cstddef>
#include <mutex>
#include <memory>


namespace tui {


__using( ::std::, lock_guard, mutex, shared_ptr, size_t, uint8_t, vector )


class terminal;


class renderer
{
public:
	using	buffer	=	vector< uint8_t >;

	explicit renderer( terminal& parent );
	~renderer( ) noexcept = default;

	void clear( ) noexcept;
	void refresh( );
	void set_color( const uint8_t color ) noexcept;
	void draw( const point& pixel ) noexcept;
	void draw( const line& segment ) noexcept;
	void draw( const rectangle& area, bool fill = true ) noexcept;

private:
	struct terminal_listener;

	void on_resize( const point& size );
	void plot_unsafe( int column, int row ) noexcept;

	terminal&	m_terminal;
	buffer		m_front;
	buffer		m_back;
	point		m_terminal_size;
	point		m_screen_size;
	uint8_t		m_color;
	mutable mutex	m_mutex;
	shared_ptr< terminal_listener > m_terminal_listener;
};


}


#endif


