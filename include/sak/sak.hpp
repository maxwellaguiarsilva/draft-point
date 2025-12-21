
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
 * File:   sak/sak.hpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-08-05 17:28
 */



#pragma once
#ifndef header_guard_039286706
#define header_guard_039286706

//	sak	=	swiss_army_knife


#include <stdexcept>
#include <sak/default_ctc_dtc.hpp>
#include <sak/using.hpp>

using	::std::runtime_error;


#define assert( expression, message ) \
	if( not ( expression ) ) throw runtime_error( message );

#define between( value, left, right ) ( value >= left and value <= right )


namespace sak {

constexpr char delta_lower_case	=	( 'a' - 'A' );
inline char to_lower_case( char code ) noexcept { return ( between( code, 'A', 'Z' ) ? code + delta_lower_case : code ); };
inline char to_upper_case( char code ) noexcept { return ( between( code, 'a', 'z' ) ? code - delta_lower_case : code ); };


}

#endif



