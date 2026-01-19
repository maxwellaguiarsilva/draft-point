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
//	File:   tests/adhoc/0005_shadertoy_demo/0005_shadertoy_demo.cpp
//	Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	
//	Created on 2026-01-12 16:03:05
//	


#include <exception>
#include <sak/using.hpp>
#include <game/shadertoy.hpp>
#include <tui/terminal.hpp>
#include <tui/renderer.hpp>
#include <sak/math/math.hpp>
#include <cmath>
#include <iostream>


namespace {


__using( ::std::, sin, cos, exp, tanh, abs )
__using( ::sak::, g2f, g3f, to_point )
__using( ::sak::math::, sum )


using   vec2    =   g2f::point;
using   vec3    =   g3f::point;


//	GLSL-like helpers
constexpr auto dot( const auto& left, const auto& right ) noexcept { return sum( left * right ); }


}


auto main( int, char*[ ] ) -> int
{{
	__using( ::sak::, exit_success, exit_failure, g2f, g3f, to_point )
	__using( ::std::, exception, sin, cos, exp, tanh, abs, cerr, endl )
	
	try
	{
		::tui::terminal terminal;
		::tui::renderer renderer( terminal );
		::game::shadertoy shadertoy( renderer );

		const auto size = renderer.size( );
		const float aspect = static_cast< float >( size[ 0 ] ) / static_cast< float >( size[ 1 ] );

		shadertoy.run( [ aspect ]( vec3 input ) -> vec3
		{
			const float x = input[ 0 ];
			const float y = input[ 1 ];
			const float t = input[ 2 ] * 2.0f;

			//	vec2 p=(FC.xy*2.-r)/r.y
			vec2 p{ ( x * 2.0f - 1.0f ) * aspect, y * 2.0f - 1.0f };
			
			//	l+=abs(.7-dot(p,p))
			float l = abs( 0.7f - dot( p, p ) );
			
			//	v=p*(1.-(l))/.2
			vec2 v = p * ( 1.0f - l ) / 0.2f;
			
			//	o (vec3 for RGB)
			vec3 o{ 0.01f, 0.01f, 0.01f };

			//	for(float i;i++<8.;o+=(sin(v.xyyx)+1.)*abs(v.x-v.y)*.2)v+=cos(v.yx*i+vec2(0,i)+t)/i+.7;
			for( float i = 1.0f; i <= 8.0f; ++i )
			{
				v = v + ( vec2{ cos( v[ 1 ] * i + t ), cos( v[ 0 ] * i + i + t ) } / i + 0.7f );
				o = o + ( vec3{ sin( v[ 0 ] ), sin( v[ 1 ] ), sin( v[ 1 ] ) } + 1.0f ) * abs( v[ 0 ] - v[ 1 ] ) * 0.2f;
			}

			//	o=tanh(exp(p.y*vec4(1,-1,-2,0))*exp(-4.*l.x)/o);
			vec3 e = ( p[ 1 ] * vec3{ 1.0f, -1.0f, -2.0f } ) 
				| []( float val ){ return exp( val ); } 
				| to_point;
			
			e *= exp( -4.0f * l );
			
			return	( e / o ) | []( float val ){ return tanh( val ); } | to_point;
		} );
	}
	catch( const exception& error )
	{
		cerr << "error: " << error.what( ) << endl;
		return	exit_failure;
	}

	return	exit_success;
}}


