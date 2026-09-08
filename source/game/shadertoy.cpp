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
//	File:   source/game/shadertoy.cpp
//	Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	
//	Created on 2026-01-12 16:03:05
//	


#include <game/shadertoy.hpp>
#include <tui/terminal.hpp>
#include <sak/string.hpp>
#include <sak/math/math.hpp>
#include <chrono>


namespace game {


using	::std::function;
using	::std::make_shared;
using	::sak::byte;
using	::sak::math::min;
using	::std::to_string;
using	::std::chrono::high_resolution_clock;
using	::std::chrono::duration;


namespace {

	//	maps rgb components in [ 0.0, 1.0 ] to an xterm color code ( byte )
	struct __to_xterm
	{
		constexpr auto operator ( ) ( const g3f::point& rgb ) const noexcept -> byte
		{
			const int r = static_cast< int >( rgb[ 0 ] * 5.0f );
			const int g = static_cast< int >( rgb[ 1 ] * 5.0f );
			const int b = static_cast< int >( rgb[ 2 ] * 5.0f );
			return	static_cast< byte >( 16 + 36 * r + 6 * g + b );
		}
	};
	inline constexpr auto to_xterm = __to_xterm{ };
}


shadertoy::renderer_listener::renderer_listener( const g2i::point& new_size )
{ resize( new_size ); }


void shadertoy::renderer_listener::resize( const g2i::point& new_size )
{
	const g2f::point screen_size{ g2i::width( new_size ), g2i::height( new_size ) };
	normalization_scale	=	2.0f / min( screen_size );
	half_screen			=	screen_size / 2.0f;
}


shadertoy::shadertoy( ::tui::renderer& renderer )
	:m_renderer{ renderer }
	,m_renderer_listener{ make_shared< renderer_listener >( m_renderer.size( ) ) }
{
	m_renderer += m_renderer_listener;
}


auto shadertoy::run( const function< void( char, float ) >& frame_callback, const function< g3f::point( g2f::point ) >& pixel_shader ) -> void
{
	auto last_time = high_resolution_clock::now( );

	while( true )
	{
		const auto current_time = high_resolution_clock::now( );
		const duration< float > delta = current_time - last_time;
		last_time = current_time;

		char code = ::tui::terminal::read_char( );
		if( code == 'q' or code == 'Q' or code == 27 )
			break;

		if( code == 'p' or code == 'P' )
			m_is_paused = not m_is_paused;

		if( not m_is_paused )
			m_time += delta.count( );

		frame_callback( code, m_time );

		const auto& listener			=	*m_renderer_listener;
		const auto& half_screen			=	listener.half_screen;
		const auto  normalization_scale	=	listener.normalization_scale;
		const g2f::point direction{ 1.0f, -1.0f };

		//	converts terminal pixels to the float domain of the shader
		auto to_terminal = [ &, pixel_shader, half_screen, normalization_scale, direction ]( const g2i::point& pixel ) -> byte
		{
			const g2f::point current{ g2i::left( pixel ), g2i::top( pixel ) };
			const g2f::point coord = ( current - half_screen ) * direction * normalization_scale;
			return	to_xterm( pixel_shader( coord ) );
		};
		m_renderer.fill_with( to_terminal );

		m_renderer.print( { 1, 1 }, " | fps: "
			+	to_string( m_fps.compute( ) )
			+	" | time: "
			+	to_string( m_time )
			+	( m_is_paused ? " [PAUSED] | " : " | " )
		);
		m_renderer.refresh( );
	}

	m_renderer.clear( );
	m_renderer.refresh( );
}


}


