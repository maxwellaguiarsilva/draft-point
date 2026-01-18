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
 * File:   include/sak/ranges/chunk.hpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2026-01-05 20:32:14
 */


#pragma once
#ifndef header_guard_555387710
#define header_guard_555387710


#include <sak/sak.hpp>
#include <ranges>
#include <algorithm>
#include <cstddef>


namespace sak::ranges {


template< ::std::ranges::viewable_range t_range >
auto chunk( t_range&& range, const ::std::size_t size )
{
	return	::std::views::iota( ::std::size_t( 0 ), ( ::std::ranges::size( range ) + size - 1 ) / size )
		|	::std::views::transform( [ &range, size ]( const ::std::size_t index ) {
			auto start	=	::std::ranges::begin( range ) + ( index * size );
			auto end	=	::std::ranges::begin( range ) + ::std::min( ( index + 1 ) * size, ::std::ranges::size( range ) );
			return	::std::ranges::subrange( start, end );
		} );
}


}


#endif


