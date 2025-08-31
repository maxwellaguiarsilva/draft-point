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
 * File:   game/fps.cpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-08-26 20:15
 */



#include <game/fps.hpp>


namespace game {


using	::std::string;
using	::std::chrono::milliseconds;
using	::std::chrono::high_resolution_clock;
using	::std::chrono::duration_cast;
using	::std::this_thread::sleep_for;
using	::std::to_string;


fps::fps( )
	:m_start_time{ high_resolution_clock::now( ) }
{ }


fps::~fps( ) { }

void fps::set_limit( int limit )
{
	assert( ( between( limit, 1, 300 ) ), "the fps limit must be between 1 and 300!" );
	m_limit = limit;
}

void fps::enable_limit( bool flg_enable ) noexcept { m_enable_limit = flg_enable; }
bool fps::is_enable( ) const noexcept { return m_enable_limit; }

int fps::compute( )
{
	auto	end_time	=	high_resolution_clock::now( );
	auto	frame_time	=	duration_cast<milliseconds>( end_time - m_start_time );
	m_start_time		=	end_time;

	milliseconds target_frame_time = milliseconds{ 1000 } / ( m_enable_limit ? m_limit : 1000 );

	if( frame_time < target_frame_time )
		sleep_for( target_frame_time - frame_time );

	return 1000 / frame_time.count( );
}


} 


