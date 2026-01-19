//	
//	Copyright (C) 2025 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
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
//	File:   tests/adhoc/0003_generic_dispatcher/0003_generic_dispatcher.cpp
//	Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	
//	Created on 2025-11-26 21:36:02
//


#include <print>
#include <string>
#include <vector>
#include <format>
#include <sak/ensure.hpp>
#include <sak/using.hpp>
#include <sak/ranges/views/join_with.hpp>
#include <sak/pattern/dispatcher.hpp>


__using( ::std::
	,string
	,vector
	,make_shared
	,print
	,println
	,format
	,exception
	,runtime_error
)
using	::sak::pattern::dispatcher;


class button_listener
{
public:
	virtual ~button_listener( ) = default;
	virtual void on_clicked( const string& button_name ) const = 0;
	virtual void on_hover( int duration ) const = 0;
};


class button_logger final : public button_listener 
{
public:
	void on_clicked( const string& button_name ) const override { println( "button clicked: {}", button_name ); }
	void on_hover( int duration ) const override { println( "hover: {}", duration ); }
};

class unsafe_logger final : public button_listener
{
public:
	void on_clicked( const string& button_name ) const override { throw runtime_error( format( "error on button clicked listener: {}", button_name ) ); }
	void on_hover( int duration ) const override { throw runtime_error( format( "error on button hover listener: {}", duration ) ); }
};


using	button_result	=	dispatcher<button_listener>::result;
using	::std::rethrow_exception;

void handle_result( const button_result& result ) {
	if( result.has_value( ) )
		return;
	println( "error: {} listeners failed", result.error( ).size( ) );
	for( const auto& failed : result.error( ) )
		if( auto locked = failed.listener.lock( ) )
			try { rethrow_exception( failed.exception ); } catch( const exception& error ) {
				println( "    -   {}", error.what( ) );
			}
}


auto main( const int argument_count, const char* argument_values[ ] ) -> int
{{
	__using( ::sak::
		,exit_success
		,exit_failure
		,ensure
	)
	__using( ::sak::ranges::views::, join_with )
	__using( ::std::
		,string
		,vector
		,println
		,exception
	)

	println( "{}", join_with( vector< string >( argument_values, argument_values + argument_count ), "\n" ) );

	dispatcher<button_listener> notifier;
	
	auto normal = make_shared<button_logger>( );
	auto unsafe = make_shared<unsafe_logger>( );
	notifier += normal;
	notifier += unsafe;

	handle_result( notifier( &button_listener::on_clicked, "button_start" ) );
	handle_result( notifier( &button_listener::on_hover, 100 ) );

	return	exit_success;
}}


