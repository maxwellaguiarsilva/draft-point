//	
//	SPDX-FileCopyrightText: 2025 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	SPDX-License-Identifier: GPL-3.0-or-later
//	


#include <print>
#include <string>
#include <vector>
#include <exception>
#include <sak/ensure.hpp>
#include <sak/using.hpp>
#include <game/fps.hpp>


auto main( const int argument_count, const char* argument_values[ ] ) -> int
{
	__using( ::sak::
		,exit_success
		,exit_failure
		,ensure
	)
	__using( ::std::
		,string
		,vector
		,println
		,exception
	)
	using	::game::fps;

	const vector< string > arguments( argument_values, argument_values + argument_count );
	for( const auto& value : arguments )
		println( "{}", value );
	
	try
	{
		println( "starting tests for: game/fps" );

		fps monitor( 60 );

		//	initial limit
		println( "test 1: verifying fps stabilization" );
		monitor.compute( );	//	first call to reset m_start_time
		for( int i = 0; i < 20; ++i )
		{
			int current_fps = monitor.compute( );
			println( "frame {}: {} fps", i, current_fps );
		}

		//	limit change
		println( "test 2: changing limit to 30 fps" );
		monitor.limit( 30 );
		monitor.compute( );
		for( int i = 0; i < 20; ++i )
		{
			int current_fps = monitor.compute( );
			println( "frame {}: {} fps", i, current_fps );
		}

		println( "all tests for game/fps passed" );
	}
	catch( const exception& error )
	{
		println( "test failed: {}", error.what( ) );
		return	exit_failure;
	}

	return	exit_success;
}


