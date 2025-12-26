/*
 * Copyright (C) 2025 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
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
 * File:   tests/adhoc/0005_perfect_forwarding/0005_perfect_forwarding.cpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-12-26 18:18
 */


//	--------------------------------------------------
#include <print>
#include <string>
#include <vector>
#include <cstdlib>

#include <utility>
#include <iostream>

//	--------------------------------------------------

auto main( const int argument_count, const char* argument_values[ ] ) -> int
{{
	using	::std::string;
	using	::std::vector;
	using	::std::println;

	const vector< string > arguments( argument_values, argument_values + argument_count );
	for( const auto& value : arguments )
		println( "{}", value );
	
	println( "Iniciando testes para: perfect_forwarding..." );

	auto target = []( int& ) { println( "Recebido: Lvalue reference" ); };
	auto target_rview = []( int&& ) { println( "Recebido: Rvalue reference" ); };

	// Para demonstrar perfect forwarding precisamos de um template ou auto&& em um contexto de dedução
	auto wrapper = [&]( auto&& argument ) {
		// Se passarmos para target diretamente, ele sempre verá como Lvalue (pois 'argument' tem nome)
		// Por isso usamos ::std::forward
		if constexpr ( requires { target( ::std::forward<decltype( argument )>( argument ) ); } )
			target( ::std::forward<decltype( argument )>( argument ) );
		else
			target_rview( ::std::forward<decltype( argument )>( argument ) );
	};

	int value = 10;
	println( "Passando lvalue:" );
	wrapper( value );

	println( "Passando rvalue:" );
	wrapper( 20 );

    return	EXIT_SUCCESS;
}};
