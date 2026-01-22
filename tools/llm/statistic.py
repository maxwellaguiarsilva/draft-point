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
#   File:   tools/llm/statistic.py
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-16 13:55:19
#

import json
import os
from lib.common import run_mcp_tool, ensure, validate_params
from lib.fso import text_file


_statistic_file = "/home/.gemini/statistic.json"


def format_output( data ):
    if not data:
        return  "no statistics recorded yet"
    
    #   sort by count descending
    sorted_data = sorted( data, key=lambda x: x[ "count" ], reverse=True )
    
    lines = [ ]
    for item in sorted_data:
        lines.append( f"{item[ 'name' ]}( {item[ 'count' ]} ): {item[ 'short-description' ]}" )
    
    return  "\n".join( lines )


def increment_event( data, name, short_description ):
    #   normalize name: lower, trim, spaces to hyphens
    name = name.lower( ).strip( ).replace( " ", "-" )
    
    if short_description:
        short_description = short_description.lower( ).strip( )
        ensure( "." not in short_description, "short-description cannot contain periods ('.')" )
    
    #   find entry
    entry = next( ( item for item in data if item[ "name" ] == name ), None )
    
    if entry:
        entry[ "count" ] += 1
        if short_description:
            entry[ "short-description" ] = short_description
    else:
        ensure( short_description, f"entry '{name}' does not exist" )
        
        new_entry = {
            "name": name,
            "short-description": short_description,
            "count": 1
        }
        data.append( new_entry )


from typing import Any


def run_statistic( name: Any = None, short_description: str = None ) -> str:
    """records or retrieves agent behavioral statistics
if 'name' is provided, increments the count for that event ( can be a string or a list of strings )
if no arguments are provided, returns the current statistics table
this tool accepts a literal call with no arguments
if you identify that you have made a mistake that has already been recorded previously, increment the counter
this is a support tool to help prioritize attention for repeat offenders"""
    f = text_file( _statistic_file )
    if f.exists:
        data = json.loads( f.content )
    else:
        data = [ ]

    name_param = name
    
    if name_param:
        names = [ ]
        if isinstance( name_param, str ):
            names.append( name_param )
        elif isinstance( name_param, list ):
            names.extend( name_param )
        else:
            ensure( False, "field 'name' must be a string or a list of strings" )
            
        normalized_names = [ ]
        for n in names:
            increment_event( data, n, short_description )
            normalized_names.append( n.lower( ).strip( ).replace( " ", "-" ) )
            
        #   save data
        f.write( json.dumps( data, indent="\t" ) )
        
        #   filter data to only show incremented events
        display_data = [ item for item in data if item[ "name" ] in normalized_names ]
        return  format_output( display_data )
            
    return  format_output( data )


if __name__ == "__main__":
    run_mcp_tool( run_statistic )


