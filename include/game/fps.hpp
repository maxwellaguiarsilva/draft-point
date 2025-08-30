/*
 * Copyright (C) 2025 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
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
 * File:   game/fps.hpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-08-26 20:15
 */



#pragma once
#ifndef header_guard_602704350
#define header_guard_602704350

#include <sak.hpp>
#include <string>
#include <tui/terminal.hpp>
#include <chrono>
#include <thread>


namespace game {


using	::std::string;
using	::tui::terminal;
using	::std::chrono::milliseconds;
using	::std::chrono::high_resolution_clock;
using	::std::chrono::duration_cast;
using	::std::this_thread::sleep_for;


class fps
{
public:

	explicit fps( terminal& terminal );
	virtual ~fps( );

	disable_copy_move_ctc( fps );

	void show( int left = 0, int top = 0 );
	void compute( );
	void hide( );

private:
	terminal&	m_terminal;
	int			m_top		=	0;
	int			m_left		=	0;
	int			m_limit		=	60;
	high_resolution_clock::time_point	m_start_time;
	bool						m_enable	=	true;

};


} 


#endif


