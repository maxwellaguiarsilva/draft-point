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
#   File:   adhoc-tool
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-04 20:18:54


import json
import sys
import subprocess
import os
import re

def quick_upload( message ):
    try:
        #   Executing git commands in sequence
        #   If any command fails, it will raise a CalledProcessError
        
        #   1. Pull latest changes
        subprocess.run( [ "git", "pull" ], check=True, capture_output=True, text=True )
        
        #   2. Add all changes
        subprocess.run( [ "git", "add", "." ], check=True, capture_output=True, text=True )
        
        #   3. Commit with the provided message
        #   We use a list to avoid shell injection and handle spaces correctly
        subprocess.run( [ "git", "commit", "-m", message ], check=True, capture_output=True, text=True )
        
        #   4. Push to remote
        subprocess.run( [ "git", "push" ], check=True, capture_output=True, text=True )
        
        #   5. Record success statistic
        stats_process = subprocess.run( [ "python3", "tools/agent-statistic.py", json.dumps( { "name": "success" } ) ], check=True, capture_output=True, text=True )
        
        return f"Quick upload successful: `{message}`\n\n{stats_process.stdout}"
    except subprocess.CalledProcessError as e:
        error_msg = e.stderr if e.stderr else e.stdout
        return f"Quick upload failed at command: {" ".join(e.cmd)}\nError: {error_msg}"
    except Exception as e:
        return f"An unexpected error occurred during quick upload: {str(e)}"

def get_include_tree( target_file ):
    include_dir = "include"
    #   Captures both the full include (with <> or "") and the inner content
    include_pattern = re.compile( r'#include\s*([<\"]([^>\"]+)[>\"])' )
    
    global_visited = set( )
    duplicates = set( )
    
    def resolve_path( include_name ):
        #   Try relative to include/
        path = os.path.join( include_dir, include_name )
        if os.path.exists( path ) and path.endswith( ".hpp" ):
            return path
        return None

    def build_node( display_name, file_path, local_visited ):
        node = { "display": display_name, "path": file_path, "children": [ ] } 
        
        #   Duplication check is agnostic to the type of include
        if display_name in global_visited:
            duplicates.add( display_name )
            node[ "already_visited" ] = True
            return node
            
        global_visited.add( display_name )

        if not file_path:
            node[ "is_system" ] = True
            return node

        #   Safety check for infinite recursion
        if file_path in local_visited:
            node[ "recursion" ] = True
            return node
        
        new_local_visited = local_visited | { file_path }
        
        try:
            with open( file_path, "r", encoding="utf-8" ) as f:
                content = f.read( )
            
            matches = include_pattern.findall( content )
            for full_match, inner_match in matches:
                resolved = resolve_path( inner_match )
                child_node = build_node( full_match, resolved, new_local_visited )
                node[ "children" ].append( child_node )
        except Exception as e:
            node[ "error" ] = str( e )
            
        return node

    #   Initial file is always considered a project file
    root_node = build_node( target_file, target_file, set( ) )
    
    #   Formatting the tree view
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
    
    if duplicates:
        output.append( "" )
        output.append( "Duplicated Includes (indirectly included more than once):" )
        for dup in sorted( list( duplicates ) ):
            if dup != target_file:
                output.append( f"  - {dup}" )
    else:
        output.append( "" )
        output.append( "No duplicated includes found." )

    return "\n".join( output )

def run_adhoc( params ):
    action = params.get( "action" )
    
    if action == "quick_upload":
        message = params.get( "message" )
        if not message:
            return "Error: 'message' parameter is required for quick_upload action."
        return quick_upload( message )
    
    if action == "include_tree":
        file_path = params.get( "file_path" )
        if not file_path:
            return "Error: 'file_path' parameter is required for include_tree action."
        return get_include_tree( file_path )
    
    #   Keep support for existing adhoc functionality if needed
    files = params.get( "files", [ ] )
    if files:
        return "Error: verify_rules_batch is no longer supported in adhoc_tool."
        
    return "Error: No valid action or parameters provided to adhoc_tool."

if __name__ == "__main__":
    if len( sys.argv ) > 1:
        try:
            params = json.loads( sys.argv[ 1 ] )
            print( run_adhoc( params ) )
        except json.JSONDecodeError:
            print( "Error: Invalid JSON parameters." )
    else:
        print( "Error: No parameters provided." )
