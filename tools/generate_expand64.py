#!/usr/bin/python3

#   
#   SPDX-FileCopyrightText: 2026 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   SPDX-License-Identifier: GPL-3.0-or-later
#   


from pathlib import Path

from sak.common import ensure


marker = "//\t__expand64__"
level_count = 64
project_root = Path( __file__ ).resolve( ).parent.parent

family_list = [
    {
         "name": "using"
        ,"file_path": "include/sak/expand64/using_namespace.hpp"
        ,"name_params": "p, a"
        ,"name_body": "using\tp a;"
        ,"chain_params": "p, a, ..."
        ,"chain_call_args": "p, a"
        ,"next_args": "p, __VA_ARGS__"
        ,"public_params": "prefix, ..."
        ,"public_args": "prefix"
    }
    ,{
         "name": "using_inline"
        ,"file_path": "include/sak/expand64/using_inline.hpp"
        ,"name_params": "p, a"
        ,"name_body": "inline constexpr auto& a = p a;"
        ,"chain_params": "p, a, ..."
        ,"chain_call_args": "p, a"
        ,"next_args": "p, __VA_ARGS__"
        ,"public_params": "prefix, ..."
        ,"public_args": "prefix"
    }
    ,{
         "name": "using_static"
        ,"file_path": "include/sak/expand64/using_static.hpp"
        ,"name_params": "p, a"
        ,"name_body": "static constexpr auto& a = p a;"
        ,"chain_params": "p, a, ..."
        ,"chain_call_args": "p, a"
        ,"next_args": "p, __VA_ARGS__"
        ,"public_params": "prefix, ..."
        ,"public_args": "prefix"
    }
    ,{
         "name": "use_macro"
        ,"file_path": "include/sak/expand64/using_macro.hpp"
        ,"name_params": "a_macro, a_args"
        ,"name_body": "a_macro a_args"
        ,"chain_params": "a_macro, a_args, ..."
        ,"chain_call_args": "a_macro, a_args"
        ,"next_args": "a_macro, __VA_ARGS__"
        ,"public_params": "a_macro, ..."
        ,"public_args": "a_macro"
    }
]


def build_chain_macro( family: dict, index: int ) -> str:
    name = family[ "name" ]
    tail = "" if index == level_count else f"\t__VA_OPT__( __{name}_{index + 1}( {family[ 'next_args' ]} ) )"
    return  f"#define __{name}_{index}( {family[ 'chain_params' ]} )\t__{name}_name( {family[ 'chain_call_args' ]} ){tail}"


def build_expansion( family: dict ) -> str:
    name = family[ "name" ]
    line_list = [
         f"#define __{name}_name( {family[ 'name_params' ]} )\t{family[ 'name_body' ]}"
        ,""
        ,""
    ]
    line_list.extend( build_chain_macro( family, index ) for index in range( 1, level_count + 1 ) )
    line_list.extend( [
         ""
        ,""
        ,f"#define __{name}( {family[ 'public_params' ]} )\t__VA_OPT__( __{name}_1( {family[ 'public_args' ]}, __VA_ARGS__ ) )"
    ] )
    return  "\n".join( line_list )


def apply_family( family: dict ) -> str:
    file_path = project_root / family[ "file_path" ]
    ensure( file_path.exists( ), f"missing header file: {file_path}" )
    line_list = file_path.read_text( ).split( "\n" )
    ensure( marker in line_list, f"missing marker in {file_path}" )
    marker_index = line_list.index( marker )
    end_index = line_list.index( "#endif", marker_index )
    new_line_list = line_list[ : marker_index + 1 ] + build_expansion( family ).split( "\n" ) + [ "", "" ] + line_list[ end_index: ]
    file_path.write_text( "\n".join( new_line_list ) )
    return  str( file_path )


def main( ) -> None:
    for family in family_list:
        print( apply_family( family ) )


if __name__ == "__main__":
    main( )


