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
 * File:   sak/ranges/views/join_with.hpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2026-01-09 17:04
 */


#pragma once
#ifndef header_guard_145342292
#define header_guard_145342292


#include <sak/sak.hpp>
#include <sak/using.hpp>
#include <sak/math/math.hpp>
#include <sak/ranges/fold_left_first.hpp>
#include <ranges>
#include <functional>


namespace sak::ranges::views {


//    ------------------------------
__using( ::sak::math::
	,plus
)
__using( ::sak::ranges::
	,fold_left_first
)
__using( ::std::
	,bind
)
__using( ::std::ranges::
	,range_value_t
)
__using( ::std::placeholders::
	,_1
	,_2
)
//    ------------------------------


struct __join_with
{
	template< ::std::ranges::input_range t_range, typename t_delimiter >
	constexpr auto operator ( ) ( t_range&& range, t_delimiter&& delimiter ) const
	{
		return	fold_left_first( range, bind( plus, bind( plus, _1, delimiter ), _2 ) ).value_or( range_value_t< t_range >{ } );
	}
};


inline constexpr auto join_with = __join_with{ };


}


#endif


