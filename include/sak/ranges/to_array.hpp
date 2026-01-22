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
//	File:   include/sak/ranges/to_array.hpp
//	Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	
//	Created on 2026-01-10 17:28:10
//	

#pragma once
#ifndef header_guard_578822352
#define header_guard_578822352


#include <sak/sak.hpp>
#include <array>
#include <ranges>
#include <algorithm>
#include <utility>
#include <cstddef>


namespace sak::ranges {


__using( ::std::
	,array
	,size_t
	,forward
)
__using( ::std::ranges::
	,input_range
	,range_value_t
	,copy
)


template< input_range t_range >
struct __array_from
{
	t_range m_range;

	template< size_t t_size >
	constexpr operator array< range_value_t< t_range >, t_size >( ) &&
	{
		array< range_value_t< t_range >, t_size > result;
		copy( m_range, result.begin( ) );
		return	result;
	}
};


struct __to_array { };
inline constexpr __to_array to_array{ };


template< input_range t_range >
constexpr auto operator | ( t_range&& subject, __to_array )
{
	return	__array_from< t_range >{ ::std::forward< t_range >( subject ) };
}


}


#endif


