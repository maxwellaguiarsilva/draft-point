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
 * File:   include/sak/geometry/line_view.hpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2026-01-13 14:29:29
 */


#pragma once
#ifndef header_guard_107605823
#define header_guard_107605823


#include <ranges>
#include <iterator>
#include <sak/using.hpp>
#include <sak/geometry/point.hpp>
#include <sak/math/math.hpp>


namespace sak {


__using( ::std::
	,forward_iterator_tag
	,ptrdiff_t
	,default_sentinel
	,default_sentinel_t
)
__using( ::std::ranges::
	,view_interface
)
__using( ::sak::math::
	,abs
	,sign
	,max
	,bind_back
	,greater_equal
)


template< is_point t_point >
class line_view : public view_interface< line_view< t_point > >
{
public:
	using	value_type	=	t_point;

	constexpr line_view( t_point start, t_point end )
		:m_start( start )
		,m_difference( end - start )
		,m_walker_step( m_difference | abs | to_point )
		,m_step( m_difference | sign | to_point )
		,m_total( max( m_walker_step ) )
	{ }
	
	constexpr line_view( ) = default;

	struct iterator
	{
		using	iterator_category	=	forward_iterator_tag;
		using	value_type			=	t_point;
		using	difference_type		=	ptrdiff_t;

		constexpr iterator( ) = default;
		explicit constexpr iterator( const line_view& parent )
			:m_parent( &parent )
			,m_current( parent.m_start )
			,m_walker( parent.m_walker_step )
			,m_index( 0 )
		{ }

		constexpr auto operator * ( ) const noexcept -> value_type { return m_current; }
		
		constexpr auto operator ++ ( ) noexcept -> iterator&
		{
			const auto& parent = *m_parent;
			const t_point direction = m_walker | bind_back( greater_equal, parent.m_total ) | to_point;

			m_current += parent.m_step * direction;
			m_walker += parent.m_walker_step - direction * parent.m_total;
			++m_index;
			return	*this;
		}

		constexpr auto operator ++ ( int ) noexcept -> iterator
		{
			auto copy = *this;
			++*this;
			return	copy;
		}

		constexpr auto operator == ( default_sentinel_t ) const noexcept -> bool
		{
			return	m_index >= m_parent->m_total;
		}

		const line_view* m_parent{ nullptr };
		t_point m_current;
		t_point m_walker;
		typename t_point::value_type m_index{ 0 };
	};

	constexpr auto begin( ) const noexcept { return iterator( *this ); }
	constexpr auto end( ) const noexcept { return default_sentinel; }

private:
	t_point m_start;
	t_point m_difference;
	t_point m_walker_step;
	t_point m_step;
	typename t_point::value_type m_total;
};


template< is_point t_point >
struct __line_to_closure 
{ 
	t_point m_end; 
};


template< is_point t_point >
constexpr auto operator | ( t_point start, __line_to_closure< t_point > closure )
{
	return	line_view< t_point >( start, closure.m_end );
}


struct __line_to
{
	template< is_point t_point >
	constexpr auto operator ( ) ( t_point end ) const 
	{ 
		return	__line_to_closure< t_point >{ end }; 
	}
};
inline constexpr auto line_to = __line_to{ };


}


#endif


