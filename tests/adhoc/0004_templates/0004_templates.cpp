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
 * File:   0004_templates.cpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-12-19 17:19:06
 */



//	--------------------------------------------------
#include <print>
#include <string>
#include <vector>
#include <array>
#include <ranges>
#include <algorithm>
#include <functional>
#include <cstdlib>

#include <tuple>
#include <utility>
#include <concepts>
#include <type_traits>
//	--------------------------------------------------


using	::std::apply;
using	::std::is_class_v;
using	::std::is_function_v;
using	::std::remove_cvref_t;
using	::std::remove_pointer_t;
using	::std::tuple_size;


template< typename t_invocable >
concept invocable	=
		is_class_v< remove_cvref_t< t_invocable > >
	or	is_function_v< remove_pointer_t< remove_cvref_t< t_invocable > > >
;
template< typename t_tuple >
concept is_tuple = requires { typename tuple_size< remove_cvref_t< t_tuple > >::type; };


template< invocable t_invocable >
struct tupled
{
    t_invocable m_function;
    template< is_tuple t_tuple >
    inline constexpr auto operator()( t_tuple&& args ) const { return apply( m_function, ::std::forward< t_tuple >( args ) ); }
};


template< typename func_type >
tupled( func_type ) -> tupled< func_type >;


inline constexpr auto less_equal = tupled{ ::std::less_equal{} };


auto main( const int argument_count, const char* argument_values[ ] ) -> int
{{
	using	::std::string;
	using	::std::vector;
	using	::std::array;
	using	::std::println;
	using	::std::make_pair;
	using	::std::ranges::all_of;
	using	::std::views::zip;
	
	const vector< string > arguments( argument_values, argument_values + argument_count );
	for( const auto& value : arguments )
		println( "{}", value );
	
	array left	=	{ 1, 2, 3, 4, 5 };
	array right	=	{ 2, 3, 4, 5, 6 };
	
	println( "left < right: {}", all_of( zip( left, right ), less_equal ) );
    println( "pair (1, 2) less_equal: {}", less_equal( make_pair( 1, 2 ) ) );
    println( "array [5, 5] less_equal: {}", less_equal( array{ 5, 5 } ) );
	
    return	EXIT_SUCCESS;
}};


