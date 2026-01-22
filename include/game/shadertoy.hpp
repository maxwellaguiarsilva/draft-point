//	
//	Copyright (C) 2026 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	
//	This program is free software: you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//	
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//	
//	You should have received a copy of the GNU General Public License
//	along with this program.  If not, see <http://www.gnu.org/licenses/>.
//	
//	
//	File:   include/game/shadertoy.hpp
//	Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	
//	Created on 2026-01-12 16:03:05
//	

#pragma once
#ifndef header_guard_803317754
#define header_guard_803317754


#include <sak/sak.hpp>
#include <game/renderer.hpp>
#include <game/fps.hpp>
#include <functional>


namespace game {


using	::std::function;
using	::sak::byte;


class shadertoy
{
public:
	explicit shadertoy( renderer& renderer );
	virtual ~shadertoy( ) noexcept = default;

	delete_copy_move_ctc( shadertoy );

	auto run( const function< g3f::point( g3f::point ) >& shader ) -> void;

private:
	renderer&	m_renderer;
	fps			m_fps;
	bool		m_is_paused{ false };
	float		m_time{ 0.0f };

};


} 


#endif


