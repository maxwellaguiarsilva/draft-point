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


from pathlib import Path
from lib.template import template
from lib.common import run_mcp_tool, ensure
from cpp_lib.config import default_cpp_config
from cpp_lib.project_model import parse_hierarchy


def get_next_adhoc_prefix( adhoc_dir ):
    path = Path( adhoc_dir )
    path.mkdir( exist_ok = True )
    
    ids = {
        int( p.name.split( "_" )[ 0 ] )
        for p in path.iterdir( )
        if p.is_dir( ) and p.name.split( "_" )[ 0 ].isdigit( )
    }
    
    next_counter = 1
    while next_counter in ids:
        next_counter += 1
    
    return  f"{next_counter:04d}"



def run_create_test( params ):
    ensure( "hierarchy" in params, "missing 'hierarchy' parameter" )

    hierarchy = params[ "hierarchy" ]
    
    if( params.get( "flg_adhoc", False ) ):
        tests_dir = default_cpp_config[ "paths" ][ "adhoc" ]
        prefix = get_next_adhoc_prefix( tests_dir )
        rel_path = f"{prefix}_{hierarchy}/{prefix}_{hierarchy}.{default_cpp_config[ 'language' ][ 'source_extension' ]}"
    else:
        tests_dir = default_cpp_config[ "paths" ][ "tests" ]
        hierarchy_list = parse_hierarchy( hierarchy )
        rel_path = "/".join( hierarchy_list[ :-1 ] + [ f"test_{ '_'.join( hierarchy_list ) }.{default_cpp_config[ 'language' ][ 'source_extension' ]}" ] )

    file_path = f"{tests_dir}/{rel_path}"

    return  template( "cpp/test-cpp", comment_string = default_cpp_config[ "language" ][ "comment_string" ] ).create_file( 
         file_path
        ,{
             "hierarchy": hierarchy
            ,"include_list": params.get( "include_list", [ ] )
        }
    )



if __name__ == "__main__":
    run_mcp_tool( run_create_test )


