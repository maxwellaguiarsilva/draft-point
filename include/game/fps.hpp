//	
//	SPDX-FileCopyrightText: 2025 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	SPDX-License-Identifier: GPL-3.0-or-later
//	


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

	auto limit( int limit ) -> void;
	auto compute( ) -> int;

	bool enable	=	true;

private:
	int		m_limit;
	time_point	m_start_time;

};


} 


#endif


