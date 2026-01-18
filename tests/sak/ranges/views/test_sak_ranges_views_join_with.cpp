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
 * File:   tests/sak/ranges/views/test_sak_ranges_views_join_with.cpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2026-01-09 17:07:16
 */


#include <string>
#include <vector>
#include <sak/ensure.hpp>
#include <sak/using.hpp>
#include <sak/ranges/views/join_with.hpp>


auto main( ) -> int
{{
	__using( ::sak::, exit_success, ensure )
	__using( ::std::, string, vector )
	__using( ::sak::ranges::views::, join_with )

	{
		const vector< string > words = { "hello", "world", "from", "sak" };
		const string delimiter = " ";
		
		const auto joined = join_with( words, delimiter );
		
		ensure( joined == "hello world from sak", "joined string mismatch" );
	}

	{
		const vector< string > empty_vector = { };
		const string delimiter = "-";
		const auto empty_joined = join_with( empty_vector, delimiter );
		ensure( empty_joined == "", "empty join should return an empty string" );
	}

	{
		const vector< string > single_word = { "only" };
		const string delimiter = ",";
		const auto single_joined = join_with( single_word, delimiter );
		ensure( single_joined == "only", "single word join mismatch" );
	}

	return	exit_success;
}}


