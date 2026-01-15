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
#   File:   include_tree
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-13 14:30:00


import os
import re
from lib.common import run_main, ensure
from lib.project_core import project_core


#   project_core: r'#include\\s*[<\\"]([^>\\"]+)[>\\"]'
#   include_tree: r'#include\s*([<"">]([^>"">]+)[>""<])'


class include_tree:
    def __init__( self, tree ):
        self.tree = tree

    def get_report( self, target_file: str ) -> str:
        abs_path = os.path.abspath( target_file )
        node = self.tree.nodes.get( abs_path )
        
        ensure( node, f"file {target_file} not found in project tree" )
        
        output = [ ]
        
        def print_tree( current_node, prefix="", is_last=True, visited=None, is_redundant=False ):
            if visited is None:
                visited = set( )
            
            connector = "└── " if is_last else "├── "
            
            display_name = current_node.hierarchy
            if current_node.is_external:
                display_name = f"<{display_name}>"
            elif current_node.path and current_node.path.endswith( ".hpp" ):
                display_name = f"\"{current_node.hierarchy}.hpp\""
            elif current_node.path and current_node.path.endswith( ".cpp" ):
                display_name = f"\"{current_node.hierarchy}.cpp\""

            line = prefix + connector + display_name
            
            if is_redundant:
                line += " (redundant)"
            elif current_node in visited:
                line += " (recursion)"
            
            output.append( line )
            
            if is_redundant or current_node in visited:
                return

            new_visited = visited | { current_node }
            new_prefix = prefix + ( "    " if is_last else "│   " )
            
            for i, ref in enumerate( current_node.direct_includes ):
                print_tree( 
                    ref.target_node, 
                    new_prefix, 
                    i == len( current_node.direct_includes ) - 1, 
                    new_visited,
                    ref.is_redundant
                )

        #   Special case for root node to match original display style if possible
        #   Original used target_file as display name for root.
        #   We'll use a slightly different approach to start the tree.
        
        def print_root( root_node ):
            output.append( root_node.path if root_node.path else root_node.hierarchy )
            for i, ref in enumerate( root_node.direct_includes ):
                print_tree( 
                    ref.target_node, 
                    "", 
                    i == len( root_node.direct_includes ) - 1, 
                    { root_node },
                    ref.is_redundant
                )

        print_root( node )
        return "\n".join( output )

def run_include_tree( params: dict ) -> str:
    core = project_core( params.get( "config", { } ) )
    file_path = params.get( "file_path" )
    
    if not file_path:
        for c in core.cpp_list:
            if c.is_main and not c.is_test:
                file_path = c.path
                break
    
    ensure( file_path, "could not determine a target file for include_tree analysis" )
    
    analyzer = include_tree( core.tree )
    return analyzer.get_report( file_path )

if __name__ == "__main__":
    run_main( run_include_tree )
