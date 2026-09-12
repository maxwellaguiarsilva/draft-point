//	
//	SPDX-FileCopyrightText: 2026 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	SPDX-License-Identifier: GPL-3.0-or-later
//	


#include <meta>
#include <memory>
#include <print>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>
#include <exception>
#include <sak/ensure.hpp>
#include <sak/ranges/contains.hpp>
#include <sak/using.hpp>
#include <sak/pattern/dispatcher.hpp>


namespace
{


__using( ::std::, define_static_array, define_static_string, make_shared, shared_ptr, string_view, vector )
__using( ::std::meta::
	,access_context
	,has_identifier
	,identifier_of
	,info
	,is_function
	,is_override
	,is_special_member_function
	,is_virtual
	,members_of
)
__using( ::sak::pattern::, dispatcher )


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


template< typename t_base, typename t_derived >
consteval auto overridden_methods( )
{
	vector< info > result;
	for( auto method : members_of( ^^t_base, access_context::current( ) ) )
	{
		if( not is_function( method ) or is_special_member_function( method ) or not is_virtual( method ) )
			continue;
		for( auto candidate : members_of( ^^t_derived, access_context::current( ) ) )
		{
			if( not is_function( candidate ) or not is_override( candidate ) )
				continue;
			if( has_identifier( method ) and has_identifier( candidate ) and identifier_of( method ) == identifier_of( candidate ) )
			{
				result.push_back( method );
				break;
			}
		}
	}
	return	result;
}


template< typename t_base, typename t_derived >
consteval auto is_overridden( const string_view name ) -> bool
{
	for( auto method : overridden_methods< t_base, t_derived >( ) )
		if( identifier_of( method ) == name )
			return	true;
	return	false;
}


template< typename t_base, typename t_derived >
consteval auto overridden_names( )
{
	vector< const char* > result;
	for( auto method : overridden_methods< t_base, t_derived >( ) )
		result.push_back( define_static_string( identifier_of( method ) ) );
	return	define_static_array( result );
}


template< info t_method >
consteval auto method_pointer( )
{
	return	&[: t_method :];
}

 
template< typename t_base, typename t_derived, info t_method >
consteval auto is_overridden( )
{
	return	is_overridden< t_base, t_derived >( identifier_of( t_method ) );
}


template< info t_method, typename t_listener, typename... t_args >
auto dispatch_reflected( dispatcher< t_listener >& dispatcher_instance, t_args&&... arguments )
{
	return	dispatcher_instance( &[: t_method :], arguments... );
}


}


auto main( const int argument_count, const char* argument_values[ ] ) -> int
{
	__using( ::sak::, exit_success, exit_failure, ensure )
	__using( ::std::, make_shared, shared_ptr, string, vector, println, exception )
	__using( ::sak::ranges::, contains )

	const vector< string > arguments( argument_values, argument_values + argument_count );

	if( contains( arguments, { "-h", "--help" } ) )
	{
		println( "this executable is a battery of tests about: is_overrided" );
		return	exit_success;
	}
	try
	{
		println( "starting tests for: is_overrided" );

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

		println( "all tests for is_overrided passed" );
	}
	catch( const exception& error )
	{
		println( "test failed: {}", error.what( ) );
		return	exit_failure;
	}

	return	exit_success;
}


