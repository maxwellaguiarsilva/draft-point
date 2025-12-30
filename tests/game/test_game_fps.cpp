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
 * File:   tests/game/test_game_fps.cpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-12-26 18:18
 */


//	--------------------------------------------------
#include <print>
#include <string>
#include <vector>
#include <cstdlib>

#include <game/fps.hpp>

//	--------------------------------------------------

auto main( const int argument_count, const char* argument_values[ ] ) -> int
{{
	using	::std::string;
	using	::std::vector;
	using	::std::println;

	const vector< string > arguments( argument_values, argument_values + argument_count );
	for( const auto& value : arguments )
		println( "{}", value );
	
	println( "Starting tests for: game/fps..." );

	using	::game::fps;
	fps monitor( 60 );

	// Test 1: Initial limit
	println( "Test 1: Verifying FPS stabilization..." );
	monitor.compute( ); // First call to reset m_start_time
	for( int i = 0; i < 5; ++i )
	{
		int current_fps = monitor.compute( );
		println( "Frame {}: {} FPS", i, current_fps );
	}

	// Test 2: Limit change
	println( "Test 2: Changing limit to 30 FPS..." );
	monitor.set_limit( 30 );
	monitor.compute( );
	for( int i = 0; i < 5; ++i )
	{
		int current_fps = monitor.compute( );
		println( "Frame {}: {} FPS", i, current_fps );
	}

    return	EXIT_SUCCESS;
}};


