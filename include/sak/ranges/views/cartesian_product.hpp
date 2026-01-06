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
 * File:   sak/ranges/views/cartesian_product.hpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2026-01-05 20:53
 */


#pragma once
#ifndef header_guard_147258369
#define header_guard_147258369


#include <sak/sak.hpp>
#include <sak/using.hpp>
#include <ranges>
#include <utility>


namespace sak::ranges::views {


//    ------------------------------
__using( ::std::
	,make_pair
)
__using( ::std::ranges::
	,viewable_range
)
__using( ::std::views::
	,transform
	,join
)
//    ------------------------------


struct __cartesian_product
{
	template< viewable_range t_first_range, viewable_range t_second_range >
	constexpr auto operator ( ) ( t_first_range&& first_range, t_second_range&& second_range ) const
	{
		return	::std::forward< t_first_range >( first_range )
			bitor	transform( [ second_range = ::std::forward< t_second_range >( second_range ) ] ( auto const& first_element ) {
				return	second_range bitor transform( [ first_element ] ( auto const& second_element ) {
					return	make_pair( first_element, second_element );
				} );
			} )
			bitor	join;
	}
};


inline constexpr auto cartesian_product = __cartesian_product{ };


}


#endif


