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
 * File:   0003_generic_dispatcher.cpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-11-26 21:28:59
 */


#include <vector>
#include <algorithm>
#include <memory>
#include <functional>
#include <expected>
#include <exception>
#include <mutex>
#include <utility>
#include <sak/sak.hpp>


using	::std::vector;
using	::std::make_shared;
using	::std::shared_ptr;
using	::std::weak_ptr;
using	::std::erase_if;
using	::std::expected;
using	::std::unexpected;
using	::std::exception_ptr;
using	::std::current_exception;
using	::std::lock_guard;
using	::std::mutex;
using	::std::atomic;
using	::std::memory_order_acquire;
using	::std::memory_order_release;
using	::std::invoke;



template< typename t_listener >
class dispatcher final
{
public:

	dispatcher( ) = default;
	delete_copy_move_ctc( dispatcher );

	struct failed_info
	{
		weak_ptr< t_listener >  listener;
		exception_ptr           exception;
	};
	using	error	=	vector< failed_info >;
	using	result	=	expected< void, error >;
	using	list	=	vector< weak_ptr< t_listener > >;

    void operator +=( const shared_ptr<t_listener>& instance )
	{
		auto lock = lock_guard( m_mutex );
		m_list.emplace_back( instance );
	}

    template <typename t_method, typename... t_call_args>
    result operator () (
		 t_method member_function_pointer
		,t_call_args&&... arguments
	)
	{
		unsigned l_clear_count;
		list l_list;
		{
			auto lock	=	lock_guard( m_mutex );
			l_list		=	m_list;
			l_clear_count	=	m_clear_count.load( memory_order_acquire );
		};
        error   failed_list;
		bool flg_clear = false;
		for( const auto& current_listener : l_list )
			if( auto locked = current_listener.lock( ) )
				try {
					invoke( member_function_pointer, locked.get( ), arguments... );
				} catch( ... ) { failed_list.emplace_back( current_listener, current_exception( ) ); }
			else
				flg_clear = true;

		if( flg_clear )
			clear( l_clear_count );

        if( failed_list.empty( ) )
			return	{ };
		return	unexpected( failed_list );
    }

private:
    list	m_list;
	mutex	m_mutex;
	atomic<unsigned>	m_clear_count	=	0;

	void clear( unsigned l_clear_count )
	{
		auto lock = lock_guard( m_mutex );
		if(	l_clear_count != m_clear_count.load( memory_order_acquire ) )
			return;
		erase_if( m_list, [ ](const auto& ptr) { return ptr.expired( ); } );
		m_clear_count.fetch_add( 1, memory_order_release );
	}
};


//	aqui começa o código que serve apenas para demonstrar como usar

#include <print>
#include <string>
#include <format>

using	::std::string;
using	::std::print;
using	::std::println;
using	::std::format;
using	::std::exception;
using	::std::runtime_error;


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
    println( "error dispatching on_clicked: {} listeners failed", result.error( ).size( ) );
    for( const auto& failed : result.error( ) )
        if( auto locked = failed.listener.lock( ) )
            try { rethrow_exception( failed.exception ); } catch( const exception& error ) {
                println( "    -   {}" , error.what( ) );
            }
}

int main( const int argument_count, const char* argument_values[ ] )
{{
	const vector< string > arguments( argument_values, argument_values + argument_count );
	for( const auto& value : arguments )
		println( "{}", value );

    dispatcher<button_listener> notifier;
    
    auto normal = make_shared<button_logger>( );
    auto unsafe = make_shared<unsafe_logger>( );
	notifier += normal;
	notifier += unsafe;

    handle_result( notifier( &button_listener::on_clicked, "btn_start" ) );
    handle_result( notifier( &button_listener::on_hover, 100 ) );

    return	EXIT_SUCCESS;
}};


