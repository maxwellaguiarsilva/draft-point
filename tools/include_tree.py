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
    def __init__( self, include_dir: str ):
        self.include_dir = include_dir
        self.include_pattern = re.compile( r'#include\s*([<"">]([^>"">]+)[>""<])' )
        self.closure_cache = { }

    def _resolve_path( self, include_name: str ):
        path = os.path.join( self.include_dir, include_name )
        if os.path.exists( path ) and path.endswith( ".hpp" ):
            return path
        return None

    def _get_closure( self, file_path: str, visited: set ) -> set:
        if not file_path or file_path in visited:
            return set( )
        if file_path in self.closure_cache:
            return self.closure_cache[ file_path ]
            
        closure = set( )
        try:
            with open( file_path, "r", encoding="utf-8" ) as f:
                content = f.read( )
            
            matches = self.include_pattern.findall( content )
            for full_match, inner_match in matches:
                closure.add( full_match )
                resolved = self._resolve_path( inner_match )
                closure |= self._get_closure( resolved, visited | { file_path } )
        except:
            pass
            
        self.closure_cache[ file_path ] = closure
        return closure

    def _build_node( self, display_name: str, file_path: str, local_visited: set, is_redundant: bool = False ):
        node = { "display": display_name, "path": file_path, "children": [ ], "is_redundant": is_redundant }
        
        if is_redundant:
            return node

        if not file_path:
            node[ "is_system" ] = True
            return node

        if file_path in local_visited:
            node[ "recursion" ] = True
            return node
        
        new_local_visited = local_visited | { file_path }
        
        try:
            with open( file_path, "r", encoding="utf-8" ) as f:
                content = f.read( )
            
            matches = self.include_pattern.findall( content )
            
            children_info = [ ]
            for full_match, inner_match in matches:
                resolved = self._resolve_path( inner_match )
                closure = self._get_closure( resolved, new_local_visited )
                children_info.append( {
                    "full": full_match,
                    "resolved": resolved,
                    "closure": closure | { full_match }
                } )
                
            for i, info in enumerate( children_info ):
                redundant = False
                for j, other in enumerate( children_info ):
                    if i != j and info[ "full" ] in other[ "closure" ]:
                        redundant = True
                        break
                
                child_node = self._build_node( info[ "full" ], info[ "resolved" ], new_local_visited, redundant )
                node[ "children" ].append( child_node )
                
        except Exception as e:
            node[ "error" ] = str( e )
            
        return node

    def get_report( self, target_file: str ) -> str:
        self.closure_cache.clear( )
        
        root_node = self._build_node( target_file, target_file, set( ) )
        
        output = [ ]
        
        def print_tree( node, prefix="", is_last=True ):
            connector = "└── " if is_last else "├── "
            line = prefix + connector + node[ "display" ]
            
            if node.get( "recursion" ):
                line += " (recursion)"
            elif node.get( "is_redundant" ):
                line += " (redundant)"
            elif node.get( "error" ):
                line += f" (error: {node['error']})"

            output.append( line )
            
            new_prefix = prefix + ( "    " if is_last else "│   " )
            for i, child in enumerate( node[ "children" ] ):
                print_tree( child, new_prefix, i == len( node[ "children" ] ) - 1 )

        print_tree( root_node )
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
    
    analyzer = include_tree( core.config[ "paths" ][ "include" ] )
    return analyzer.get_report( file_path )

if __name__ == "__main__":
    run_main( run_include_tree )
