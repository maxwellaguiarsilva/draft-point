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
#include <sak/math/math.hpp>


namespace game {


using	::sak::math::between;
using	::sak::ensure;
using	::std::chrono::microseconds;
using	::std::chrono::high_resolution_clock;
using	::std::chrono::duration_cast;
using	::std::this_thread::sleep_for;


fps::fps( int limit )
	:m_start_time{ high_resolution_clock::now( ) }
{
	set_limit( limit );
}


auto fps::set_limit( int limit ) -> void
{
	ensure( between( limit, 1, 300 ), "the fps limit must be between 1 and 300!" );
	m_limit = limit;
}

auto fps::compute( ) -> int
{
	//	the first frame count is inaccurate, but this avoids the overhead of a conditional check
	//	for a one-time correction, the fps stabilizes from the second frame onwards
	//	for this reason, calculating `frame_time` before `sleep_for` is not a mistake, it's the correct decision
	auto	end_time	=	high_resolution_clock::now( );
	auto	frame_time	=	duration_cast<microseconds>( end_time - m_start_time );
	m_start_time		=	end_time;

	microseconds target_frame_time = microseconds{ 1000000 } / ( enable ? m_limit : 1000000 );

	if( frame_time < target_frame_time )
		sleep_for( target_frame_time - frame_time );

	return ( frame_time.count( ) > 0 ) ? ( 1000000 / frame_time.count( ) ) : m_limit;
}


} 


