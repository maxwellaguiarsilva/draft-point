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
 * MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/* 
 * File:   tests/sak/pattern/test_sak_pattern_dispatcher.cpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 *
 * Created on 2025-12-28 18:25
 */


//	--------------------------------------------------
#include <print>
#include <string>
#include <vector>
#include <cstdlib>
#include <memory>
#include <format>
#include <exception>
#include <stdexcept>

#include <sak/ensure.hpp>
#include <sak/pattern/dispatcher.hpp>
//	--------------------------------------------------


namespace {


__using( ::std::
		,make_shared
		,shared_ptr
		,string
		,println
		,format
		,exception
		,runtime_error
		,rethrow_exception
)
using	::sak::pattern::dispatcher;
using	::sak::ensure;


struct mock_listener
{
	bool called = false;
	void on_event( ) { called = true; }
};


void test_dispatcher_basic_notification( )
{
	println( "running: test_dispatcher_basic_notification" );
	
	dispatcher< mock_listener > l_dispatcher;
	auto l_listener = make_shared< mock_listener >( );
	
l_dispatcher += l_listener;
	auto l_result = l_dispatcher( &mock_listener::on_event );
	
	ensure( l_result.has_value( ), "error: notification failed" );
ensure( l_listener->called, "error: listener was not called" );
	
	println( "   -> success: basic notification works\n" );
}


struct button_listener
{
	virtual ~button_listener( ) = default;
	virtual void on_clicked( const string& button_name ) const = 0;
};


struct button_logger final : public button_listener 
{
	void on_clicked( const string& button_name ) const override { println( "   -> button clicked: {}" , button_name ); }
};


struct unsafe_logger final : public button_listener
{
	void on_clicked( const string& button_name ) const override 
	{
		throw runtime_error( format( "error on button clicked: {}" , button_name ) ); 
	}
};


void handle_result( const dispatcher< button_listener >::result& result )
{
	if( result.has_value( ) )
		return;
	
	println( "   ! error: {} listeners failed", result.error( ).size( ) );
	for( const auto& failed : result.error( ) )
		if( auto locked = failed.listener.lock( ) )
			try { rethrow_exception( failed.exception ); } catch( const exception& error ) {
				println( "     - caught: {}" , error.what( ) );
			}
}


void test_dispatcher_complex_and_errors( )
{
	println( "running: test_dispatcher_complex_and_errors" );
	
	dispatcher< button_listener > l_dispatcher;
	
	auto l_normal = make_shared< button_logger >( );
	auto l_unsafe = make_shared< unsafe_logger >( );
	
l_dispatcher += l_normal;
l_dispatcher += l_unsafe;

	auto l_result = l_dispatcher( &button_listener::on_clicked, "btn_test" );
handle_result( l_result );
		sak::ensure( not l_result.has_value( ), "error: should have failed for one listener" );
sak::ensure( l_result.error( ).size( ) == 1, "error: unexpected number of failures" );
	
	println( "   -> success: complex interface and error handling verified\n" );
}


void test_dispatcher_cleanup( )
{
	println( "running: test_dispatcher_cleanup" );
	
dispatcher< mock_listener > l_dispatcher;
	
	{
		auto l_temp_listener = make_shared< mock_listener >( );
		l_dispatcher += l_temp_listener;
	}
	
	// At this point, the weak_ptr inside dispatcher is expired.
	// The next call will trigger the cleanup mechanism.
	auto l_result = l_dispatcher( &mock_listener::on_event );
	
sak::ensure( l_result.has_value( ), "error: notification with expired listener failed" );
	
	println( "   -> success: cleanup system executed safely\n" );
}


}


auto main( const int argument_count, const char* argument_values[ ] ) -> int
{{
	using	::std::string;
	using	::std::vector;
	using	::std::println;

	const vector< string > arguments( argument_values, argument_values + argument_count );
	for( const auto& value : arguments )
		println( "{}" , value );
	
	println( "Starting tests for: sak/pattern/dispatcher..." );

	test_dispatcher_basic_notification( );
	test_dispatcher_complex_and_errors( );
	test_dispatcher_cleanup( );

	println( "All tests for sak/pattern/dispatcher passed!" );

    return	EXIT_SUCCESS;
}};