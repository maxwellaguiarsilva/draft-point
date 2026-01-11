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
 * File:   game/fps.hpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-08-26 20:27
 */


#pragma once
#ifndef header_guard_602704350
#define header_guard_602704350


#include <sak/sak.hpp>
#include <chrono>
#include <thread>


namespace game {


using	time_point = ::std::chrono::high_resolution_clock::time_point;


class fps final
{
public:
	explicit fps( int limit = 60 );
	~fps( ) noexcept = default;

	delete_copy_move_ctc( fps );

	auto set_limit( int limit ) -> void;
	auto compute( ) -> int;

	bool enable	=	true;

private:
	int		m_limit;
	time_point	m_start_time;

};


} 


#endif


