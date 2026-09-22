//	
//	SPDX-FileCopyrightText: 2026 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	SPDX-License-Identifier: GPL-3.0-or-later
//	


#include <format>
#include <map>
#include <unordered_map>
#include <sak/pattern/bitmask.hpp>
#include <sak/pattern/dispatcher.hpp>
#include <sak/pattern/to_number.hpp>
#include <sak/pattern/tupled.hpp>
#include <sak/pattern/value_or.hpp>


namespace {


enum class mock_flag : uint8_t
{
	 none	=	0
	,read	=	1 << 0
	,write	=	1 << 1
	,exec	=	1 << 2
};
using	enum	mock_flag;


enum class sample
{
	 first	=	0
	,second
	,third
};
using	enum	sample;


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
	virtual ~mock_listener( ) = default;
	virtual void event( ) = 0;
};


struct mock_observer final : public mock_listener
{
	bool called = false;
	void event( ) override { called = true; }
};


void test_dispatcher_basic_notification( )
{
	println( "running: test_dispatcher_basic_notification" );
	
	bool had_error = false;
	auto record_error = [ & ]( const dispatcher< mock_listener >::error& ) { had_error = true; };
	dispatcher< mock_listener > dispatcher_instance( record_error );
	auto listener_instance = make_shared< mock_observer >( );
	
	dispatcher_instance += listener_instance;
	dispatcher_instance.dispatch< ^^mock_listener::event >( );
	
	ensure( not had_error, "error: notification failed" );
	ensure( listener_instance->called, "error: listener was not called" );
	
	println( "   -> success: basic notification works" );
}


struct button_listener
{
	virtual ~button_listener( ) = default;
	virtual void clicked( const string& button_name ) const = 0;
};


struct button_logger final : public button_listener 
{
	void clicked( const string& button_name ) const override
	{
		println( "   -> button clicked: {}", button_name );
	}
};


struct unsafe_logger final : public button_listener
{
	void clicked( const string& button_name ) const override 
	{
		throw runtime_error( format( "error on button clicked: {}", button_name ) ); 
	}
};


void handle_error( const dispatcher< button_listener >::error& failed_list )
{
	println( "   ! error: {} listeners failed", failed_list.size( ) );
	for( const auto& failed : failed_list )
		if( auto locked = failed.listener.lock( ) )
			try { rethrow_exception( failed.exception ); } catch( const exception& error ) {
				println( "     - caught: {}", error.what( ) );
			}
}


void test_dispatcher_complex_and_errors( )
{
	println( "running: test_dispatcher_complex_and_errors" );
	
	dispatcher< button_listener >::error captured_errors;
	auto on_error = [ & ]( const dispatcher< button_listener >::error& failed_list )
		{
			captured_errors = failed_list;
			handle_error( failed_list );
		};
	dispatcher< button_listener > dispatcher_instance( on_error );
	
	auto normal_logger = make_shared< button_logger >( );
	auto unsafe_logger_instance = make_shared< unsafe_logger >( );
	
	dispatcher_instance += normal_logger;
	dispatcher_instance += unsafe_logger_instance;

	dispatcher_instance.dispatch< ^^button_listener::clicked >( "btn_test" );
	ensure( captured_errors.size( ) == 1, "error: unexpected number of failures" );
	
	println( "   -> success: complex interface and error handling verified" );
}


void test_dispatcher_cleanup( )
{
	println( "running: test_dispatcher_cleanup" );
	
	bool had_error = false;
	auto record_error = [ & ]( const dispatcher< mock_listener >::error& ) { had_error = true; };
	dispatcher< mock_listener > dispatcher_instance( record_error );
	
	{
		auto temp_listener = make_shared< mock_observer >( );
		dispatcher_instance += temp_listener;
	}
	
	//	at this point, the weak_ptr inside dispatcher is expired
	//	the next call will trigger the cleanup mechanism
	dispatcher_instance.dispatch< ^^mock_listener::event >( );
	
	ensure( not had_error, "error: notification with expired listener failed" );
	
	println( "   -> success: cleanup system executed safely" );
}


}


