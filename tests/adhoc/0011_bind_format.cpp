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
//	File:   tests/adhoc/0011_bind_format.cpp
//	Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	
//	Created on 2026-09-03 08:40:38
//	


#include <print>
#include <format>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>
#include <exception>
#include <functional>
#include <sak/ensure.hpp>
#include <sak/format.hpp>
#include <sak/ranges/contains.hpp>
#include <sak/sak.hpp>
#include <sak/using.hpp>


auto main( const int argument_count, const char* argument_values[ ] ) -> int
{
	__using( ::sak::, exit_success, exit_failure, ensure, byte )
	__using( ::std::
		,bind_front
		,exception
		,format
		,make_format_args
		,println
		,string
		,string_view
		,vector
		,vformat
	)
	__using( ::sak::ranges::, contains )

	const vector< string > arguments( argument_values, argument_values + argument_count );

	if( contains( arguments, { "-h", "--help" } ) )
	{
		println( "this executable is a battery of tests about: bind_format" );
		return	exit_success;
	}
	try
	{
		println( "starting tests for: bind_format" );

		//	--------------------------------------------------
		//	1. fixing format string and leading color code:
		//	std::format has both char and wchar_t overloads, so explicit function pointer cast is required
		//	format<const int&, const int&> collapses args&& to const int&, accepting both bound lvalues and call rvalues
		//	the format string must be passed as std::format_string (its copy/move ctor is constexpr, not consteval)
		//	--------------------------------------------------
		using binary_formatter = string ( * )( std::format_string< const int&, const int& >, const int&, const int& );
		auto binary_pointer = static_cast< binary_formatter >( format< const int&, const int& > );
		auto bound_binary = bind_front( binary_pointer, std::format_string< const int&, const int& >{ "\033[{};5;{}m" }, 38 );
		const string binary_result = bound_binary( 12 );
		ensure( binary_result == "\033[38;5;12m", "bound_binary produced incorrect string" );
		println( "{}", binary_result );

		//	--------------------------------------------------
		//	2. fixing only the format string:
		//	--------------------------------------------------
		using unary_formatter = string ( * )( std::format_string< const int& >, const int& );
		auto unary_pointer = static_cast< unary_formatter >( format< const int& > );
		auto bound_unary = bind_front( unary_pointer, std::format_string< const int& >{ "\033[38;5;{}m" } );
		const string unary_result = bound_unary( 12 );
		ensure( unary_result == "\033[38;5;12m", "bound_unary produced incorrect string" );
		println( "{}", unary_result );

		//	--------------------------------------------------
		//	3. fixing format string and color code using byte:
		//	--------------------------------------------------
		using byte_formatter = string ( * )( std::format_string< const int&, const byte& >, const int&, const byte& );
		auto byte_pointer = static_cast< byte_formatter >( format< const int&, const byte& > );
		auto bound_byte = bind_front( byte_pointer, std::format_string< const int&, const byte& >{ "\033[{};5;{}m" }, 48 );
		const string byte_result = bound_byte( byte{ 7 } );
		ensure( byte_result == "\033[48;5;7m", "bound_byte produced incorrect string" );
		println( "{}", byte_result );

		//	--------------------------------------------------
		//	4. runtime formatting using vformat:
		//	vformat operates on string_view instead of format_string,
		//	allowing arbitrary runtime strings without consteval restrictions
		//	--------------------------------------------------
		auto dynamic_formatter = [ ]( const string_view format_pattern, const auto&... format_arguments ) {
			return	vformat( format_pattern, make_format_args( format_arguments... ) );
		};
		const string runtime_pattern = "\033[{};5;{}m";
		auto bound_dynamic = bind_front( dynamic_formatter, runtime_pattern, 38 );
		const string dynamic_result = bound_dynamic( 12 );
		ensure( dynamic_result == "\033[38;5;12m", "bound_dynamic produced incorrect string" );
		println( "{}", dynamic_result );

		//	--------------------------------------------------
		//	5. niebloid sak::format:
		//	encapsulates runtime dispatch via vformat, enabling seamless
		//	inline usage with bind_front without casts or boilerplate
		//	--------------------------------------------------
		const string sak_result = bind_front( ::sak::format, "\033[{};5;{}m", 38 )( 12 );
		ensure( sak_result == "\033[38;5;12m", "sak_result produced incorrect string" );
		println( "{}", sak_result );

		println( "all tests for bind_format passed" );
	}
	catch( const exception& error )
	{
		println( "test failed: {}", error.what( ) );
		return	exit_failure;
	}

	return	exit_success;
}


