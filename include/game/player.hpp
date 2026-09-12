//	
//	SPDX-FileCopyrightText: 2026 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	SPDX-License-Identifier: GPL-3.0-or-later
//	


#pragma once
#ifndef header_guard_185813530
#define header_guard_185813530


#include <game/renderer.hpp>
#include <game/direction.hpp>


namespace game {


using	::game::direction;
using	enum	::game::direction;


class player
{
public:
	explicit player( const g2i::point& position ) noexcept;

	auto direction( ) const noexcept -> const ::game::direction&;
	auto direction( const ::game::direction& heading ) noexcept -> void;

	auto step_move( ) noexcept -> void;

	auto draw( renderer& renderer ) const noexcept -> void;
	
	g2i::point position;
private:
	::game::direction m_direction;
};


} 


#endif


