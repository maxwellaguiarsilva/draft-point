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
 * Created on 2026-01-03 13:03
 */



#pragma once
#ifndef header_guard_794521060
#define header_guard_794521060

#include <sak/sak.hpp>
#include <tui/geometry.hpp>
#include <vector>
#include <cstdint>



namespace tui {



using	::std::vector;
using	::std::uint8_t;



class terminal;


class renderer
{
public:
	struct cell
	{
		uint8_t	up;
		uint8_t	down;

		constexpr auto operator ==( const cell& ) const noexcept -> bool = default;
	};

	using	buffer	=	vector< cell >;


	explicit renderer( terminal& parent );

	void clear( ) noexcept;
	void refresh( );
	void set_color( const uint8_t color ) noexcept;
	void draw( const line& data );
	void draw( const rectangle& data, bool fill = true );


private:
	void draw_point( int x, int y ) noexcept;

	terminal&	m_parent;
	buffer		m_front;
	buffer		m_back;
	uint8_t		m_color;

};


} 


#endif


