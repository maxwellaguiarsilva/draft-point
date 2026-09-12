//	
//	SPDX-FileCopyrightText: 2026 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	SPDX-License-Identifier: GPL-3.0-or-later
//	


#include <game/fps.hpp>
#include <sak/math/math.hpp>
#include <sak/using.hpp>


namespace game {


using	::sak::math::between;
using	::sak::ensure;
__using( ::std::chrono::
	,microseconds
	,high_resolution_clock
	,duration_cast
)
using	::std::this_thread::sleep_for;


fps::fps( int limit )
	:m_start_time{ high_resolution_clock::now( ) }
{
	this->limit( limit );
}


auto fps::limit( int limit ) -> void
{
	ensure( between( limit, 1, 300 ), "the fps limit must be between 1 and 300!" );
	m_limit = limit;
}

auto fps::compute( ) -> int
{
	auto	end_time	=	high_resolution_clock::now( );
	auto	frame_time	=	duration_cast<microseconds>( end_time - m_start_time );

	const auto target_frame_time = microseconds{ 1000000 } / ( enable ? m_limit : 1000000 );

	if( frame_time < target_frame_time )
	{
		sleep_for( target_frame_time - frame_time );
		frame_time	=	target_frame_time;
	}

	m_start_time		=	high_resolution_clock::now( );

	return	( frame_time.count( ) > 0 ) ? ( 1000000 / frame_time.count( ) ) : m_limit;
}


}


