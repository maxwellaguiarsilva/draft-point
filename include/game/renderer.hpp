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
//	File:   include/game/renderer.hpp
//	Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	
//	Created on 2026-01-10 15:44:21
//	


#pragma once
#ifndef header_guard_260786294
#define header_guard_260786294


#include <game/geometry.hpp>


namespace game {


__using( ::std::
	,shared_ptr
	,string
	,function
)
using	::sak::byte;


class renderer
{
public:
	using	geometry	=	::sak::g2i;

	class listener
	{
	public:
		virtual ~listener( ) = default;
		virtual void resize( const geometry::size& new_size ) = 0;
	};

	virtual ~renderer( ) noexcept = default;

	virtual void clear( const byte value = 0 ) noexcept = 0;
	virtual void refresh( ) = 0;
	virtual void color( const byte value ) noexcept = 0;
	virtual void draw( const geometry::position& pixel ) noexcept = 0;
	virtual void draw( const geometry::line& segment ) noexcept = 0;
	virtual void draw( const geometry::rectangle& area, bool is_filled = true ) noexcept = 0;
	virtual void print( const geometry::position& position, const string& text ) noexcept = 0;
	virtual void fill_with( const function< byte( geometry::position ) >& shader ) noexcept = 0;

	virtual auto size( ) const noexcept -> geometry::size = 0;

	virtual void operator +=( const shared_ptr< listener >& subject ) = 0;
};


} 


#endif


