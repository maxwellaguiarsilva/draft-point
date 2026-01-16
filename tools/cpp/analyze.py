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
#   File:   tools/cpp/analyze.py
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-14 18:00:00


import concurrent.futures
from lib.common import run_main, get_cpu_count
import json
import os
import subprocess
import sys


from cpp.lib.project_core import project_core
from cpp.code_verifier import run_code_verifier


def run_analyze( params ):
    """runs static analysis (cppcheck) and automatically fixes formatting rules
    beyond checking, it also applies fixes for the rules verified by 'verify_formatting' on all .cpp and .hpp files
    this command takes no arguments"""
    core = project_core( params.get( "config", { } ) )
    
    #   1. Format code (Verificação de estilo nativa)
    include_ext = core.config["patterns"]["header_extension"]
    source_ext  = core.config["patterns"]["source_extension"]
    
    files_to_check = [ ]
    for obj in core.hpp_list + core.cpp_list:
        files_to_check.append( obj.path )
            
    core.print( "=" * 50 )
    core.print( f"checking code formatting for {len(files_to_check)} files..." )
    
    max_workers = core.config[ "build_behavior" ].get( "max_threads", get_cpu_count( ) )

    #   chamada nativa para a função code_verifier.run_code_verifier
    #   note que passamos flg_auto_fix=True por padrão na ferramenta analyze
    fmt_params = {
         "files": files_to_check
        ,"flg_auto_fix": True
    }
    
    result_fmt = run_code_verifier( fmt_params )
    if "File:" in result_fmt:
        core.print( result_fmt )
    
    core.print( "done formatting" )
    core.print( "=" * 50 )

    #   2. Static Analysis (cppcheck)
    core.run_cppcheck( )
    
    return "analysis completed successfully"


if __name__ == "__main__":
    run_main( run_analyze )
