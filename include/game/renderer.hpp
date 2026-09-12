//	
//	SPDX-FileCopyrightText: 2026 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	SPDX-License-Identifier: GPL-3.0-or-later
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


