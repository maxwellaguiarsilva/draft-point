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
 * You should have received a copy of the GNU General License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/* 
 * File:   0001_vector_copy.cpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-11-20 09:04
 */



//	#include <sak/sak.hpp>
#include <string>
#include <print>
#include <vector>
#include <memory>
#include <cstdlib>


using	::std::print;
using	::std::println;
using	::std::string;
using	::std::vector;
using	::std::unique_ptr;
using	::std::make_unique;

const auto m_red	=	"\033[41;5m";
const auto m_blue	=	"\033[44;5m";
const auto m_reset	=	"\033[0m";

class base
{
public:
	base( ) { println( "{}{}: ctc base{}", m_blue, static_cast<void*>(this), m_reset ); }
	virtual ~base( ) { println( "{}{}: dtc base{}", m_red, static_cast<void*>(this), m_reset ); }
};
class derived final : public base
{
public:
	derived( ) { println( "{}{}: ctc derived{}", m_blue, static_cast<void*>(this), m_reset ); }
	~derived( ) override { println( "{}{}: dtc derived{}", m_red, static_cast<void*>(this), m_reset ); }
};


class non_virtual_base
{
public:
	non_virtual_base( ) { println( "{}{}: ctc non_virtual_base{}", m_blue, static_cast<void*>(this), m_reset ); }
	~non_virtual_base( ) { println( "{}{}: dtc non_virtual_base{}", m_red, static_cast<void*>(this), m_reset ); }
};
class non_virtual_derived : public non_virtual_base
{
public:
	non_virtual_derived( ) { println( "{}{}: ctc non_virtual_derived{}", m_blue, static_cast<void*>(this), m_reset ); }
	~non_virtual_derived( ) { println( "{}{}: dtc non_virtual_derived{}", m_red, static_cast<void*>(this), m_reset ); }
};


auto main( const int argument_count, const char* argument_values[ ] ) -> int
{{
	using	::std::string;
	using	::std::vector;

	const vector< string > arguments( argument_values, argument_values + argument_count );
	for( const auto& value : arguments )
		println( "{}", value );

	const string line( 50, '-' );
	println( "{}\n", line );
	{
		vector< unique_ptr< base > > list;
		list.emplace_back( make_unique< derived >( ) );
		list.emplace_back( make_unique< derived >( ) );
		list.pop_back( );
	}
	println( "{}\n", line );
	{
		vector< unique_ptr< non_virtual_base > > list;
		list.emplace_back( make_unique< non_virtual_derived >( ) );
		list.emplace_back( make_unique< non_virtual_derived >( ) );
		list.pop_back( );
	}
	println( "{}\n", line );
	return	EXIT_SUCCESS;
}};
