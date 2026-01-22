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
//	File:   include/sak/pattern/dispatcher.hpp
//	Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	
//	Created on 2025-12-23 19:05:44
//	

#pragma once
#ifndef header_guard_148476052
#define header_guard_148476052


#include <sak/sak.hpp>
#include <vector>
#include <expected>
#include <exception>
#include <atomic>
#include <mutex>
#include <functional>


namespace sak {
namespace pattern {


//    ------------------------------
__using( ::std::
	,vector
	,erase_if
	,make_shared
	,shared_ptr
	,weak_ptr
	,expected
	,unexpected
	,exception_ptr
	,current_exception
	,atomic
	,memory_order_acquire
	,memory_order_release
	,mutex
	,lock_guard
	,invoke
)
//    ------------------------------


template< typename t_listener >
class dispatcher final
{
public:
	
	dispatcher( ) = default;
	delete_copy_move_ctc( dispatcher );
	
	struct failed_info
	{
		weak_ptr< t_listener >	listener;
		exception_ptr			exception;
	};
	using	error	=	vector< failed_info >;
	using	result	=	expected< void, error >;
	using	list	=	vector< weak_ptr< t_listener > >;
	
	void operator +=( const shared_ptr< t_listener >& instance )
	{
		if( not instance )
			return;
		auto lock = lock_guard( m_mutex );
		m_list.emplace_back( instance );
	}
	
	template< typename t_method, typename... t_call_args >
	result operator ( ) (
		 t_method member_function_pointer
		,t_call_args&&... arguments
	)
	{
		unsigned clear_count;
		list listeners_list;
		{
			auto lock		=	lock_guard( m_mutex );
			listeners_list	=	m_list;
			clear_count		=	m_clear_count.load( memory_order_acquire );
		};
		
		error   failed_list;
		bool should_clear = false;
		for( const auto& current_listener : listeners_list )
			if( auto locked = current_listener.lock( ) )
				try {
					invoke( member_function_pointer, locked.get( ), arguments... );
				} catch( ... ) { failed_list.emplace_back( current_listener, current_exception( ) ); }
			else
				should_clear = true;
		
		if( should_clear )
			clear( clear_count );
		
		if( failed_list.empty( ) )
			return	{ };
		return	unexpected( failed_list );
	}
	
private:
	list	m_list;
	mutex	m_mutex;
	atomic< unsigned >	m_clear_count	=	0;
	
	void clear( unsigned clear_count )
	{
		auto lock = lock_guard( m_mutex );
		if( clear_count not_eq m_clear_count.load( memory_order_acquire ) )
			return;
		erase_if( m_list, [ ]( const auto& ptr ) { return ptr.expired( ); } );
		m_clear_count.fetch_add( 1, memory_order_release );
	}
};


} };


#endif


