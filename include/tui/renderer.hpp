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
//	File:   include/tui/renderer.hpp
//	Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	
//	Created on 2026-01-03 15:28:53
//	


#pragma once
#ifndef header_guard_794521060
#define header_guard_794521060


#include <tui/terminal.hpp>
#include <game/renderer.hpp>


namespace tui {


__using( ::std::
	,function
	,lock_guard
	,mutex
	,shared_ptr
	,size_t
	,string
	,vector
)
using	::sak::pattern::dispatcher;
using	::sak::byte;


class renderer final : public ::game::renderer
{
public:
	using	buffer	=	vector< byte >;
	using	listener =	::game::renderer::listener;

	explicit renderer( terminal& parent );
	~renderer( ) noexcept override = default;

	void clear( const byte value = 0 ) noexcept override;
	void refresh( ) override;
	void color( const byte value ) noexcept override;
	void draw( const geometry::position& point ) noexcept override;
	void draw( const geometry::line& line ) noexcept override;
	void draw( const geometry::rectangle& rectangle, bool is_filled = true ) noexcept override;
	void print( const geometry::position& position, const string& text ) noexcept override;
	void fill_with( const function< byte( geometry::position ) >& shader ) noexcept override;
	auto size( ) const noexcept -> geometry::size override;

	void operator +=( const shared_ptr< listener >& subject ) override;

private:
	struct terminal_listener;

	void resize( const geometry::size& size );
	void plot_unsafe( const geometry::position& point ) noexcept;

	terminal&	m_terminal;
	buffer		m_copy;
	buffer		m_main;
	geometry::size	m_terminal_size;
	geometry::size	m_margin;
	geometry::size	m_screen_size;
	byte		m_color;
	mutable mutex	m_mutex;
	shared_ptr< terminal_listener > m_terminal_listener;
	dispatcher< listener >	m_dispatcher;
};


}


#endif


