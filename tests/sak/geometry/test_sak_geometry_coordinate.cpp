/*
 * Copyright ( C ) 2025 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * ( at your option ) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/* 
 * File:   test_sak_geometry_coordinate.cpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-12-26 17:43:29
 */



//	--------------------------------------------------
#include <print>
#include <string>
#include <vector>
#include <sak/geometry/geometry.hpp>
//	--------------------------------------------------


auto main( const int argument_count, const char* argument_values[ ] ) -> int
{{
	using	::std::string;
	using	::std::vector;
	using	::std::println;

	const vector< string > arguments( argument_values, argument_values + argument_count );
	for( const auto& value : arguments )
		println( "{}", value );
	
	using	::sak::geometry::point;
	using	::sak::geometry::coordinate;

	println( "Iniciando testes da classe coordinate..." );

	// Teste 1: Construtor padrão e acesso
	{
		point p;
		if ( p.size( ) != 2 or p[ 0 ] != 0 or p[ 1 ] != 0 ) return EXIT_FAILURE;
		println( "Teste 1 (Construtor padrão) passou." );
	}

	// Teste 2: Construtor com parâmetros
	{
		point p( 10, 20 );
		if ( p[ 0 ] != 10 or p[ 1 ] != 20 ) return EXIT_FAILURE;
		println( "Teste 2 (Construtor com parâmetros) passou." );
	}

	// Teste 3: Operações aritméticas (+, -)
	{
		point p1( 1, 2 );
		point p2( 3, 4 );
		point p3 = p1 + p2;
		if ( p3[ 0 ] != 4 or p3[ 1 ] != 6 ) return EXIT_FAILURE;

		point p4 = p2 - p1;
		if ( p4[ 0 ] != 2 or p4[ 1 ] != 2 ) return EXIT_FAILURE;
		println( "Teste 3 (Adição e Subtração) passou." );
	}

	// Teste 4: Operações com escalares (*, /, %)
	{
		point p( 10, 20 );
		point p1 = p * 2;
		if ( p1[ 0 ] != 20 or p1[ 1 ] != 40 ) return EXIT_FAILURE;

		point p2 = p / 2;
		if ( p2[ 0 ] != 5 or p2[ 1 ] != 10 ) return EXIT_FAILURE;

		point p3 = p % 3;
		if ( p3[ 0 ] != 1 or p3[ 1 ] != 2 ) return EXIT_FAILURE;
		println( "Teste 4 (Operações com escalares) passou." );
	}

	// Teste 5: Atribuição composta
	{
		point p( 1, 1 );
		p += point( 2, 3 );
		if ( p[ 0 ] != 3 or p[ 1 ] != 4 ) return EXIT_FAILURE;

		p *= 2;
		if ( p[ 0 ] != 6 or p[ 1 ] != 8 ) return EXIT_FAILURE;
		println( "Teste 5 (Atribuição composta) passou." );
	}

	// Teste 6: encloses
	{
		point p1( 0, 0 );
		point p2( 5, 5 );
		point p3( 10, 10 );

		if ( not p1.encloses( p2 ) ) return EXIT_FAILURE;   // 0,0 <= 5,5
		if ( not p2.encloses( p3 ) ) return EXIT_FAILURE;   // 5,5 <= 10,10
		if ( p3.encloses( p2 ) ) return EXIT_FAILURE;       // 10,10 <= 5,5 is false
		println( "Teste 6 (encloses) passou." );
	}

	// Teste 7: get_length
	{
		point p( 3, 4 );
		if ( p.get_length( ) != 5 ) return EXIT_FAILURE;
		println( "Teste 7 (get_length - int) passou." );
	}

	// Teste 8: Float coordinate
	{
		using f_point = coordinate< float, 3 >;
		f_point fp( 1.0f, 2.0f, 2.0f );
		
		// Length of (1, 2, 2) is sqrt(1+4+4) = sqrt(9) = 3
		if ( fp.get_length( ) != 3.0f ) return EXIT_FAILURE;

		f_point fp2 = fp * 0.5f;
		if ( fp2[ 0 ] != 0.5f or fp2[ 1 ] != 1.0f or fp2[ 2 ] != 1.0f ) return EXIT_FAILURE;
		println( "Teste 8 (Float coordinate e get_length) passou." );
	}

	// Teste 9: Iteradores
	{
		point p( 10, 20 );
		if ( ::sak::math::sum( p ) != 30 ) return EXIT_FAILURE;
		println( "Teste 9 (Iteradores/Sum) passou." );
	}

	// Teste 10: Escalar à esquerda
	{
		point p( 1, 2 );
		point p1 = 10 + p;
		if ( p1[ 0 ] != 11 or p1[ 1 ] != 12 ) return EXIT_FAILURE;

		point p2 = 20 - p;
		if ( p2[ 0 ] != 19 or p2[ 1 ] != 18 ) return EXIT_FAILURE;
		println( "Teste 10 (Escalar à esquerda) passou." );
	}

	println( "Todos os testes da classe coordinate passaram com sucesso!" );

    return	EXIT_SUCCESS;
}};