auto main( const int argument_count, const char* argument_values[ ] ) -> int
{
	__using( ::sak::
		,byte
		,exit_success
		,exit_failure
		,ensure
	)
	__using( ::std::
		,array
		,equal_to
		,exception
		,less_equal
		,make_pair
		,map
		,println
		,string
		,unordered_map
		,vector
	)
	__using( ::std::ranges::, all_of )
	__using( ::std::views::, zip )
	__using( ::sak::pattern::
		,bitmask
		,cast
		,operator|
		,to_number
		,tupled
		,value_or
	)

	const vector< string > arguments( argument_values, argument_values + argument_count );
	for( const auto& value : arguments )
		println( "{}", value );
	
	try
	{
		println( "starting tests for: sak/pattern" );

		//	--------------------------------------------------
		//	dispatcher
		//	--------------------------------------------------
		test_dispatcher_basic_notification( );
		test_dispatcher_complex_and_errors( );
		test_dispatcher_cleanup( );

		//	--------------------------------------------------
		//	to_number
		//	--------------------------------------------------
		ensure( to_number( string{ "42" }, 0 ) == 42, "valid integer should be parsed" );
		ensure( to_number( string{ "-7" }, 0 ) == -7, "negative integer should be parsed" );
		ensure( to_number( string{ "5" }, byte{ 8 } ) == 5, "type should be deduced from the default value" );
		ensure( to_number( string{ "3.5" }, 0.0 ) == 3.5, "floating point should be parsed" );

		ensure( to_number( string{ "" }, 7 ) == 7, "empty string should yield the default value" );
		ensure( to_number( string{ "abc" }, 0 ) == 0, "non-numeric string should yield the default value" );
		ensure( to_number( string{ "8abc" }, 0 ) == 0, "trailing garbage should fail full consumption" );
		ensure( to_number( string{ "99999999999999999999999999" }, 0 ) == 0, "out-of-range value should yield the default value" );

		//	--------------------------------------------------
		//	tupled
		//	--------------------------------------------------
		{
			const auto tupled_less_equal = tupled( less_equal{ } );

			{
				const array left	=	{ 1, 2, 3, 4, 5 };
				const array right	=	{ 2, 3, 4, 5, 6 };
				
				ensure( all_of( zip( left, right ), tupled_less_equal ), "all_of zip( left, right ) with tupled less_equal should be true" );
			}

			{
				ensure( tupled_less_equal( make_pair( 1, 2 ) ), "tupled less_equal( pair( 1, 2 ) ) should be true" );
				ensure( not tupled_less_equal( make_pair( 2, 1 ) ), "tupled less_equal( pair( 2, 1 ) ) should be false" );
			}

			{
				ensure( tupled_less_equal( array{ 5, 5 } ), "tupled less_equal( array{ 5, 5 } ) should be true" );
				ensure( not tupled_less_equal( array{ 6, 5 } ), "tupled less_equal( array{ 6, 5 } ) should be false" );
			}
		}

		//	--------------------------------------------------
		//	value_or
		//	--------------------------------------------------
		{
			map< string, float > fruits_price = { { "apple", 1.5f }, { "banana", 0.75f } };
			
			ensure( value_or( fruits_price, "apple", 0.0f ) == 1.5f, "value for 'apple' should be 1.5f" );
			ensure( value_or( fruits_price, "banana", 0.0f ) == 0.75f, "value for 'banana' should be 0.75f" );
			ensure( value_or( fruits_price, "orange", 0.0f ) == 0.0f, "value for 'orange' should be 0.0f" );
		}

		{
			unordered_map< int, string > id_to_name = { { 1, "Alice" }, { 2, "Bob" } };
			string unknown = "Unknown";

			ensure( value_or( id_to_name, 1, unknown ) == "Alice", "value for id 1 should be 'alice'" );
			ensure( value_or( id_to_name, 2, unknown ) == "Bob", "value for id 2 should be 'bob'" );
			ensure( value_or( id_to_name, 3, unknown ) == "Unknown", "value for id 3 should be 'unknown'" );
		}

		{
			const vector< string > values = { "program", "5", "12" };
			const string default_value = "8";

			ensure( value_or( values, 1uz, default_value ) == "5", "indexed value for 1 should be '5'" );
			ensure( value_or( values, 2uz, default_value ) == "12", "indexed value for 2 should be '12'" );
			ensure( value_or( values, 9uz, default_value ) == "8", "indexed value out of range should be '8'" );
		}

		{
			const array< int, 3 > values = { 10, 20, 30 };
			const int default_value = 0;

			ensure( value_or( values, 1uz, default_value ) == 20, "indexed value for 1 should be 20" );
			ensure( value_or( values, 5uz, default_value ) == 0, "indexed value out of range should be 0" );
		}

		//	--------------------------------------------------
		//	bitmask
		//	--------------------------------------------------
		{
			bitmask< mock_flag > mask;
			ensure( not mask.any( read, write, exec ), "default bitmask should have no flags set" );

			mask.use( read );
			ensure( mask.all( read ), "read flag should be set" );
			ensure( not mask.all( read, write ), "read and write should not both be set" );
			ensure( mask.any( read, write ), "at least read is set" );

			mask.use( write, exec );
			ensure( mask.all( read, write, exec ), "all flags should be set after variadic use" );

			mask.remove( write );
			ensure( not mask.all( write ), "write flag should be removed" );
			ensure( mask.all( read, exec ), "read and exec should remain set" );

			mask.toggle( write, read );
			ensure( mask.all( write ), "write flag should be toggled on" );
			ensure( not mask.all( read ), "read flag should be toggled off" );

			mask.use( { read } );
			ensure( mask.all( { read, write, exec } ), "all flags should be set via initializer_list" );

			mask.remove( { exec, read } );
			ensure( not mask.any( exec, read ), "exec and read should be removed via initializer_list" );
			ensure( mask.all( write ), "write flag should still be set" );

			mask.toggle( { write, read } );
			ensure( mask.all( read ), "read should be toggled on" );
			ensure( not mask.all( write ), "write should be toggled off" );

			mask.clear( );
			ensure( not mask.any( read, write, exec ), "clear should reset all flags" );

			bitmask< mock_flag > initialized_mask{ read, write };
			ensure( initialized_mask.all( read, write ), "variadic constructor should initialize flags" );
			ensure( not initialized_mask.all( exec ), "exec should not be initialized" );

			bitmask< mock_flag > braced_mask{ { exec, write } };
			ensure( braced_mask.all( exec, write ), "initializer_list constructor should initialize flags" );
			ensure( not braced_mask.all( read ), "read should not be initialized" );
		}

		//	--------------------------------------------------
		//	cast
		//	--------------------------------------------------
		{
			ensure( cast< int >( 3.14f ) == 3, "scalar cast from float to int should truncate" );
			ensure( cast< uint8_t >( write ) == 2, "scalar cast from enum to uint8_t should match underlying" );

			const array values = { 1.1f, 2.9f, 3.5f };
			const array expected = { 1, 2, 3 };
			ensure( all_of( zip( values | cast< int >, expected ), tupled( equal_to{ } ) ), "piped cast should convert range element-wise" );
		}

		//	--------------------------------------------------
		//	enum
		//	--------------------------------------------------
		{
			constexpr array< int, 3 > numbers = { 10, 20, 30 };
			static_assert( ( numbers | second ) == 20 );

			ensure( ( numbers | first ) == 10, "pipe enum index should yield the first value" );
			ensure( ( numbers | second ) == 20, "pipe enum index should yield the second value" );
			ensure( ( numbers | third ) == 30, "pipe enum index should yield the third value" );

			const array< string, 3 > names = { "first", "second", "third" };
			ensure( ( names | third ) == "third", "pipe enum index should work with strings" );
		}

		println( "all tests for sak/pattern passed" );
	}
	catch( const exception& error )
	{
		println( "test failed: {}", error.what( ) );
		return	exit_failure;
	}

	return	exit_success;
}


