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
#   File:   tools/lib/common.py
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-13 19:46:34
#


import datetime
import json
import sys
import os
import subprocess
import inspect


def ensure( expression, message ):
    if not expression:
        raise Exception( message )


def all_is_instance( items, cls ):
    return  isinstance( items, cls ) or ( isinstance( items, list ) and all( isinstance( item, cls ) for item in items ) )

def ensure_list( value, cls = None, message = "" ):
    ensure( cls is None or all_is_instance( value, cls ), message or f"invalid parameter: must be a {cls.__name__} or a list of {cls.__name__}" )
    return  value if isinstance( value, list ) else [ value ]


def get_tool_metadata( action ):
    parameters = [ ]
    for name, param in inspect.signature( action ).parameters.items( ):
        p_def = { "name": name }
        if param.annotation is not inspect.Parameter.empty:
            p_def[ "type" ] = param.annotation
        if param.default is not inspect.Parameter.empty:
            p_def[ "default" ] = param.default
        parameters.append( p_def )
        
    return  {
         "description": inspect.getdoc( action ) or ""
        ,"parameters": parameters
    }


def validate_params( params, required = None, optional = None ):
    required = required or [ ]
    for key in required:
        ensure( key in params, f"missing required parameter: '{key}'" )
    for key in params:
        ensure( key in ( set( required ) | set( optional or [ ] ) ), f"unexpected parameter: '{key}'" )


def get_cpu_count( ):
    try:
        return  len( os.sched_getaffinity( 0 ) )
    except AttributeError:
        return  os.cpu_count( ) or 1


def deep_update( source, overrides ):
    for key, value in overrides.items( ):
        if isinstance( value, dict ) and key in source and isinstance( source[ key ], dict ):
            deep_update( source[ key ], value )
        else:
            source[ key ] = value
    return  source


def create_process( command, **kwargs ):
    params = {
         "check": True
        ,"capture_output": True
        ,"text": True
    }
    params.update( kwargs )
    try:
        return  subprocess.run( command, **params )
    except subprocess.CalledProcessError as e:
        error_msg = get_process_text( e ).strip( )
        if not error_msg:
             error_msg = str( e )
             
        cmd_str = ' '.join( e.cmd ) if isinstance( e.cmd, list ) else str( e.cmd )
        raise Exception( f"failed at command: {cmd_str}\nerror: {error_msg}" ) from None


def get_process_text( result ):
    lines = [ ]
    if result.stderr: lines.append( result.stderr.rstrip( "\n" ) )
    if result.stdout: lines.append( result.stdout.rstrip( "\n" ) )
    return  "\n".join( lines )


def print_line( strong = True ):
    line_size = 50
    char = "=" if strong else "-"
    print( char * line_size )


def get_json_args( ):
    params  =   { }
    if len( sys.argv ) > 1:
        try:
            params = json.loads( sys.argv[ 1 ] )
        except json.JSONDecodeError:
            print( "invalid json parameters" )
            sys.exit( 1 )
    return  params


def to_dict( obj, members ):
    def __to_dict( value ):
        if hasattr( value, "to_dict" ):
            return  value.to_dict
        if isinstance( value, dict ):
            return  { key: __to_dict( val ) for key, val in value.items( ) }
        if isinstance( value, list ):
            return  [ __to_dict( val ) for val in value ]
        return  value

    return  { member: __to_dict( getattr( obj, member ) ) for member in members }


def to_json( data ):
    return  json.dumps( data, indent = 4, default = str )


def run_mcp_tool( action ):
    try:
        if os.environ.get( "MCP_CALL_LOCK" ) != "1":
            from llm.statistic import run_statistic
            ensure( False, run_statistic( name="direct-mcp-call" ) )
        print( action( **( get_json_args( ) ) ) )
    except Exception as error:
        print( str( error ), file = sys.stderr )
        sys.exit( 1 )


