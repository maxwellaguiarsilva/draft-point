//	
//	SPDX-FileCopyrightText: 2026 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	SPDX-License-Identifier: GPL-3.0-or-later
//	


#include <format>
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
	virtual void clicked( const string& button_name ) const = 0;
	virtual void hover( int duration ) const = 0;
};


class button_logger final : public button_listener 
{
public:
	void clicked( const string& button_name ) const override { println( "button clicked: {}", button_name ); }
	void hover( int duration ) const override { println( "hover: {}", duration ); }
};

class unsafe_logger final : public button_listener
{
public:
	void clicked( const string& button_name ) const override { throw runtime_error( format( "error on button clicked listener: {}", button_name ) ); }
	void hover( int duration ) const override { throw runtime_error( format( "error on button hover listener: {}", duration ) ); }
};


using	button_error	=	dispatcher<button_listener>::error;
using	::std::rethrow_exception;

void handle_error( const button_error& failed_list )
{
	println( "error: {} listeners failed", failed_list.size( ) );
	for( const auto& failed : failed_list )
		if( auto locked = failed.listener.lock( ) )
			try { rethrow_exception( failed.exception ); } catch( const exception& error ) {
				println( "    -   {}", error.what( ) );
			}
}


auto main( const int argument_count, const char* argument_values[ ] ) -> int
{
	__using( ::sak::
		,exit_success
		,ensure
	)
	__using( ::std::
		,string
		,vector
		,println
		,exception
	)
	__using( ::std::views::, join_with )
	__using( ::std::ranges::, to )

	println( "{}", vector< string >( argument_values, argument_values + argument_count ) | join_with( '\n' ) | to< string >( ) );

	dispatcher<button_listener> notifier( handle_error );
	
	auto normal = make_shared<button_logger>( );
	auto unsafe = make_shared<unsafe_logger>( );
	notifier += normal;
	notifier += unsafe;

	notifier.dispatch< ^^button_listener::clicked >( "button_start" );
	notifier.dispatch< ^^button_listener::hover >( 100 );

	return	exit_success;
}


