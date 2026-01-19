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
//	File:   include/tui/color.hpp
//	Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	
//	Created on 2026-01-12 16:03:05
//


#pragma once
#ifndef header_guard_682947105
#define header_guard_682947105


#include <sak/sak.hpp>
#include <tui/geometry.hpp>


namespace tui {
namespace color {


using	::sak::byte;


struct __to_xterm
{
	constexpr auto operator ( ) ( const g3f::point& rgb ) const noexcept -> byte
	{
		//	rgb components are expected to be in [ 0.0, 1.0 ]
		const int r = static_cast< int >( rgb[ 0 ] * 5.0f );
		const int g = static_cast< int >( rgb[ 1 ] * 5.0f );
		const int b = static_cast< int >( rgb[ 2 ] * 5.0f );

		return	static_cast< byte >( 16 + 36 * r + 6 * g + b );
	}
};
inline constexpr auto to_xterm = __to_xterm{ };


} }


#endif


