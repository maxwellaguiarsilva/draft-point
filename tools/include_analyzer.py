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
#   File:   include_analyzer
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-13 14:30:00


import json
import os
import re
import sys

class include_tree:
    def __init__( self, include_dir: str = "include" ):
        self.include_dir = include_dir
        self.include_pattern = re.compile( r'#include\s*([<"">]([^>""]+)[>""<])' )
        self.global_visited = set( )
        self.duplicates = set( )

    def _resolve_path( self, include_name: str ):
        path = os.path.join( self.include_dir, include_name )
        if os.path.exists( path ) and path.endswith( ".hpp" ):
            return path
        return None

    def _build_node( self, display_name: str, file_path: str, local_visited: set ):
        node = { "display": display_name, "path": file_path, "children": [ ] }
        
        if display_name in self.global_visited:
            self.duplicates.add( display_name )
            node[ "already_visited" ] = True
            return node
            
        self.global_visited.add( display_name )

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
            for full_match, inner_match in matches:
                resolved = self._resolve_path( inner_match )
                child_node = self._build_node( full_match, resolved, new_local_visited )
                node[ "children" ].append( child_node )
        except Exception as e:
            node[ "error" ] = str( e )
            
        return node

    def get_report( self, target_file: str ) -> str:
        self.global_visited.clear( )
        self.duplicates.clear( )
        
        root_node = self._build_node( target_file, target_file, set( ) )
        
        output = [ ]
        
        def print_tree( node, prefix="", is_last=True ):
            connector = "└── " if is_last else "├── "
            line = prefix + connector + node[ "display" ]
            
            if node.get( "recursion" ):
                line += " (RECURSION)"
            elif node.get( "already_visited" ):
                line += " (*)"
            elif node.get( "error" ):
                line += f" (ERROR: {node['error']})"

            output.append( line )
            
            new_prefix = prefix + ( "    " if is_last else "│   " )
            for i, child in enumerate( node[ "children" ] ):
                print_tree( child, new_prefix, i == len( node[ "children" ] ) - 1 )

        print_tree( root_node )
        
        output.append( "" )
        output.append( "(*) Item already expanded elsewhere in the tree." )
        
        if self.duplicates:
            output.append( "" )
            output.append( "Duplicated Includes (indirectly included more than once):" )
            for dup in sorted( list( self.duplicates ) ):
                if dup != target_file:
                    output.append( f"  - {dup}" )
        else:
            output.append( "" )
            output.append( "No duplicated includes found." )

        return "\n".join( output )

def run_include_tree( file_path: str ) -> str:
    analyzer = include_tree( )
    return analyzer.get_report( file_path )

if __name__ == "__main__":
    if len( sys.argv ) > 2:
        command = sys.argv[ 1 ]
        try:
            if command == "include_tree":
                args = json.loads( sys.argv[ 2 ] )
                print( run_include_tree( args[ "file_path" ] ) )
        except Exception as e:
            print( f"Error: {e}", file=sys.stderr )
            sys.exit( 1 )
    else:
        print( "Usage: include_analyzer.py include_tree '{\"file_path\": \"...\"}'", file=sys.stderr )
        sys.exit( 1 )
