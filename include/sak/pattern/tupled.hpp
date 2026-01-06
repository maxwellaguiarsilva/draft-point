/*
 * Copyright (C) 2025 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
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
 * File:   sak/pattern/tupled.hpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-12-23 19:05
 */


#pragma once
#ifndef header_guard_360999933
#define header_guard_360999933

#include <sak/sak.hpp>
#include <tuple>
#include <utility>
#include <concepts>
#include <type_traits>


namespace sak {
namespace pattern {

__using( ::std::, apply, is_class_v, is_function_v, remove_cvref_t, remove_pointer_t, tuple_size )


template< typename t_invocable >
concept invocable	=
		is_class_v< remove_cvref_t< t_invocable > >
	or	is_function_v< remove_pointer_t< remove_cvref_t< t_invocable > > >;
template< typename t_tuple >
concept is_tuple = requires { typename tuple_size< remove_cvref_t< t_tuple > >::type; };


template< invocable t_invocable >
struct __tupled_caller
{
    t_invocable m_function;
    template< is_tuple t_tuple >
    constexpr auto operator ( )( const t_tuple& args ) const { return apply( m_function, args ); }
};
struct __tupled
{
	template< invocable t_invocable >
    constexpr auto operator( )( const t_invocable& invocable ) const {
		return	__tupled_caller< t_invocable >{ invocable };
	}
};
inline constexpr auto tupled = __tupled{ };


} }


#endif


