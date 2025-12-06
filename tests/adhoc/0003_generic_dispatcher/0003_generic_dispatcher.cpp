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


#include <print>
#include <vector>
#include <algorithm>
#include <string>
#include <format>
#include <memory>
#include <functional>
#include <expected>
#include <exception>
#include <mutex>

using	::std::vector;
using	::std::string;
using	::std::format;
using	::std::println;
using	::std::print;
using	::std::shared_ptr;
using	::std::weak_ptr;
using	::std::remove_if;
using	::std::make_shared;
using	::std::expected;
using	::std::unexpected;
using	::std::exception;
using	::std::exception_ptr;
using	::std::runtime_error;
using	::std::current_exception;
using	::std::rethrow_exception;
using	::std::function;
using	::std::lock_guard;
using	::std::mutex;


template< typename t_listener >
class dispatcher
{
public:
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
		clear( );
	}

    //	t_method_args para deduzir a assinatura do ponteiro de função
    //	t_call_args para deduzir os argumentos passados no broadcast
    template <typename... t_method_args, typename... t_call_args>
    result operator () (
		 void ( t_listener::*member_function_pointer )( t_method_args... )
		,t_call_args&&... arguments
	)
	{
		list l_list;
		{
			auto lock = lock_guard( m_mutex );
			clear( );
			l_list = m_list;
		};
        error   failed_list;
		for( const auto& current_listener : l_list )
			try {
				if( auto locked = current_listener.lock( ) )
					( locked.get( )->*member_function_pointer )( arguments... );
			} catch( ... ) { failed_list.emplace_back( current_listener, current_exception( ) ); }

        if( failed_list.empty( ) )
			return	{ };
		return	unexpected( failed_list );
    }

private:
    list	m_list;
	mutex	m_mutex;

	void clear( )
	{
		m_list.erase(
			remove_if( m_list.begin() , m_list.end( ), [ ](const auto& ptr) { return ptr.expired(); } )
			,m_list.end( )
		);
	}
};


class button_listener
{
public:
    virtual ~button_listener( ) = default;
    virtual void on_clicked( const string& button_name ) = 0;
    virtual void on_hover( int duration ) = 0;
};


class button_logger final : public button_listener 
{
public:
    void on_clicked( const string& button_name ) override { println( "button clicked: {}", button_name ); }
    void on_hover( int duration ) override { println( "hover: {}", duration ); }
};

class unsafe_logger final : public button_listener
{
public:
    void on_clicked( const string& button_name ) override { throw runtime_error( format( "error on button clicked listener: {}", button_name ) ); }
    void on_hover( int duration ) override { throw runtime_error( format( "error on button hover listener: {}", duration ) ); }
};



using button_result = dispatcher<button_listener>::result;

void handle_result( const button_result& result ) {
    if( result.has_value( ) )
        return;
    println( "Error dispatching on_clicked: {} listeners failed", result.error( ).size( ) );
    for( const auto& failed : result.error( ) )
        if( auto locked = failed.listener.lock( ) )
            try { rethrow_exception( failed.exception ); } catch( const exception& error ) {
                println( "{}" , error.what( ) );
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


