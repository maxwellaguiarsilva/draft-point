//	
//	SPDX-FileCopyrightText: 2025 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	SPDX-License-Identifier: GPL-3.0-or-later
//	


#pragma once
#ifndef header_guard_578639205
#define header_guard_578639205


#include <game/player.hpp>
#include <game/fps.hpp>


namespace game {


using	::game::player;
using	::game::fps;
using	::std::shared_ptr;
using	::std::make_shared;


class game
{
public:
	explicit game( renderer& renderer );
	virtual ~game( ) noexcept = default;

	delete_copy_move_ctc( game );

	auto run( ) -> void;

private:
	struct renderer_listener final : public renderer::listener
	{
		explicit renderer_listener( const g2i::point& new_size );
		void resize( const g2i::point& new_size ) override;
		g2i::point start;
		g2i::point end;
		g2i::point size;
		g2i::point label_position;
	};

	renderer&	m_renderer;
	player		m_player;
	fps			m_fps;
	shared_ptr< renderer_listener > m_renderer_listener;

};


}


#endif


