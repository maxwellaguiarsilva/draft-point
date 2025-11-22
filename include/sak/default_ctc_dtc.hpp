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
 * File:   sak/default_ctc_dtc.hpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-11-16 13:56
 */



#pragma once
#ifndef header_guard_065083550
#define header_guard_065083550



#define use_default_copy_ctc( class_name ) \
	class_name( const class_name& ) = default; \
	class_name& operator=( const class_name& ) = default;
#define use_default_move_ctc( class_name ) \
	class_name( class_name&& ) = default; \
	class_name& operator=( class_name&& ) = default;
#define use_default_copy_move_ctc( class_name ) \
	use_default_copy_ctc( class_name ); \
	use_default_move_ctc( class_name );

#define use_non_default_copy_ctc( class_name ) \
	class_name( const class_name& ); \
	class_name& operator=( const class_name& );
#define use_non_default_move_ctc( class_name ) \
	class_name( class_name&& ); \
	class_name& operator=( class_name&& );
#define use_non_default_copy_move_ctc( class_name ) \
	use_non_default_copy_ctc( class_name ); \
	use_non_default_move_ctc( class_name );

#define delete_copy_ctc( class_name ) \
	class_name( const class_name& ) = delete; \
	class_name& operator=( const class_name& ) = delete;
#define delete_move_ctc( class_name ) \
	class_name( class_name&& ) = delete; \
	class_name& operator=( class_name&& ) = delete;
#define delete_copy_move_ctc( class_name ) \
	delete_copy_ctc( class_name ); \
	delete_move_ctc( class_name );

#define use_default_dtc( class_name ) \
	virtual ~class_name( ) = default;
#define use_non_default_dtc( class_name ) \
	virtual ~class_name( );




#endif


