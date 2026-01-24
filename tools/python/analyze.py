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
#   File:   tools/python/analyze.py
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-24 15:51:01
#


import os
from lib.common import run_mcp_tool, print_line
from python.code_verifier import run_code_verifier
from python.python_lib.python_config import project_python_config


def run_analyze( ) -> str:
    """applies python_code_verifier in all *.py files found inside tools with the flg_auto_fix = true"""
    files_to_check  =   [ ]
    path_tools      =   project_python_config[ "paths" ][ "tools" ]
    extension       =   project_python_config[ "language" ][ "extension" ]

    for root, dirs, files in os.walk( path_tools ):
        for file in files:
            if file.endswith( f".{extension}" ):
                files_to_check.append( os.path.join( root, file ) )
                
    print_line( )
    print( f"checking python code formatting for {len( files_to_check )} files..." )
    
    result_fmt = run_code_verifier( files_to_check, flg_auto_fix = True )
    if result_fmt.strip( ):
        print( result_fmt )
    
    print_line( )
    return  "python analysis completed successfully"


if __name__ == "__main__":
    run_mcp_tool( run_analyze )


