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
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/* 
 * File:   sak/pattern/value_or.hpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-12-27 10:30
 */


#pragma once
#ifndef header_guard_1735295400
#define header_guard_1735295400


namespace sak {
namespace pattern {


struct __value_or
{
	template< typename t_container >
	constexpr auto operator ( ) ( const t_container& container, const typename t_container::key_type& key, const typename t_container::mapped_type& default_value ) const noexcept -> const typename t_container::mapped_type&
	{
		const auto iterator = container.find( key );
		return iterator not_eq container.end( ) ? iterator->second : default_value;
	}
};
inline constexpr auto value_or = __value_or{ };


} }


#endif


