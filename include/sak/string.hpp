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
 * File:   sak/string.hpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-12-23 21:31
 */



#pragma once
#ifndef header_guard_501228163
#define header_guard_501228163

#include <sak/sak.hpp>
#include <string>


namespace sak {


using	::std::string;
using	::sak::math::between;


constexpr char delta_lower_case	=	( 'a' - 'A' );
inline char to_lower_case( char code ) noexcept { return ( between( code, 'A', 'Z' ) ? code + delta_lower_case : code ); };
//	inline char to_upper_case( char code ) noexcept { return ( between( code, 'a', 'z' ) ? code - delta_lower_case : code ); };


}


#endif


