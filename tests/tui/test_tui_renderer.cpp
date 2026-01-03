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
 * File:   tests/tui/test_tui_renderer.cpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2026-01-03 13:08
 */


#include <game/fps.hpp>
#include <tui/terminal.hpp>
#include <tui/renderer.hpp>
#include <chrono>
#include <thread>


auto main( ) -> int
{{

	try
	{
		using namespace ::tui;
		::game::fps fps( 30 );

		terminal term;
		renderer& rend = term.get_renderer( );

		rectangle border{ { 5, 5 }, { 40, 20 } };
		rectangle rect{ { 10, 8 }, { 20, 15 } };
		line diag1{ { 5, 5 }, { 40, 20 } };
		line diag2{ { 5, 20 }, { 40, 5 } };
		line cross1{ { 25, 8 }, { 35, 15 } };
		line cross2{ { 35, 8 }, { 25, 15 } };

		while( true )
		{
			fps.compute( );
			rend.clear( );
			
			point limit = term.size( ) - 2;
			border.start = ( border.start + 1 ) % limit;
			border.end   = ( border.end   + 1 ) % limit;
			rect.start   = ( rect.start   + 1 ) % limit;
			rect.end     = ( rect.end     + 1 ) % limit;
			diag1.start  = ( diag1.start  + 1 ) % limit;
			diag1.end    = ( diag1.end    + 1 ) % limit;
			diag2.start  = ( diag2.start  + 1 ) % limit;
			diag2.end    = ( diag2.end    + 1 ) % limit;
			cross1.start = ( cross1.start + 1 ) % limit;
			cross1.end   = ( cross1.end   + 1 ) % limit;
			cross2.start = ( cross2.start + 1 ) % limit;
			cross2.end   = ( cross2.end   + 1 ) % limit;

			//	draw white border
			rend.set_color( 15 );
			rend.draw( border, false );

			//	draw red filled rectangle
			rend.set_color( 9 );
			rend.draw( rect, true );

			//	draw blue diagonal lines
			rend.set_color( 12 );
			rend.draw( diag1 );
			rend.draw( diag2 );

			//	draw green cross
			rend.set_color( 10 );
			rend.draw( cross1 );
			rend.draw( cross2 );

			rend.refresh( );
			term.refresh( );
		}
	}
	catch( ... )
	{
		return	EXIT_FAILURE;
	}

	return	EXIT_SUCCESS;
}}


