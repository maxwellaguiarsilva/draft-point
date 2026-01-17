#!/usr/bin/python3


#   Copyright (C) 2025 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#   
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#   
#   You should have received a copy of the GNU General License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#   
#   
#   File:   tools/cpp/create_class.py
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-15 15:00:00


import time
from lib.common import run_mcp_tool, ensure, write_file
from lib import metadata_provider
from lib import template_engine
from cpp_lib.config import default_cpp_config
from cpp_lib.project_tree import parse_hierarchy


def generate_header_guard( ):
    return f"header_guard_{ str( time.time_ns( ) )[ -9: ] }"

def create_class( class_hierarchy, include_list=[ ], using_list=[ ], create_header_only=False ):
    message = ""
    hierarchy_list = parse_hierarchy( class_hierarchy )
    rel_path = "/".join( hierarchy_list )
    
    include_dir = default_cpp_config[ "paths" ][ "include" ]
    file_path = f"{include_dir}/{rel_path}.hpp"
    data = {
         "header_guard": generate_header_guard( )
        ,"class_name": hierarchy_list[ -1 ]
        ,"include_list": include_list
        ,"namespace_list": hierarchy_list[ :-1 ]
        ,"using_list": using_list
        ,"des_file_path": f"{rel_path}.hpp"
    }
    message += write_file( file_path
        ,template_engine.render( "class-hpp"
            ,metadata_provider.get_canonical_metadata( file_path ) | data
        )
    )

    if( create_header_only ):
        return message

    source_dir = default_cpp_config[ "paths" ][ "source" ]
    file_path = f"{source_dir}/{rel_path}.cpp"
    data = {
         "include_list": [ f"{rel_path}.hpp" ]
        ,"des_file_path": f"{rel_path}.cpp"
    }
    message += write_file( file_path
        ,template_engine.render( "class-cpp"
            ,metadata_provider.get_canonical_metadata( file_path ) | data
        )
    )

    return message

def run_create_class( params ):
    ensure( "class_hierarchy" in params, "missing 'class_hierarchy' parameter" )
    
    return create_class( 
         params[ "class_hierarchy" ]
        ,params.get( "include_list", [ ] )
        ,params.get( "using_list", [ ] )
        ,params.get( "create_header_only", False )
    )


if __name__ == "__main__":
    run_mcp_tool( run_create_class )