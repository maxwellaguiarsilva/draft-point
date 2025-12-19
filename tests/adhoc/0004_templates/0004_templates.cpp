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



#include <print>
#include <string>
#include <tuple>
#include <array>
#include <ranges>
#include <utility>
#include <algorithm>
#include <functional>


using	::std::string;
using	::std::vector;
//	--------------------------------------------------
using	::std::apply;
using	::std::array;
using	::std::println;
using	::std::ranges::all_of;
using	::std::views::zip;


//	faça o concept invocable e também o que verifica se t_tuple é uma tupla

template< invocable t_invocable >
struct tupled
{
    t_invocable m_function;
    template< typename t_tuple >
    inline constexpr auto operator()( t_tuple&& args ) const { return apply( m_function, ::std::forward< t_tuple >( args ) ); }
};


template< typename func_type >
tupled( func_type ) -> tupled< func_type >;


inline constexpr auto less_equal = tupled{ ::std::less_equal{} };


auto main( const int argument_count, const char* argument_values[ ] ) -> int
{{
	const vector< string > arguments( argument_values, argument_values + argument_count );
	for( const auto& value : arguments )
		println( "{}", value );

	array left	=	{ 1, 2, 3, 4, 5 };
	array right	=	{ 2, 3, 4, 5, 6 };

	println( "left < right: {}", all_of( zip( left, right ), less_equal ) );

    return	EXIT_SUCCESS;
}};


