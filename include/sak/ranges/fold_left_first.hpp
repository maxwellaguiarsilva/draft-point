//	
//	Copyright (C) 2026 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
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
//	File:   include/sak/ranges/fold_left_first.hpp
//	Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	
//	Created on 2026-01-08 01:06:17
//


#pragma once
#ifndef header_guard_197609099
#define header_guard_197609099


#include <concepts>
#include <functional>
#include <iterator>
#include <optional>
#include <ranges>
#include <utility>
#include <sak/using.hpp>


namespace sak {
namespace ranges {


namespace {


__using( ::std::
	,assignable_from
	,constructible_from
	,convertible_to
	,copy_constructible
	,in_place
	,indirectly_readable
	,input_iterator
	,invoke
	,invoke_result_t
	,invocable
	,iter_reference_t
	,iter_value_t
	,move
	,optional
	,ref
	,sentinel_for
)
namespace ranges = ::std::ranges;


template< class t_function, class t_type, class t_iterator >
concept indirectly_binary_left_foldable	=
		copy_constructible< t_function >
	and	indirectly_readable< t_iterator >
	and	invocable< t_function&, t_type, iter_reference_t< t_iterator > >
	and	assignable_from< t_type&, invoke_result_t< t_function&, t_type, iter_reference_t< t_iterator > > >
	and	convertible_to< invoke_result_t< t_function&, t_type, iter_reference_t< t_iterator > >, t_type >;


}


struct __fold_left_first
{
	template<
		 input_iterator t_iterator
		,sentinel_for< t_iterator > t_sentinel
		,indirectly_binary_left_foldable< iter_value_t< t_iterator >, t_iterator > t_function
	>
	requires constructible_from< iter_value_t< t_iterator >, iter_reference_t< t_iterator > >
	constexpr auto operator ( ) ( t_iterator first, t_sentinel last, t_function function ) const
	{
		using	value_type = iter_value_t< t_iterator >;

		if( first == last )
			return	optional< value_type >( );
		
		optional< value_type > init( in_place, *first );

		for( ++first; first not_eq last; ++first )
			*init = invoke( function, ::std::move( *init ), *first );
		
		return	init;
	}

	template<
		 ranges::input_range t_range
		,indirectly_binary_left_foldable< ranges::range_value_t< t_range >, ranges::iterator_t< t_range > > t_function
	>
	requires constructible_from< ranges::range_value_t< t_range >, ranges::range_reference_t< t_range > >
	constexpr auto operator ( ) ( t_range&& range, t_function function ) const { return ( *this )( ranges::begin( range ), ranges::end( range ), ref( function ) ); }
};
inline constexpr auto fold_left_first = __fold_left_first{ };


} }


#endif


