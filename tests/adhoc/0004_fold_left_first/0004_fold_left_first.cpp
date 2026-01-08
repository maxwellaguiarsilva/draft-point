/*
 * Copyright (C) 2026 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
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
 * File:   adhoc/0004_fold_left_first/0004_fold_left_first.cpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2026-01-04 14:28
 */


#include <algorithm>
#include <exception>
#include <functional>
#include <iterator>
#include <optional>
#include <print>
#include <ranges>
#include <string>
#include <utility>
#include <vector>
#include <sak/ensure.hpp>
#include <sak/using.hpp>


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

}

auto main( const int argument_count, const char* argument_values[ ] ) -> int
{{
	__using( ::sak::
		,exit_success
		,exit_failure
		,ensure
	)
	__using( ::std::
		,string
		,vector
		,println
		,exception
		,plus
	)

	const vector< string > arguments( argument_values, argument_values + argument_count );
	for( const auto& value : arguments )
		println( "{}", value );
	
	try
	{
		println( "starting tests for: fold_left_first..." );

		//	empty range
		{
			const vector< int > v;
			const auto result = fold_left_first( v, plus< int >( ) );
			ensure( not result.has_value( ), "empty range should return empty optional" );
		}

		//	single element
		{
			const vector< int > v = { 42 };
			const auto result = fold_left_first( v, plus< int >( ) );
			ensure( result.has_value( ), "single element range should return a value" );
			ensure( *result == 42, "single element range should return the element itself" );
		}

		//	multiple elements
		{
			const vector< int > v = { 1, 2, 3, 4, 5 };
			const auto result = fold_left_first( v, plus< int >( ) );
			ensure( result.has_value( ), "multiple elements range should return a value" );
			ensure( *result == 15, "sum of 1..5 should be 15" );
		}

		//	string concatenation
		{
			const vector< string > v = { "a", "b", "c" };
			const auto result = fold_left_first( v, plus< string >( ) );
			ensure( result.has_value( ), "string range should return a value" );
			ensure( *result == "abc", "concatenation of a, b, c should be abc" );
		}

		println( "all tests for fold_left_first passed!" );
	}
	catch( const exception& error )
	{
		println( "test failed: {}", error.what( ) );
		return	exit_failure;
	}

	return	exit_success;
}}


