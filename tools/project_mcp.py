#!/usr/bin/python3


import json
import os
import subprocess
from lib.common import _invoke_subprocess
from typing import Any
from fastmcp import FastMCP


#   create an mcp server instance
mcp = FastMCP( name="project-mcp" )



def _invoke_tool( name: str, args: Any = None ) -> str:
    """runs a command and formats the output for mcp return"""
    try:
        process = _invoke_subprocess( [ "python3", f"tools/{name}.py", json.dumps( args if args is not None else { } ) ] )
        return f"{name} successful:\n{process.stdout}"
    except subprocess.CalledProcessError as e:
        return f"{name} failed:\n{e.stdout}\n{e.stderr}".strip( )
    except Exception as e:
        return f"{name} failed: {str( e )}"


@mcp.tool( )
def adhoc_tool( params: dict ) -> str:
    """executes experimental logic defined in tools/adhoc-tool.py
    this tool is used for prototyping new functionalities
    the 'params' dictionary is passed to the script
    """
    return _invoke_tool( "adhoc_tool", params )


@mcp.tool( )
def agent_statistic( name: Any = None ) -> str:
    """records or retrieves agent behavioral statistics
    if 'name' is provided, increments the count for that event ( can be a string or a list of strings )
    if no arguments are provided, returns the current statistics table
    this tool accepts a literal call with no arguments
    if you identify that you have made a mistake that has already been recorded previously, increment the counter
    this is a support tool to help prioritize attention for repeat offenders
    """
    return _invoke_tool( "agent_statistic", locals( ).copy( ) )


@mcp.tool( )
def quick_upload( message: str ) -> str:
    """performs a quick git upload: pull, add all, commit with message, and push
    this tool is intended for simple, non-conflicting changes to increase agility
    """
    return _invoke_tool( "quick_upload", locals( ).copy( ) )


@mcp.tool( )
def include_tree( file_path: str ) -> str:
    """displays the include tree of a c++ file (cpp or hpp)
    it recursively analyzes includes
    """
    return _invoke_tool( "include_tree", locals( ).copy( ) )


@mcp.tool( )
def	create_class(
         class_hierarchy : str
        ,include_list: list[ str ] = [ ]
        ,using_list: list[ str ] = [ ]
        ,create_header_only: bool = False
    ) -> str:
    """creates a new c++ class with corresponding .hpp and .cpp files
    the class_hierarchy parameter defines the namespace and class name (e.g., "game/player" creates class 'player' in namespace 'game')
    optional include_list and using_list parameters allow specifying additional headers to include and 'using' declarations to add
    good example: include_list=["string", "vector"], using_list=[ "::std::string", "::std::vector", "item_list   =   vector< string >"]
    bad example: include_list="<string>", using_list="using std::string;"
    """
    return _invoke_tool( "create_class", locals( ).copy( ) )


@mcp.tool( )
def create_test(
         hierarchy: str
        ,flg_adhoc: bool = False
        ,include_list: list[ str ] = [ ]
    ) -> str:
    """creates a new c++ test file
    if flg_adhoc is true, creates an adhoc test in tests/adhoc/nnnn_hierarchy/
    in adhoc mode, 'hierarchy' must be a simple name (no slashes or paths)
    if flg_adhoc is false, creates a structured test in tests/path/test_path_hierarchy.cpp
    """
    return _invoke_tool( "create_test", locals( ).copy( ) )


@mcp.tool( )
def compile( ) -> str:
    """compiles the project using
    this command takes no arguments"""
    return _invoke_tool( "compile" )


@mcp.tool( )
def analyze( ) -> str:
    """runs static analysis (cppcheck) and automatically fixes formatting rules
    beyond checking, it also applies fixes for the rules verified by 'verify_formatting' on all .cpp and .hpp files
    this command takes no arguments"""
    return _invoke_tool( "analyze" )


@mcp.tool( )
def code_verifier( files: list[ str ], flg_auto_fix: bool = False ) -> str:
    """this tool is exclusively for cpp and hpp files
    verifies if a list of files follows the project's formatting rules
    if flg_auto_fix is true, allows the tool to attempt to adjust automatically ( false as default )
    returns a consolidated list of violations
    to verify and process the entire project, prefer the `analyze` tool. the `code_verifier` tool is recommended for a small group of files or just a single file
    """
    return _invoke_tool( "code_verifier", locals( ).copy( ) )


if __name__ == "__main__":
    mcp.run( )
