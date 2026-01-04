/*
 * Copyright ( C ) 2025 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * ( at your option ) any later version.
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
 * File:   tests/sak/pattern/test_sak_pattern_dispatcher.cpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 *
 * Created on 2025-12-28 18:25
 */


//	--------------------------------------------------
#include <print>
#include <string>
#include <vector>
#include <memory>
#include <format>
#include <exception>
#include <stdexcept>

#include <sak/ensure.hpp>
#include <sak/using.hpp>
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
	
	dispatcher< mock_listener > dispatcher_instance;
	auto listener_instance = make_shared< mock_listener >( );
	
	dispatcher_instance += listener_instance;
	auto result = dispatcher_instance( &mock_listener::on_event );
	
	ensure( result.has_value( ), "error: notification failed" );
	ensure( listener_instance->called, "error: listener was not called" );
	
	println( "   -> success: basic notification works" );
}


struct button_listener
{
	virtual ~button_listener( ) = default;
	virtual void on_clicked( const string& button_name ) const = 0;
};


struct button_logger final : public button_listener 
{
	void on_clicked( const string& button_name ) const override
	{
		println( "   -> button clicked: {}" , button_name );
	}
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
	
	dispatcher< button_listener > dispatcher_instance;
	
	auto normal_logger = make_shared< button_logger >( );
	auto unsafe_logger_instance = make_shared< unsafe_logger >( );
	
	dispatcher_instance += normal_logger;
	dispatcher_instance += unsafe_logger_instance;

	auto result = dispatcher_instance( &button_listener::on_clicked, "btn_test" );
	handle_result( result );
	ensure( not result.has_value( ), "error: should have failed for one listener" );
	ensure( result.error( ).size( ) == 1, "error: unexpected number of failures" );
	
	println( "   -> success: complex interface and error handling verified" );
}


void test_dispatcher_cleanup( )
{
	println( "running: test_dispatcher_cleanup" );
	
	dispatcher< mock_listener > dispatcher_instance;
	
	{
		auto temp_listener = make_shared< mock_listener >( );
		dispatcher_instance += temp_listener;
	}
	
	//	at this point, the weak_ptr inside dispatcher is expired
	//	the next call will trigger the cleanup mechanism
	auto result = dispatcher_instance( &mock_listener::on_event );
	
	ensure( result.has_value( ), "error: notification with expired listener failed" );
	
	println( "   -> success: cleanup system executed safely" );
}


}


auto main( const int argument_count, const char* argument_values[ ] ) -> int
{{
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

	const vector< string > arguments( argument_values, argument_values + argument_count );
	for( const auto& value : arguments )
		println( "{}" , value );
	
	try
	{
		println( "starting tests for: sak/pattern/dispatcher..." );

		test_dispatcher_basic_notification( );
		test_dispatcher_complex_and_errors( );
		test_dispatcher_cleanup( );

		println( "all tests for sak/pattern/dispatcher passed!" );
	}
	catch( const exception& error )
	{
		println( "test failed: {}", error.what( ) );
		return	exit_failure;
	}

	return	exit_success;
}}


