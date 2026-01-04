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
 * File:   sak/ensure.hpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-12-28 14:15
 */


#pragma once
#ifndef header_guard_553966683
#define header_guard_553966683

#include <string>
#include <stdexcept>
#include <cstdlib>


namespace sak {


using	::std::string;
using	::std::runtime_error;


inline const int exit_success = EXIT_SUCCESS;
inline const int exit_failure = EXIT_FAILURE;


struct __ensure
{
	constexpr auto operator ( ) ( bool expression, const string& message ) const { if( not expression ) throw runtime_error( message ); }
};
inline constexpr auto ensure = __ensure{ };


} 


#endif


