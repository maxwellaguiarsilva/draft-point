//	
//	SPDX-FileCopyrightText: 2025 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	SPDX-License-Identifier: GPL-3.0-or-later
//	


#pragma once
#ifndef header_guard_399468167
#define header_guard_399468167


#include <game/geometry.hpp>


namespace game {


using	::std::string;


enum class direction
{
	 up	=	0
	,down
	,left
	,right
	,up_left
	,up_right
	,down_left
	,down_right
	,none
};

using enum direction;

struct __use_direction
{
private:
	struct __direction { string name; g2i::point point; };

public:
	constexpr auto operator ( )( const direction& heading ) const noexcept -> const __direction&
	{
		static __direction	up_d			{	"up"			,{	0	,-1	}	};
		static __direction	down_d			{	"down"			,{	0	,1	}	};
		static __direction	left_d			{	"left"			,{	-1	,0	}	};
		static __direction	right_d			{	"right"			,{	1	,0	}	};
		static __direction	up_left_d		{	"up_left"		,{	-1	,-1	}	};
		static __direction	up_right_d		{	"up_right"		,{	1	,-1	}	};
		static __direction	down_left_d		{	"down_left"		,{	-1	,1	}	};
		static __direction	down_right_d	{	"down_right"	,{	1	,1	}	};
		static __direction	none_d			{	"none"			,{	0	,0	}	};

		switch( heading )
		{
			case	up:			return	up_d;
			case	down:		return	down_d;
			case	left:		return	left_d;
			case	right:		return	right_d;
			case	up_left:	return	up_left_d;
			case	up_right:	return	up_right_d;
			case	down_left:	return	down_left_d;
			case	down_right:	return	down_right_d;
			case	none:		[ [ fallthrough ] ];
			default:			return	none_d;
		}
	}

};
inline constexpr auto use_direction	=	__use_direction{ };


}


#endif


