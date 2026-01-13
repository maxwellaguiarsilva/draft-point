#!/usr/bin/python3


import json
import os
import subprocess
from typing import Any
from fastmcp import FastMCP


#   create an mcp server instance
mcp = FastMCP( name="project-tools-mcp" )


_special_tool_config = {
     "create_class":      { "script": "file-generator" }
    ,"create_test":       { "script": "file-generator" }
    ,"verify_formatting": { "script": "code-verifier" }
    ,"include_tree":      { "script": "include-analyzer" }
    ,"quick_upload":      { "script": "git-util" }
    ,"compile":           { "script": "project-builder", "subcommand": "run_build" }
    ,"analyze":           { "script": "project-builder" }
}


def _call( args: list[ str ] ) -> subprocess.CompletedProcess:
    """Internal helper to run subprocess with consistent parameters."""
    return subprocess.run( args, capture_output=True, text=True, check=True )

def _run_and_format( name: str, args: Any = None ) -> str:
    """Runs a command and formats the output for MCP return."""
    config = _special_tool_config.get( name, { } )
    script = f"{config.get( 'script', name.replace( '_', '-' ) )}.py"
    label = name.replace( '_', ' ' )
    
    cmd = [ "python3", f"tools/{script}" ]
    
    #   Subcommand is mandatory for our standardized script entry points
    cmd.append( config.get( "subcommand", name ) )
    
    #   Arguments are always passed as a JSON string
    cmd.append( json.dumps( args if args is not None else { } ) )
            
    try:
        process = _call( cmd )
        return f"{label} successful:\n{process.stdout}"
    except subprocess.CalledProcessError as e:
        details = f"{e.stdout}\n{e.stderr}".strip( )
        return f"{label} failed:\n{details}"
    except Exception as e:
        return f"{label} failed: {str( e )}"


@mcp.tool( )
def	create_class(
         class_hierarchy : str
        ,include_list: list[ str ] = [ ]
        ,using_list: list[ str ] = [ ]
        ,create_header_only: bool = False
    ) -> str:
    """Creates a new C++ class with corresponding .hpp and .cpp files.
    The class_hierarchy parameter defines the namespace and class name (e.g., "game/player" creates class 'player' in namespace 'game').
    Optional include_list and using_list parameters allow specifying additional headers to include and 'using' declarations to add.
    Good Example: include_list=["string", "vector"], using_list=["::std::string", "::std::vector", "item_list   =   vector< string >"]
    Bad Example: include_list="<string>", using_list="using std::string;"
    """
    args = {
        "class_hierarchy": class_hierarchy,
        "include_list": include_list,
        "using_list": using_list,
        "create_header_only": create_header_only
    }
    return _run_and_format( "create_class", args )


@mcp.tool( )
def create_test(
         hierarchy: str
        ,flg_adhoc: bool = False
        ,include_list: list[ str ] = [ ]
    ) -> str:
    """Creates a new C++ test file.
    If flg_adhoc is True, creates an adhoc test in tests/adhoc/NNNN_hierarchy/.
    IMPORTANT: In adhoc mode, 'hierarchy' MUST be a simple name (no slashes or paths).
    If flg_adhoc is False, creates a structured test in tests/path/test_path_hierarchy.cpp.
    """
    args = {
        "hierarchy": hierarchy,
        "flg_adhoc": flg_adhoc,
        "include_list": include_list
    }
    return _run_and_format( "create_test", args )


@mcp.tool( )
def compile( ) -> str:
    """Compiles the project using project-builder.py. This command takes no arguments."""
    return _run_and_format( "compile" )


@mcp.tool( )
def analyze( ) -> str:
    """Runs static analysis (cppcheck) and automatically fixes formatting rules.
    Beyond checking, it also applies fixes for the rules verified by 'verify_formatting' (newlines, return spacing, etc.) on all .cpp and .hpp files.
    This command takes no arguments."""
    return _run_and_format( "analyze" )


@mcp.tool( )
def verify_formatting( files: list[ str ], flg_auto_fix: bool = False ) -> str:
    """Verifies if a list of files follows the project's formatting rules (newlines, return spacing, bracket spacing, etc.).
    If flg_auto_fix is True, allows the tool to attempt to adjust automatically ( Default False ).
    Returns a consolidated list of violations.
    These rules are specific to hpp and cpp files.
    To verify and process the entire project, prefer the `analyze`` tool. The `verify_formatting` tool is recommended for a small group of files or just a single file.
    """
    args = {
        "files": files,
        "flg_auto_fix": flg_auto_fix
    }
    return _run_and_format( "verify_formatting", args )


@mcp.tool( )
def include_tree( file_path: str ) -> str:
    """Displays the include tree of a C++ file (cpp or hpp).
    It recursively analyzes includes, identifies duplicates, and shows system headers without expanding them.
    """
    args = {
        "file_path": file_path
    }
    return _run_and_format( "include_tree", args )


@mcp.tool( )
def agent_statistic( name: Any = None ) -> str:
    """Records or retrieves agent behavioral statistics.
    If 'name' is provided, increments the count for that event.
    If no arguments are provided, returns the current statistics table.
    Check the current data at the beginning of every conversation.
    This tool accepts a literal call with no arguments.
    If you identify that you have made a mistake that has already been recorded previously, increment the counter.
    This is a support tool to help prioritize attention for repeat offenders.
    """
    args = { }
    if isinstance( name, str ):
        args[ "name" ] = name
    return _run_and_format( "agent_statistic", args )


@mcp.tool( )
def adhoc_tool( params: dict ) -> str:
    """Executes experimental logic defined in tools/adhoc-tool.py.
    This tool is used for prototyping new functionalities.
    The 'params' dictionary is passed to the script.
    """
    return _run_and_format( "adhoc_tool", params )


@mcp.tool( )
def quick_upload( message: str ) -> str:
    """Performs a quick git upload: pull, add all, commit with message, and push.
    This tool is intended for simple, non-conflicting changes to increase agility.
    """
    args = { "message": message }
    return _run_and_format( "quick_upload", args )


if __name__ == "__main__":
    mcp.run( )


