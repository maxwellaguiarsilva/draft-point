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
#include <memory>
#include <expected>


using	::std::vector;
using	::std::string;
using	::std::println;
using	::std::print;
using	::std::shared_ptr;
using	::std::weak_ptr;
using	::std::remove_if;
using	::std::make_shared;


template< typename t_listener >
class dispatcher
{
public:
	using	dispatcher_error = ::std::vector< ::std::weak_ptr< t_listener > >;
	using	dispatcher_result = ::std::expected< void, dispatcher_error >;

    void operator +=( const shared_ptr<t_listener>& instance ) { list.emplace_back( instance ); }

    //	t_method_args para deduzir a assinatura do ponteiro de função
    //	t_call_args para deduzir os argumentos passados no broadcast
    template <typename... t_method_args, typename... t_call_args>
    dispatcher_result operator () (
		 void ( t_listener::*member_function_pointer )( t_method_args... )
		,t_call_args&&... arguments
	)
	{
        dispatcher_error   failed_list;
		list.erase(
			remove_if( list.begin( ), list.end( ), [&]( const auto& current_listener ) {
				try {
					if( auto strong_ptr = current_listener.lock( ) )
                        return	( strong_ptr.get( )->*member_function_pointer )( ::std::forward<t_call_args>( arguments )... ), false;
					return	true;
				} catch( ... ) {
					failed_list.emplace_back( current_listener );
				}
				return	true;
			} )
			,list.end( )
		);

        if( not failed_list.empty( ) )
            return	::std::unexpected( failed_list );
        return	{ };
    }

private:
    vector< weak_ptr< t_listener > > list;
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


int main( const int argument_count, const char* argument_values[ ] )
{{
	const vector< string > arguments( argument_values, argument_values + argument_count );
	for( const auto& value : arguments )
		println( "{}", value );

    auto logger = make_shared<button_logger>( );
    dispatcher<button_listener> notifier;
    
    notifier += logger;

    if( auto result = notifier( &button_listener::on_clicked, "btn_start" ); not result.has_value( ) )
        println( "Error dispatching on_clicked: {} listeners failed", result.error( ).size( ) );
    if( auto result = notifier( &button_listener::on_hover, 100 ); not result.has_value( ) )
        println( "Error dispatching on_hover: {} listeners failed", result.error( ).size( ) );

    return	EXIT_SUCCESS;
}};


