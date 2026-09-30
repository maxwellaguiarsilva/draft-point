//	
//	SPDX-FileCopyrightText: 2026 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
//	SPDX-License-Identifier: GPL-3.0-or-later
//	


#include <vector>
#include <exception>
#include <sak/ranges/contains.hpp>
#include <sak/fso/text_file.hpp>


auto main( const int argument_count, const char* argument_values[ ] ) -> int
{
	__using( ::sak::, exit_success, exit_failure, ensure )
	__using( ::std::, string, vector, println, exception )
	__using( ::sak::ranges::, contains )
	__using( ::sak::fso::, file, text_file )
	__using( ::std::filesystem::, remove_all, temp_directory_path )

	const vector< string > arguments( argument_values, argument_values + argument_count );

	if( contains( arguments, { "-h", "--help" } ) )
	{
		println( "this executable is a battery of tests about: sak/fso" );
		return	exit_success;
	}
	try
	{
		println( "starting tests for: sak/fso" );

		const auto root = temp_directory_path( ) / "sak_fso_test";
		remove_all( root );
		const auto sample_path = root / "nested" / "note.txt";

		text_file sample( sample_path );
		ensure( not sample.exists( ), "new text_file must not exist" );
		ensure( not sample.modified_at( ).has_value( ), "new text_file must have no modified_at" );
		ensure( not sample.created_at( ).has_value( ), "new text_file must have no created_at" );
		ensure( not sample.content( ).has_value( ), "new text_file must have no content" );

		const auto message = sample.write( "hello\n" );
		ensure( message == "created file: " + sample_path.string( ) + "\n", "write must report the created file" );
		ensure( sample.exists( ), "written text_file must exist" );
		ensure( sample.content( ).value( ) == "hello\n", "written content must round-trip" );
		ensure( sample.modified_at( ).has_value( ), "written text_file must have modified_at" );
		ensure( sample.created_at( ).has_value( ), "written text_file must have created_at" );

		ensure( sample.name( ) == "note", "name must not include the extension" );
		ensure( sample.extension( ) == "txt", "extension must not include the dot" );
		ensure( sample.folder( ) == "nested", "folder must be the parent folder name" );
		ensure( sample.base( ) == sample_path.parent_path( ), "base must be the parent path" );
		ensure( sample.path( ) == sample_path, "path must be preserved" );

		file plain( sample_path );
		ensure( plain.exists( ), "plain file must read the written path metadata" );
		ensure( plain.name( ) == sample.name( ), "plain and text file must agree on name" );

		file missing( root / "missing.bin" );
		ensure( not missing.exists( ), "missing file must not exist" );
		ensure( not missing.modified_at( ).has_value( ), "missing file must have no modified_at" );

		remove_all( root );

		println( "all tests for sak/fso passed" );
	}
	catch( const exception& error )
	{
		println( "test failed: {}", error.what( ) );
		return	exit_failure;
	}

	return	exit_success;
}


