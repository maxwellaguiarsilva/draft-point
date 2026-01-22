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
from typing import Any
from fastmcp import FastMCP
from lib.common import create_process
from lib.config import default_config


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
            ,env = os.environ | { "PYTHONPATH" : tools_dir, "MCP_VALID": "1" } 
        )
        return  f"{name} successful:\n{process.stdout}"
    except Exception as e:
        return  f"{name} failed: {str( e )}"


tool_registry = [
    {
         "mcp_name": "cpp_analyze"
        ,"group": "cpp"
        ,"script": "analyze"
        ,"description": """runs static analysis (cppcheck) and automatically fixes formatting rules
beyond checking, it also applies fixes for the rules verified by 'verify_formatting' on all .cpp and .hpp files
this command takes no arguments"""
        ,"parameters": [ ]
    }
    ,{
         "mcp_name": "cpp_code_verifier"
        ,"group": "cpp"
        ,"script": "code_verifier"
        ,"description": """verifies if a list of files follows the project's formatting rules
if flg_auto_fix is true, allows the tool to attempt to adjust automatically ( false as default )
returns a consolidated list of violations
to verify and process the entire project, prefer the `analyze` tool. the `code_verifier` tool is recommended for a small group of files or just a single file"""
        ,"parameters": [
             { "name": "files", "type": list[ str ] }
            ,{ "name": "flg_auto_fix", "type": bool, "default": False }
        ]
    }
    ,{
         "mcp_name": "cpp_compile"
        ,"group": "cpp"
        ,"script": "compile"
        ,"description": """compiles the project using
this command takes no arguments"""
        ,"parameters": [ ]
    }
    ,{
         "mcp_name": "cpp_create_class"
        ,"group": "cpp"
        ,"script": "create_class"
        ,"description": """creates a new c++ class with corresponding .hpp and .cpp files
the class_hierarchy parameter defines the namespace and class name (e.g., "game/player" creates class 'player' in namespace 'game')
optional include_list and using_list parameters allow specifying additional headers to include and 'using' declarations to add
good example: include_list=["string", "vector"], using_list=[ "::std::string", "::std::vector", "item_list   =   vector< string >"]
bad example: include_list="<string>", using_list="using std::string;" """
        ,"parameters": [
             { "name": "class_hierarchy", "type": str }
            ,{ "name": "include_list", "type": list[ str ], "default": [ ] }
            ,{ "name": "using_list", "type": list[ str ], "default": [ ] }
            ,{ "name": "create_header_only", "type": bool, "default": False }
        ]
    }
    ,{
         "mcp_name": "cpp_create_test"
        ,"group": "cpp"
        ,"script": "create_test"
        ,"description": """creates a new c++ test file
if flg_adhoc is true, creates an adhoc test in tests/adhoc/nnnn_hierarchy/
in adhoc mode, 'hierarchy' must be a simple name (no slashes or paths)
if flg_adhoc is false, creates a structured test in tests/path/test_path_hierarchy.cpp"""
        ,"parameters": [
             { "name": "hierarchy", "type": str }
            ,{ "name": "flg_adhoc", "type": bool, "default": False }
            ,{ "name": "include_list", "type": list[ str ], "default": [ ] }
        ]
    }
    ,{
         "mcp_name": "cpp_include_tree"
        ,"group": "cpp"
        ,"script": "include_tree"
        ,"description": """displays the include tree of a c++ file (cpp or hpp)
it recursively analyzes includes
call this tool without any arguments to use the project's main file"""
        ,"parameters": [
            { "name": "file_path", "type": str, "default": None }
        ]
    }
    ,{
         "mcp_name": "git_discard_changes"
        ,"group": "git"
        ,"script": "discard_changes"
        ,"description": """discards all uncommitted changes and removes untracked files
this tool reverts the repository to the state of the last commit (head)"""
        ,"parameters": [ ]
    }
    ,{
         "mcp_name": "git_quick_upload"
        ,"group": "git"
        ,"script": "quick_upload"
        ,"description": """performs a quick git upload: pull, add all, commit with message, and push
this tool is intended for simple, non-conflicting changes to increase agility"""
        ,"parameters": [
            { "name": "message", "type": str }
        ]
    }
    ,{
         "mcp_name": "llm_adhoc_tool"
        ,"group": "llm"
        ,"script": "adhoc_tool"
        ,"description": """executes experimental logic defined in tools/llm/adhoc_tool.py
this tool is used for prototyping new functionalities
the 'params' dictionary is passed to the script"""
        ,"parameters": [
            { "name": "params", "type": dict }
        ]
    }
    ,{
         "mcp_name": "llm_statistic"
        ,"group": "llm"
        ,"script": "statistic"
        ,"description": """records or retrieves agent behavioral statistics
if 'name' is provided, increments the count for that event ( can be a string or a list of strings )
if no arguments are provided, returns the current statistics table
this tool accepts a literal call with no arguments
if you identify that you have made a mistake that has already been recorded previously, increment the counter
this is a support tool to help prioritize attention for repeat offenders"""
        ,"parameters": [
            { "name": "name", "type": Any, "default": None }
        ]
    }
    ,{
         "mcp_name": "python_code_verifier"
        ,"group": "python"
        ,"script": "code_verifier"
        ,"description": """verifies if a list of python files follows the project's formatting rules
if flg_auto_fix is true, allows the tool to attempt to adjust automatically ( false as default )
returns a consolidated list of violations"""
        ,"parameters": [
             { "name": "files", "type": list[ str ] }
            ,{ "name": "flg_auto_fix", "type": bool, "default": False }
        ]
    }
]


def create_tool_wrapper( group, script, description, params_def ):
    """creates a function with specific signature for mcp registration"""
    def tool_wrapper( **kwargs ):
        return  _invoke_tool( group, script, kwargs )
    
    tool_wrapper.__doc__ = description
    
    #   setup annotations and signature
    annotations = { }
    sig_params = [ ]
    
    for p in params_def:
        name = p[ "name" ]
        annotation = p.get( "type", Any )
        default = p.get( "default", inspect.Parameter.empty )
        
        annotations[ name ] = annotation
        sig_params.append( inspect.Parameter( 
             name
            ,inspect.Parameter.POSITIONAL_OR_KEYWORD
            ,default = default
            ,annotation = annotation
        ) )
        
    annotations[ "return" ] = str
    tool_wrapper.__annotations__ = annotations
    tool_wrapper.__signature__ = inspect.Signature( sig_params )
    
    return  tool_wrapper


def register_tools( ):
    """registers tools dynamically using data from tool_registry"""
    for tool_def in tool_registry:
        mcp_name = tool_def[ "mcp_name" ]
        group = tool_def[ "group" ]
        script = tool_def[ "script" ]
        description = tool_def[ "description" ]
        params_def = tool_def[ "parameters" ]
        
        wrapper = create_tool_wrapper( group, script, description, params_def )
        mcp.tool( name = mcp_name )( wrapper )


register_tools( )


if __name__ == "__main__":
    mcp.run( )


