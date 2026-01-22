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
#   File:   tools/project_mcp.py
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-14 01:31:32
#

import json
import os
import inspect
import importlib.util
from typing import Any
from fastmcp import FastMCP
from lib.common import create_process, get_tool_metadata
from lib.base_config import default_config


#   create an mcp server instance
mcp = FastMCP( name = "project-mcp" )

#   important: this file manages the mcp server configuration
#   whenever this file is modified, you must ask the user to manually restart the mcp server
#   to apply and verify the changes. do not attempt to verify it automatically


def _invoke_tool( group: str, name: str, args: Any = None ) -> str:
    """runs a command and formats the output for mcp return"""
    tools_dir = default_config[ "paths" ][ "tools" ]
    try:
        process = create_process( 
             [ "python3", f"{tools_dir}/{group}/{name}.py", json.dumps( args if args is not None else { } ) ]
            ,env = os.environ | { "PYTHONPATH" : tools_dir, "MCP_CALL_LOCK": "1" } 
        )
        return  f"{name} successful:\n{process.stdout}"
    except Exception as e:
        return  f"{name} failed: {str( e )}"


def discover_tools( ):
    """scans the tools directory and returns a list of tool definitions"""
    tools_dir = default_config[ "paths" ][ "tools" ]
    tool_list = [ ]
    
    #   get subdirectories (groups)
    groups = [ 
        d for d in os.listdir( tools_dir ) 
        if os.path.isdir( os.path.join( tools_dir, d ) ) 
        and d not in [ "lib", "mcp-cli", "resources", "__pycache__" ] 
    ]
    
    for group in sorted( groups ):
        group_dir = os.path.join( tools_dir, group )
        for filename in sorted( os.listdir( group_dir ) ):
            if filename.endswith( ".py" ) and not filename.startswith( "__" ):
                name = filename[ :-3 ]
                module_name = f"{group}.{name}"
                file_path = os.path.join( group_dir, filename )
                
                #   load module to inspect metadata
                spec = importlib.util.spec_from_file_location( module_name, file_path )
                if spec is None or spec.loader is None:
                    continue
                    
                module = importlib.util.module_from_spec( spec )
                
                #   add tools_dir and group_dir to sys.path to resolve imports within the tool
                import sys
                sys.path.insert( 0, tools_dir )
                sys.path.insert( 0, group_dir )
                try:
                    spec.loader.exec_module( module )
                except Exception as e:
                    #   just skip tools that fail to load during discovery
                    #   they will still be listed but might fail when called
                    print( f"warning: failed to load tool {module_name}: {e}" )
                    continue
                finally:
                    sys.path.pop( 0 )
                    sys.path.pop( 0 )
                
                func_name = f"run_{name}"
                if hasattr( module, func_name ):
                    action = getattr( module, func_name )
                    metadata = get_tool_metadata( action )
                    
                    tool_list.append( {
                         "name": name
                        ,"group": group
                        ,"description": metadata[ "description" ]
                        ,"parameters": metadata[ "parameters" ]
                    } )
                    
    return  tool_list


def create_tool_wrapper( group, name, description, params_def ):
    """creates a function with specific signature for mcp registration"""
    def tool_wrapper( **kwargs ):
        return  _invoke_tool( group, name, kwargs )
    
    tool_wrapper.__doc__ = description
    
    #   setup annotations and signature
    annotations = { }
    sig_params = [ ]
    
    for p in params_def:
        name_param = p[ "name" ]
        annotation = p.get( "type", Any )
        default = p.get( "default", inspect.Parameter.empty )
        
        annotations[ name_param ] = annotation
        sig_params.append( inspect.Parameter( 
             name_param
            ,inspect.Parameter.POSITIONAL_OR_KEYWORD
            ,default = default
            ,annotation = annotation
        ) )
        
    annotations[ "return" ] = str
    tool_wrapper.__annotations__ = annotations
    tool_wrapper.__signature__ = inspect.Signature( sig_params )
    
    return  tool_wrapper


def register_tools( ):
    """registers tools dynamically using data from discover_tools"""
    for tool_def in discover_tools( ):
        group = tool_def[ "group" ]
        name = tool_def[ "name" ]
        mcp_name = f"{group}_{name}"
        description = tool_def[ "description" ]
        params_def = tool_def[ "parameters" ]
        
        wrapper = create_tool_wrapper( group, name, description, params_def )
        mcp.tool( name = mcp_name )( wrapper )


register_tools( )


if __name__ == "__main__":
    mcp.run( )


