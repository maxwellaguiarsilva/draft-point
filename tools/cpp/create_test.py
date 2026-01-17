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
#   File:   tools/cpp/create_test.py
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-15 15:00:00


import os
import re
from lib.common import run_mcp_tool, ensure, write_file
from lib import metadata_provider
from lib import template_engine
from cpp_lib.config import default_cpp_config
from cpp_lib.project_tree import parse_hierarchy


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

def create_test( hierarchy, flg_adhoc=False, include_list=[ ] ):
    paths = default_cpp_config[ "paths" ]
    
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

    data = {
         "hierarchy": display_hierarchy
        ,"include_list": include_list
        ,"des_file_path": file_path
    }
    return write_file( file_path
        ,template_engine.render( "test-cpp"
            ,metadata_provider.get_canonical_metadata( file_path ) | data
        )
    )

def run_create_test( params ):
    ensure( "hierarchy" in params, "missing 'hierarchy' parameter" )
    
    return create_test(
         params[ "hierarchy" ]
        ,params.get( "flg_adhoc", False )
        ,params.get( "include_list", [ ] )
    )


if __name__ == "__main__":
    run_mcp_tool( run_create_test )