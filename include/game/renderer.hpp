/*
 * Copyright (C) 2026 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
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
 * File:   game/renderer.hpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2026-01-10 15:44
 */


#pragma once
#ifndef header_guard_260786294
#define header_guard_260786294


#include <sak/sak.hpp>
#include <game/geometry.hpp>
#include <memory>


namespace game {


using	::std::shared_ptr;
using	::std::string;
using	::sak::byte;


class renderer
{
public:
	class listener
	{
	public:
		virtual ~listener( ) = default;
		virtual void on_resize( const point& new_size ) = 0;
	};

	virtual ~renderer( ) noexcept = default;

	virtual void clear( ) noexcept = 0;
	virtual void refresh( ) = 0;
	virtual void set_color( const byte color ) noexcept = 0;
	virtual void draw( const point& pixel ) noexcept = 0;
	virtual void draw( const line& segment ) noexcept = 0;
	virtual void draw( const rectangle& area, bool is_filled = true ) noexcept = 0;
	virtual void print( const point& position, const string& text ) noexcept = 0;

	virtual auto size( ) const noexcept -> point = 0;

	virtual void operator +=( const shared_ptr< listener >& subject ) = 0;
};


} 


#endif


