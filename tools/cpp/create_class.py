#!/usr/bin/python3

#   
#   Copyright (C) 2026 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
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
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#   
#   
#   File:   tools/cpp/create_class.py
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-16 14:22:13
#

import time
from lib.common import run_mcp_tool, ensure
from lib.template import template
from cpp_lib.config import default_cpp_config
from cpp_lib.project_model import parse_hierarchy


def run_create_class( params ):
    ensure( "class_hierarchy" in params, "missing 'class_hierarchy' parameter" )

    message = ""
    hierarchy_list = parse_hierarchy( params[ "class_hierarchy" ] )
    rel_path = "/".join( hierarchy_list )

    message +=  template( "cpp/class-hpp" ).create_file( 
         f"{default_cpp_config[ 'paths' ][ 'include' ]}/{rel_path}.{default_cpp_config[ 'language' ][ 'header_extension' ]}"
        ,{
             "comment_string": default_cpp_config[ "language" ][ "comment_string" ]
            ,"header_guard": f"header_guard_{ str( time.time_ns( ) )[ -9: ] }"
            ,"class_name": hierarchy_list[ -1 ]
            ,"include_list": params.get( "include_list", [ ] )
            ,"namespace_list": hierarchy_list[ :-1 ]
            ,"using_list": params.get( "using_list", [ ] )
        }
    )

    if( params.get( "create_header_only", False ) ):
        return  message
    
    message +=  template( "cpp/class-cpp" ).create_file( 
         f"{default_cpp_config[ 'paths' ][ 'source' ]}/{rel_path}.{default_cpp_config[ 'language' ][ 'source_extension' ]}"
        ,{
             "comment_string": default_cpp_config[ "language" ][ "comment_string" ]
            ,"include_list": [ f"{rel_path}.{default_cpp_config[ 'language' ][ 'header_extension' ]}" ]
        }
    )

    return  message


if __name__ == "__main__":
    run_mcp_tool( run_create_class )


