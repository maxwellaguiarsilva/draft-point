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
 * File:   sak/using.hpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-12-21 15:31
 */



#pragma once
#ifndef header_guard_164897448
#define header_guard_164897448


#define __using_n( _1, _2, _3, _4, _5, _6, _7, _8, n, ... ) n


#define __using_1( p, a )	using	p a;
#define __using_2( p, a, b ) \
	__using_1( p, a ) \
	__using_1( p, b )
#define __using_3( p, a, b, c ) \
	__using_2( p, a, b ) \
	__using_1( p, c )
#define __using_4( p, a, b, c, d ) \
	__using_3( p, a, b, c ) \
	__using_1( p, d )
#define __using_5( p, a, b, c, d, e ) \
	__using_4( p, a, b, c, d ) \
	__using_1( p, e )
#define __using_6( p, a, b, c, d, e, f ) \
	__using_5( p, a, b, c, d, e ) \
	__using_1( p, f )
#define __using_7( p, a, b, c, d, e, f, g ) \
	__using_6( p, a, b, c, d, e, f ) \
	__using_1( p, g )
#define __using_8( p, a, b, c, d, e, f, g, h ) \
	__using_7( p, a, b, c, d, e, f, g ) \
	__using_1( p, h )


#define __using( prefix, ... ) \
	__using_n( __VA_ARGS__, __using_8, __using_7, __using_6, __using_5, __using_4, __using_3, __using_2, __using_1 )( prefix, __VA_ARGS__ )


#endif


