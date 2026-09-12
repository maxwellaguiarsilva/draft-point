//	
//	SPDX-FileCopyrightText: 2026 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	SPDX-License-Identifier: GPL-3.0-or-later
//	


#include <meta>
#include <memory>
#include <print>
#include <string>
#include <vector>
#include <exception>
#include <sak/ensure.hpp>
#include <sak/meta/override.hpp>
#include <sak/ranges/contains.hpp>
#include <sak/using.hpp>
#include <sak/pattern/dispatcher.hpp>


class window_listener
{
public:
	virtual ~window_listener( ) = default;

	virtual void resize( int, int ) { }
	virtual void pixel_resize( int, int ) { }
	virtual void move( int, int ) { }
	virtual void show( ) { }
	virtual void hide( ) { }
	virtual void minimize( ) { }
	virtual void maximize( ) { }
	virtual void restore( ) { }
	virtual void close_requested( ) { }
	virtual void focus_gained( ) { }
	virtual void focus_lost( ) { }
};


class scene_controller final : public window_listener
{
public:
	void resize( int, int ) override { }
	void close_requested( ) override { }
	void focus_lost( ) override { }
};


class passive_listener final : public window_listener
{ };


class counting_listener final : public window_listener
{
public:
	int m_resize_count	=	0;
	int m_move_count	=	0;

	void resize( int, int ) override { ++m_resize_count; }
	void move( int, int ) override { ++m_move_count; }
};


auto main( const int argument_count, const char* argument_values[ ] ) -> int
{
	__using( ::sak::, exit_success, exit_failure, ensure )
	__using( ::sak::meta::, overridden_names, overridden_methods, is_overridden, method_pointer, dispatch_reflected )
	__using( ::sak::pattern::, dispatcher )
	__using( ::std::, make_shared, shared_ptr, string, vector, println, exception )
	__using( ::sak::ranges::, contains )

	const vector< string > arguments( argument_values, argument_values + argument_count );

	if( contains( arguments, { "-h", "--help" } ) )
	{
		println( "this executable is a battery of tests about: sak/meta" );
		return	exit_success;
	}
	try
	{
		println( "starting tests for: sak/meta" );

		constexpr auto names = overridden_names< window_listener, scene_controller >( );

		static_assert( overridden_methods< window_listener, scene_controller >( ).size( ) == 3 );
		static_assert( is_overridden< window_listener, scene_controller >( "resize" ) );
		static_assert( is_overridden< window_listener, scene_controller >( "close_requested" ) );
		static_assert( is_overridden< window_listener, scene_controller >( "focus_lost" ) );
		static_assert( not is_overridden< window_listener, scene_controller >( "move" ) );
		static_assert( not is_overridden< window_listener, scene_controller >( "show" ) );

		static_assert( overridden_methods< window_listener, passive_listener >( ).empty( ) );

		ensure( names.size( ) == 3, "expected three overridden methods" );
		for( const auto& name : names )
			println( "overridden: {}", name );

		static_assert( method_pointer< ^^window_listener::resize >( ) == &window_listener::resize );
		static_assert( is_overridden< window_listener, counting_listener, ^^window_listener::resize >( ) );
		static_assert( not is_overridden< window_listener, counting_listener, ^^window_listener::show >( ) );

		dispatcher< window_listener > window_dispatcher;
		auto counter = make_shared< counting_listener >( );
		window_dispatcher += counter;

		( void )dispatch_reflected< ^^window_listener::resize >( window_dispatcher, 10, 20 );
		( void )dispatch_reflected< ^^window_listener::move >( window_dispatcher, 5, 5 );

		ensure( counter->m_resize_count == 1, "resize should have been dispatched once" );
		ensure( counter->m_move_count == 1, "move should have been dispatched once" );
		println( "reflection to member pointer: resize={} move={}", counter->m_resize_count, counter->m_move_count );

		println( "all tests for sak/meta passed" );
	}
	catch( const exception& error )
	{
		println( "test failed: {}", error.what( ) );
		return	exit_failure;
	}

	return	exit_success;
}

