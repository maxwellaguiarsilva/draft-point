//	
//	SPDX-FileCopyrightText: 2026 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	SPDX-License-Identifier: GPL-3.0-or-later
//	


#include <print>
#include <ranges>
#include <string>
#include <vector>
#include <memory>
#include <exception>
#include <sak/ensure.hpp>
#include <sak/using.hpp>


const auto red		=	"\033[41;5m";
const auto blue		=	"\033[44;5m";
const auto reset	=	"\033[0m";

class base
{
public:
	base( ) { ::std::println( "{}{}: ctc base{}", blue, static_cast< void* >( this ), reset ); }
	virtual ~base( ) { ::std::println( "{}{}: dtc base{}", red, static_cast< void* >( this ), reset ); }
};

class derived final : public base
{
public:
	derived( ) { ::std::println( "{}{}: ctc derived{}", blue, static_cast< void* >( this ), reset ); }
	~derived( ) override { ::std::println( "{}{}: dtc derived{}", red, static_cast< void* >( this ), reset ); }
};

class non_virtual_base
{
public:
	non_virtual_base( ) { ::std::println( "{}{}: ctc non_virtual_base{}", blue, static_cast< void* >( this ), reset ); }
	~non_virtual_base( ) { ::std::println( "{}{}: dtc non_virtual_base{}", red, static_cast< void* >( this ), reset ); }
};

class non_virtual_derived : public non_virtual_base
{
public:
	non_virtual_derived( ) { ::std::println( "{}{}: ctc non_virtual_derived{}", blue, static_cast< void* >( this ), reset ); }
	~non_virtual_derived( ) { ::std::println( "{}{}: dtc non_virtual_derived{}", red, static_cast< void* >( this ), reset ); }
};


auto main( const int argument_count, const char* argument_values[ ] ) -> int
{
	__using( ::sak::, exit_success, exit_failure, ensure )
	__using( ::std::
		,string
		,vector
		,println
		,exception
		,unique_ptr
		,make_unique
	)
	__using( ::std::views::, join_with )
	__using( ::std::ranges::, to )

	println( "{}", vector< string >( argument_values, argument_values + argument_count ) | join_with( '\n' ) | to< string >( ) );

	try
	{
		println( "starting tests for: vector_copy" );

		const string line( 50, '-' );
		println( "{}", line );
		{
			vector< unique_ptr< base > > list;
			list.emplace_back( make_unique< derived >( ) );
			list.emplace_back( make_unique< derived >( ) );
			list.pop_back( );
		}
		println( "{}", line );
		{
			vector< unique_ptr< non_virtual_base > > list;
			list.emplace_back( make_unique< non_virtual_derived >( ) );
			list.emplace_back( make_unique< non_virtual_derived >( ) );
			list.pop_back( );
		}
		println( "{}", line );

		println( "all tests for vector_copy passed" );
	}
	catch( const exception& error )
	{
		println( "test failed: {}", error.what( ) );
		return	exit_failure;
	}

	return	exit_success;
}


