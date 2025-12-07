/*
 * Copyright (C) 2025 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/* 
 * File:   pattern/dispatcher.hpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-12-07 18:44
 */



#pragma once
#ifndef header_guard_148476052
#define header_guard_148476052

#include <algorithm>
#include <atomic>
#include <expected>
#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <sak/sak.hpp>
#include <utility>
#include <vector>


namespace pattern {


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


} 


#endif