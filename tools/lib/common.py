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
import subprocess


def ensure( expression, message ):
    if not expression:
        raise Exception( message )


def _invoke_subprocess( command ):
    return  subprocess.run( command, check=True, capture_output=True, text=True )


def run_main( action ):
    params  =   { }
    if len( sys.argv ) > 1:
        try:
            params = json.loads( sys.argv[ 1 ] )
        except json.JSONDecodeError:
            print( "invalid json parameters" )
            sys.exit( 1 )
    try:
        print( action( params ) )
    except Exception as error:
        print( str( error ), file = sys.stderr )
        sys.exit( 1 )


