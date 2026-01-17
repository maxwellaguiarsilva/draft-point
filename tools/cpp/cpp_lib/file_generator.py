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
#   File:   file_generator
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-04 19:36:49


import os
import time
import re
from lib.common import run_main
from cpp.lib.config import DEFAULT_CPP_CONFIG
from lib import metadata_provider
from lib import template_engine


def write_file( file_path, content ):
    os.makedirs( os.path.dirname( file_path ), exist_ok=True )
    with open( file_path, "w" ) as f:
        f.write( content )
    return f"created file: {file_path}\n"


def parse_hierarchy( hierarchy ):
    return re.split( r"[/:\\.]+", hierarchy )

def generate_header_guard( ):
    return f"header_guard_{ str( time.time_ns( ) )[ -9: ] }"

def get_next_adhoc_prefix( adhoc_dir ):
    os.makedirs( adhoc_dir, exist_ok=True )
    existing_adhocs = [ d for d in os.listdir( adhoc_dir ) if os.path.isdir( os.path.join( adhoc_dir, d ) ) ]
    existing_counters = set( )
    for d in existing_adhocs:
        match = re.match( r"(\d+)_", d )
        if( match ):
            existing_counters.add( int( match.group( 1 ) ) )
    
    next_counter = 1
    while( next_counter in existing_counters ):
        next_counter += 1
    return f"{next_counter:04d}"

def render_and_write( file_path, template_name, extra_data ):
    data = metadata_provider.get_canonical_metadata( file_path ) | extra_data
    content = template_engine.render( template_name, data )
    return write_file( file_path, content )

def create_class( class_hierarchy, include_list=[ ], using_list=[ ], create_header_only=False ):
    message = ""
    hierarchy_list = parse_hierarchy( class_hierarchy )
    rel_path = "/".join( hierarchy_list )
    
    include_dir = DEFAULT_CPP_CONFIG[ "paths" ][ "include" ]
    file_path_hpp = f"{include_dir}/{rel_path}.hpp"
    message += render_and_write( file_path_hpp, "class-hpp", {
         "header_guard": generate_header_guard( )
        ,"class_name": hierarchy_list[ -1 ]
        ,"include_list": include_list
        ,"namespace_list": hierarchy_list[ :-1 ]
        ,"using_list": using_list
        ,"des_file_path": f"{rel_path}.hpp"
    } )

    if( create_header_only ):
        return message

    source_dir = DEFAULT_CPP_CONFIG[ "paths" ][ "source" ]
    file_path_cpp = f"{source_dir}/{rel_path}.cpp"
    message += render_and_write( file_path_cpp, "class-cpp", {
         "include_list": [ f"{rel_path}.hpp" ]
        ,"des_file_path": f"{rel_path}.cpp"
    } )

    return message

def create_test( hierarchy, flg_adhoc=False, include_list=[ ] ):
    paths = DEFAULT_CPP_CONFIG[ "paths" ]
    
    if( flg_adhoc ):
        adhoc_dir = paths[ "adhoc" ]
        prefix = get_next_adhoc_prefix( adhoc_dir )
        test_folder = f"{prefix}_{hierarchy}"
        rel_path = f"{test_folder}/{prefix}_{hierarchy}.cpp"
        file_path = f"{adhoc_dir}/{rel_path}"
        display_hierarchy = hierarchy
    else:
        tests_dir = paths[ "tests" ]
        hierarchy_list = parse_hierarchy( hierarchy )
        path = "/".join( hierarchy_list[ :-1 ] )
        filename = f"test_{ '_'.join( hierarchy_list ) }.cpp"
        rel_path = f"{path}/{filename}" if path else filename
        file_path = f"{tests_dir}/{rel_path}"
        display_hierarchy = hierarchy

    return render_and_write( file_path, "test-cpp", {
         "hierarchy": display_hierarchy
        ,"include_list": include_list
        ,"des_file_path": file_path
    } )

def run_file_generator( params ):
    if( "class_hierarchy" in params ):
        return create_class( 
             params[ "class_hierarchy" ]
            ,params.get( "include_list", [ ] )
            ,params.get( "using_list", [ ] )
            ,params.get( "create_header_only", False )
        )
    elif( "hierarchy" in params ):
        return create_test(
             params[ "hierarchy" ]
            ,params.get( "flg_adhoc", False )
            ,params.get( "include_list", [ ] )
        )
    return "invalid parameters for file_generator"


if( __name__ == "__main__" ):
    run_main( run_file_generator )