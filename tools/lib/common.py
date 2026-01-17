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
#   File:   common
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-04 20:18:54


import json
import sys
import os
import subprocess


def ensure( expression, message ):
    if not expression:
        raise Exception( message )


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
    return source


def create_process( command, **kwargs ):
    params = {
         "check": True
        ,"capture_output": True
        ,"text": True
    }
    params.update( kwargs )
    return  subprocess.run( command, **params )


def print_line( strong = True ):
    line_size = 50
    char = "=" if strong else "-"
    print( char * line_size )


def get_path_parts( path ):
    base = os.path.dirname( path )
    folder = os.path.basename( base )
    name, extension = os.path.splitext( os.path.basename( path ) )
    return  {
         "base": base
        ,"folder": folder
        ,"name": name
        ,"extension": extension
    }


def write_file( file_path, content ):
    os.makedirs( os.path.dirname( file_path ), exist_ok=True )
    with open( file_path, "w" ) as f:
        f.write( content )
    return f"created file: {file_path}\n"


def read_file( file_path ):
    with open( file_path, "r" ) as f:
        return  f.read( )


def get_json_args( ):
    params  =   { }
    if len( sys.argv ) > 1:
        try:
            params = json.loads( sys.argv[ 1 ] )
        except json.JSONDecodeError:
            print( "invalid json parameters" )
            sys.exit( 1 )
    return  params


def run_mcp_tool( action ):
    try:
        print( action( get_json_args( ) ) )
    except Exception as error:
        print( str( error ), file = sys.stderr )
        sys.exit( 1 )


