//	
//	Copyright (C) 2026 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	
//	This program is free software: you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//	
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//	
//	You should have received a copy of the GNU General Public License
//	along with this program.  If not, see <http://www.gnu.org/licenses/>.
//	
//	
//	File:   tests/adhoc/memory-management/0008_main_scope_destruction.cpp
//	Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	
//	Created on 2026-07-18 10:24:34
//	


#include <print>
#include <ranges>
#include <string>
#include <vector>
#include <exception>
#include <sak/ensure.hpp>
#include <sak/using.hpp>
#include <cstdlib>
#include <sys/wait.h>


__using( ::std::
	,string
	,vector
	,println
	,atexit
	,exit
)
__using( ::sak::, ensure )


//	the event log lets each simulation register construction, destruction and context shutdown in order
vector< string > event_log;

auto constructed( const string& name ) -> string { return "construct " + name; }
auto destroyed( const string& name ) -> string { return "destroy " + name; }

const string vertex_buffer_name = "vertex_buffer", outer_name = "outer_resource", inner_name = "inner_resource";
const string terminate_event = "terminate_context", return_event = "return_point";

//	mimics a gpu resource: the destructor is the cleanup call that needs a living context
struct tracer final
{
	const string m_name;
	explicit tracer( const string& name ) : m_name( name ) { event_log.push_back( constructed( m_name ) ); }
	~tracer( ) { event_log.push_back( destroyed( m_name ) ); }
};

//	the shape of the cherno bug: the buffer declared in the function scope dies after the terminate call
auto simulate_terminate_before_destructor( ) -> void
{
	tracer vertex_buffer( vertex_buffer_name );
	event_log.push_back( terminate_event );
}

//	the shape of the cherno fix: the inner scope destroys the buffer while the context is still alive
auto simulate_terminate_after_inner_scope( ) -> void
{
	{
		tracer vertex_buffer( vertex_buffer_name );
	}
	event_log.push_back( terminate_event );
}

//	the doubt of this test: a return inside the inner scope still unwinds every enclosing scope, inner first
auto simulate_return_inside_inner_scope( ) -> void
{
	tracer outer_resource( outer_name );
	{
		tracer inner_resource( inner_name );
		event_log.push_back( return_event );
		return;
	}
}

//	the alternative placement: destruction happens at the inner closing brace, before the outer scope statements
auto simulate_return_after_inner_scope( ) -> void
{
	{
		tracer vertex_buffer( vertex_buffer_name );
	}
	event_log.push_back( return_event );
	return;
}

auto expect_events( const string& scenario_name, const auto& simulation, const vector< string >& expected_events ) -> void
{
	event_log.clear( );
	simulation( );
	ensure( event_log == expected_events, scenario_name + ": unexpected event order" );
	println( "ok: {}", scenario_name );
}

//	distinctive statuses avoid confusion with generic success and failure codes
constexpr int probe_destroyed_status = 20, probe_leaked_status = 10;
const string return_mode = "return_mode", exit_mode = "exit_mode";

bool flag_probe_destroyed = false;

//	planted on the stack of main by the child process
struct probe final
{
	~probe( ) { flag_probe_destroyed = true; }
};

//	registered with atexit: it reports the fate of the probe as the exit status and halts further cleanup
auto report_probe_status( ) -> void { ::std::_Exit( flag_probe_destroyed ? probe_destroyed_status : probe_leaked_status ); }

auto child_status( const string& program_path, const string& mode ) -> int
{
	return	WEXITSTATUS( ::std::system( ( program_path + " " + mode ).c_str( ) ) );
}


auto main( const int argument_count, const char* argument_values[ ] ) -> int
{
	__using( ::sak::, exit_success, exit_failure, ensure )
	__using( ::std::views::, join_with )
	__using( ::std::ranges::, to )
	__using( ::std::, string, vector, println, exception )

	println( "{}", vector< string >( argument_values, argument_values + argument_count ) | join_with( '\n' ) | to< string >( ) );
	
	//	child mode: the parent reads the fate of the probe from the exit status
	if( argument_count > 1 )
	{
		atexit( report_probe_status );
		const probe main_stack_probe;
		if( string( argument_values[ 1 ] ) == exit_mode )
			exit( exit_success );
		return	exit_success;
	}

	try
	{
		println( "starting tests for: main_scope_destruction" );

		expect_events( "terminate before destructor reproduces the cherno bug"
			,simulate_terminate_before_destructor
			,{ constructed( vertex_buffer_name ), terminate_event, destroyed( vertex_buffer_name ) }
		);

		expect_events( "the inner scope destroys the buffer before terminate"
			,simulate_terminate_after_inner_scope
			,{ constructed( vertex_buffer_name ), destroyed( vertex_buffer_name ), terminate_event }
		);

		expect_events( "return inside the inner scope unwinds every scope, inner first"
			,simulate_return_inside_inner_scope
			,{
				 constructed( outer_name )
				,constructed( inner_name )
				,return_event
				,destroyed( inner_name )
				,destroyed( outer_name )
			}
		);

		expect_events( "return after the inner scope only changes the moment of destruction"
			,simulate_return_after_inner_scope
			,{ constructed( vertex_buffer_name ), destroyed( vertex_buffer_name ), return_event }
		);

		//	process level proof: a normal return destroys the stack of main, exit does not
		const string program_path( argument_values[ 0 ] );
		ensure( child_status( program_path, return_mode ) == probe_destroyed_status
			,"a normal return from main must run the destructors of its stack objects" );
		ensure( child_status( program_path, exit_mode ) == probe_leaked_status
			,"exit must skip the destructors of the stack objects of main" );

		println( "all tests for main_scope_destruction passed" );
	}
	catch( const exception& error )
	{
		println( "test failed: {}", error.what( ) );
		return	exit_failure;
	}

	return	exit_success;
}


