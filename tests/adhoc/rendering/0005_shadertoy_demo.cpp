//	
//	SPDX-FileCopyrightText: 2026 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	SPDX-License-Identifier: GPL-3.0-or-later
//	


#include <game/shadertoy.hpp>


namespace {


__using( ::sak::, g2f, g3f )
__using( ::sak::ranges::, count_to, to )
__using( ::sak::math::
	,dot
	,sine
	,cosine
	,absolute
	,exponential
	,hyperbolic_tangent
)
__using_static( g2f::, left, top )


using	vec2	=	g2f::point;
using	vec3	=	g3f::point;


//	fractal iteration count of the palette loop below
constexpr auto iteration_count = 8;
//	shader clock runs twice as fast as wall time
constexpr float time_scale = 2.0f;


}


auto main( const int /*argument_count*/, const char* /*argument_values*/[ ] ) -> int
{
	__using( ::sak::, exit_success, exit_failure )
	__using( ::sak::math::
		,dot
		,sine
		,cosine
		,absolute
		,exponential
		,hyperbolic_tangent
	)
	__using_constexpr( g2f::, left, top )
	__using( ::std::, exception, println )

	try
	{
		::tui::terminal terminal;
		::tui::renderer renderer( terminal );
		::game::shadertoy shadertoy( renderer );

		float elapsed = 0.0f;
		shadertoy.run(
			[ & ]( char, float time ) -> void
			{
				elapsed = time * time_scale;
			},
			[ & ]( vec2 input ) -> vec3
			{
				//	normalized screen position with flipped y
				vec2 normalized{ left( input ), -top( input ) };

				//	distance to the bright ring
				float ring = absolute( 0.7f - dot( normalized, normalized ) );

				//	seed of the fractal iteration
				vec2 traced = normalized * ( 1.0f - ring ) / 0.2f;

				//	accumulated color
				vec3 color{ 0.01f, 0.01f, 0.01f };
				for( auto iteration : count_to( iteration_count ) )
				{
					const float divisor = iteration + 1.0f;
					const float horizontal = left( traced );
					const float vertical = top( traced );
					const vec2 drift{ cosine( vertical * divisor + elapsed ), cosine( horizontal * divisor + divisor + elapsed ) };
					traced = traced + ( drift / divisor + 0.7f );
					const vec3 wave{ sine( horizontal ), sine( vertical ), sine( vertical ) };
					color = color + ( wave + 1.0f ) * absolute( horizontal - vertical ) * 0.2f;
				}

				//	vertical falloff through the exponential lift
				vec3 falloff = ( top( normalized ) * vec3{ 1.0f, -1.0f, -2.0f } ) | exponential | to;
				falloff *= exponential( -4.0f * ring );

				return	( falloff / color ) | hyperbolic_tangent | to;
			}
		);
	}
	catch( const exception& error )
	{
		println( stderr, "error: {}", error.what( ) );
		return	exit_failure;
	}
	catch( ... )
	{
		println( stderr, "error: an unknown exception occurred" );
		return	exit_failure;
	}

	return	exit_success;
}
