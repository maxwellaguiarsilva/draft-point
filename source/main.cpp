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
 * You should have received a copy of the GNU General License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/* 
 * File:   main.cpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-08-02 10:00
 */



#include <iostream>
#include <exception>
#include <game/game.hpp>


using	::std::exception;
using	::std::cerr;
using	::game::game;


int main( )
{{
	try
	{
		game game;
		game.run( );

	} catch( const exception& e )
	{
		cerr << "error: " << e.what( ) << "\n";
		return	1;
	}

	return	0;
}}
