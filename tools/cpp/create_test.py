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
#   File:   tools/cpp/create_test.py
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-16 14:22:13
#


from lib.template import template
from lib.common import run_mcp_tool, ensure
from cpp_lib.cpp_config import project_cpp_config
from cpp_lib.project_model import project_model


def run_create_test( 
     hierarchy: str
    ,flg_adhoc: bool = False
    ,include_list: list[ str ] = [ ]
) -> str:
    """creates a new c++ test file
if flg_adhoc is true, creates an adhoc test in tests/adhoc/nnnn_hierarchy/
in adhoc mode, 'hierarchy' must be a simple name (no slashes or paths)
if flg_adhoc is false, creates a structured test in tests/path/test_path_hierarchy.cpp"""
    model = project_model( project_cpp_config )
    
    file_path = model.get_path_for_hierarchy( 
         hierarchy
        ,"test"
        ,flg_adhoc = flg_adhoc 
    )

    return  template( "cpp/test-cpp" ).create_file( 
         file_path
        ,{
             "comment_string": project_cpp_config[ "language" ][ "comment_string" ]
            ,"hierarchy": hierarchy
            ,"include_list": include_list
        }
    )



if __name__ == "__main__":
    run_mcp_tool( run_create_test )


