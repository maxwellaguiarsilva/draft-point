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
 * File:   main.cpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-08-24 11:29
 */


#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <game/game.hpp>
#include <tui/terminal.hpp>
#include <tui/renderer.hpp>


auto main( int, char*[ ] ) -> int
{{
	using	::std::cerr;
	using	::std::endl;
	using	::std::exception;

	try
	{
		::tui::terminal terminal;
		::tui::renderer renderer( terminal );
		::game::game game( renderer );
		game.run( );
	} catch( const exception& e )
	{
		cerr << "error: " << e.what( ) << endl;
		return	EXIT_FAILURE;
	} catch( ... )
	{
		cerr << "error: an unknown exception occurred" << endl;
		return	EXIT_FAILURE;
	}

	return	EXIT_SUCCESS;
}}


