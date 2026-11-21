//	
//	SPDX-FileCopyrightText: 2026 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	SPDX-License-Identifier: GPL-3.0-or-later
//	


#include <game/game.hpp>
#include <tui/renderer.hpp>


auto main( const int /*argument_count*/, const char* /*argument_values*/[ ] ) -> int
{
	__using( ::std::, exception, println )
	__using( ::sak::, exit_success, exit_failure, ensure )

	try
	{
		::tui::terminal terminal;
		::tui::renderer renderer( terminal );
		::game::game game( renderer );
		game.run( );
	} catch( const exception& e )
	{
		println( stderr, "error: {}", e.what( ) );
		return	exit_failure;
	} catch( ... )
	{
		println( stderr, "error: an unknown exception occurred" );
		return	exit_failure;
	}

	return	exit_success;
}


