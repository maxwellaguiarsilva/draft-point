//	
//	SPDX-FileCopyrightText: 2026 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	SPDX-License-Identifier: GPL-3.0-or-later
//	


#pragma once
#ifndef header_guard_803317754
#define header_guard_803317754


#include <sak/sak.hpp>
#include <tui/renderer.hpp>
#include <game/fps.hpp>
#include <functional>


namespace game {


using	::std::function;
using	::std::shared_ptr;
using	::sak::byte;


class shadertoy
{
public:
	explicit shadertoy( ::tui::renderer& renderer );
	virtual ~shadertoy( ) noexcept = default;

	delete_copy_move_ctc( shadertoy );

	auto run( const function< void( char, float ) >& frame_callback, const function< g3f::point( g2f::point ) >& pixel_shader ) -> void;

private:
	struct renderer_listener final : public renderer::listener
	{
		explicit renderer_listener( const g2i::point& new_size );
		void resize( const g2i::point& new_size ) override;
		g2f::point	half_screen;
		float		normalization_scale{ 1.0f };
	};

	::tui::renderer&	m_renderer;
	fps		m_fps;
	bool		m_is_paused{ false };
	float		m_time{ 0.0f };
	shared_ptr< renderer_listener >	m_renderer_listener;

};


} 


#endif


