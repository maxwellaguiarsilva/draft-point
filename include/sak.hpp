
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
 * File:   sak.hpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-08-05 17:28
 */



#pragma once
#ifndef hpp039286706
#define hpp039286706


#include <string>
#include <cassert>


using	::std::string;


#define set_copy_ctc_off( class_name ) \
	class_name( const class_name& ) = delete; \
	class_name& operator=( const class_name& ) = delete;
#define set_move_ctc_off( class_name ) \
	class_name( class_name&& ) = delete; \
	class_name& operator=( class_name&& ) = delete;

#define use_default_dtc( class_name ) \
	virtual ~class_name( ) = default;
#define use_virtual_dtc( class_name ) \
	virtual ~class_name( );



#endif



