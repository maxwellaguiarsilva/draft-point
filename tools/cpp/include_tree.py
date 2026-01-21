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
#   GNU General License for more details.
#   
#   You should have received a copy of the GNU General License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#   
#   
#   File:   tools/cpp/include_tree.py
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-13 14:30:00


import json
import os
import sys


from lib.common import run_mcp_tool, ensure
from cpp_lib.project_model import project_model
from cpp_lib.config import default_cpp_config


class include_tree:
    def __init__( self, project ):
        self.project = project

    def get_report( self, target_file: str ) -> str:
        node = self.project.files.get( target_file )
        ensure( node, f"file {target_file} not found in project" )
        
        output = [ f"- <{target_file}>" ]
        self._build_tree( node, 1, { target_file }, output )
        return  "\n".join( output )

    def _build_tree( self, node, depth, branch_visited, output ):
        for include in node.includes:
            output.append( f"{'    ' * depth}<{include}>" )
            header = self.project.get_file( include, is_header = True )
            if header and header.path not in branch_visited:
                self._build_tree( header, depth + 1, branch_visited | { header.path }, output )

def run_include_tree( params: dict ) -> str:
    project = project_model( default_cpp_config )
    file_path = params.get( "file_path" )
    
    if not file_path:
        for f in project.files.values( ):
            #   using the same logic as before to find main file
            if hasattr( f, "is_main" ) and f.is_main and not getattr( f, "is_test", False ):
                file_path = f.path
                break
    
    ensure( file_path, "could not determine a target file for include_tree analysis" )
    
    analyzer = include_tree( project )
    return  analyzer.get_report( file_path )

if __name__ == "__main__":
    run_mcp_tool( run_include_tree )


