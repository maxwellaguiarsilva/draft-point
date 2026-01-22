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
#   File:   tools/cpp/analyze.py
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-16 14:22:13
#

import concurrent.futures
from lib.common import run_mcp_tool, get_cpu_count, print_line


from cpp_lib.project_core import project_core
from cpp_lib.verifier import run_cpp_verifier


def run_analyze( params ):
    """runs static analysis (cppcheck) and automatically fixes formatting rules
    beyond checking, it also applies fixes for the rules verified by 'verify_formatting' on all .cpp and .hpp files
    this command takes no arguments"""
    core = project_core( params.get( "config", { } ) )
    
    include_ext = core.config["language"]["header_extension"]
    source_ext  = core.config["language"]["source_extension"]
    
    files_to_check = [ f.path for f in core.map.files.values( ) if f.path ]
            
    print_line( )
    core.print( f"checking code formatting for {len(files_to_check)} files..." )
    
    max_workers = core.config[ "build_behavior" ].get( "max_threads", get_cpu_count( ) )

    fmt_params = {
         "files": files_to_check
        ,"flg_auto_fix": True
    }
    
    result_fmt = run_cpp_verifier( fmt_params )
    if "file:" in result_fmt:
        core.print( result_fmt )
    
    core.print( "done formatting" )
    print_line( )

    core.run_cppcheck( )
    
    return  "analysis completed successfully"


if __name__ == "__main__":
    run_mcp_tool( run_analyze )


