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
 * Created on 2025-12-21 16:12
 */


#pragma once
#ifndef header_guard_164897448
#define header_guard_164897448


#define __using_name( p, a )	using	p a;


#define __using_1( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_2( p, __VA_ARGS__ ) )
#define __using_2( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_3( p, __VA_ARGS__ ) )
#define __using_3( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_4( p, __VA_ARGS__ ) )
#define __using_4( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_5( p, __VA_ARGS__ ) )
#define __using_5( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_6( p, __VA_ARGS__ ) )
#define __using_6( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_7( p, __VA_ARGS__ ) )
#define __using_7( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_8( p, __VA_ARGS__ ) )
#define __using_8( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_9( p, __VA_ARGS__ ) )
#define __using_9( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_10( p, __VA_ARGS__ ) )
#define __using_10( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_11( p, __VA_ARGS__ ) )
#define __using_11( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_12( p, __VA_ARGS__ ) )
#define __using_12( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_13( p, __VA_ARGS__ ) )
#define __using_13( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_14( p, __VA_ARGS__ ) )
#define __using_14( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_15( p, __VA_ARGS__ ) )
#define __using_15( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_16( p, __VA_ARGS__ ) )
#define __using_16( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_17( p, __VA_ARGS__ ) )
#define __using_17( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_18( p, __VA_ARGS__ ) )
#define __using_18( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_19( p, __VA_ARGS__ ) )
#define __using_19( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_20( p, __VA_ARGS__ ) )
#define __using_20( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_21( p, __VA_ARGS__ ) )
#define __using_21( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_22( p, __VA_ARGS__ ) )
#define __using_22( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_23( p, __VA_ARGS__ ) )
#define __using_23( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_24( p, __VA_ARGS__ ) )
#define __using_24( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_25( p, __VA_ARGS__ ) )
#define __using_25( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_26( p, __VA_ARGS__ ) )
#define __using_26( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_27( p, __VA_ARGS__ ) )
#define __using_27( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_28( p, __VA_ARGS__ ) )
#define __using_28( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_29( p, __VA_ARGS__ ) )
#define __using_29( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_30( p, __VA_ARGS__ ) )
#define __using_30( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_31( p, __VA_ARGS__ ) )
#define __using_31( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_32( p, __VA_ARGS__ ) )
#define __using_32( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_33( p, __VA_ARGS__ ) )
#define __using_33( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_34( p, __VA_ARGS__ ) )
#define __using_34( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_35( p, __VA_ARGS__ ) )
#define __using_35( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_36( p, __VA_ARGS__ ) )
#define __using_36( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_37( p, __VA_ARGS__ ) )
#define __using_37( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_38( p, __VA_ARGS__ ) )
#define __using_38( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_39( p, __VA_ARGS__ ) )
#define __using_39( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_40( p, __VA_ARGS__ ) )
#define __using_40( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_41( p, __VA_ARGS__ ) )
#define __using_41( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_42( p, __VA_ARGS__ ) )
#define __using_42( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_43( p, __VA_ARGS__ ) )
#define __using_43( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_44( p, __VA_ARGS__ ) )
#define __using_44( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_45( p, __VA_ARGS__ ) )
#define __using_45( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_46( p, __VA_ARGS__ ) )
#define __using_46( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_47( p, __VA_ARGS__ ) )
#define __using_47( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_48( p, __VA_ARGS__ ) )
#define __using_48( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_49( p, __VA_ARGS__ ) )
#define __using_49( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_50( p, __VA_ARGS__ ) )
#define __using_50( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_51( p, __VA_ARGS__ ) )
#define __using_51( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_52( p, __VA_ARGS__ ) )
#define __using_52( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_53( p, __VA_ARGS__ ) )
#define __using_53( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_54( p, __VA_ARGS__ ) )
#define __using_54( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_55( p, __VA_ARGS__ ) )
#define __using_55( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_56( p, __VA_ARGS__ ) )
#define __using_56( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_57( p, __VA_ARGS__ ) )
#define __using_57( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_58( p, __VA_ARGS__ ) )
#define __using_58( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_59( p, __VA_ARGS__ ) )
#define __using_59( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_60( p, __VA_ARGS__ ) )
#define __using_60( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_61( p, __VA_ARGS__ ) )
#define __using_61( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_62( p, __VA_ARGS__ ) )
#define __using_62( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_63( p, __VA_ARGS__ ) )
#define __using_63( p, a, ... )	__using_name( p, a )	__VA_OPT__( __using_64( p, __VA_ARGS__ ) )
#define __using_64( p, a, ... )	__using_name( p, a )


#define __using( prefix, ... )	__VA_OPT__( __using_1( prefix, __VA_ARGS__ ) )


#endif


