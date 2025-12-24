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

#include <sak/pattern/tupled.hpp>
//	--------------------------------------------------


inline constexpr auto less_equal = ::sak::pattern::tupled( ::std::less_equal{} );


auto main( const int argument_count, const char* argument_values[ ] ) -> int
{{
	using	::std::string;
	using	::std::vector;

	const vector< string > arguments( argument_values, argument_values + argument_count );
	for( const auto& value : arguments )
		println( "{}", value );
	
	using	::std::array;
	using	::std::println;
	using	::std::make_pair;
	using	::std::ranges::all_of;
	using	::std::views::zip;
	
	array left	=	{ 1, 2, 3, 4, 5 };
	array right	=	{ 2, 3, 4, 5, 6 };
	
	println( "left < right: {}", all_of( zip( left, right ), less_equal ) );
    println( "pair (1, 2) less_equal: {}", less_equal( make_pair( 1, 2 ) ) );
    println( "array [5, 5] less_equal: {}", less_equal( array{ 5, 5 } ) );
	
    return	EXIT_SUCCESS;
}};


