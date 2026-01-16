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
#   File:   statistic
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-10 00:01:07



import json
import os
from lib.common import run_main, ensure


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


def run_statistic( params ):
    #   validate allowed fields
    allowed_fields = { "name", "short-description" }
    for key in params:
        ensure( key in allowed_fields, f"field '{key}' is not allowed" )

    #   ensure directory exists
    os.makedirs( os.path.dirname( _statistic_file ), exist_ok=True )
    
    if os.path.exists( _statistic_file ):
        try:
            with open( _statistic_file, "r" ) as f:
                data = json.load( f )
        except ( json.JSONDecodeError, IOError ):
            data = [ ]
    else:
        data = [ ]

    name_param = params.get( "name" )
    short_description = params.get( "short-description" )
    
    if name_param:
        if isinstance( name_param, str ):
            increment_event( data, name_param, short_description )
        elif isinstance( name_param, list ):
            for n in name_param:
                increment_event( data, n, short_description )
        else:
            ensure( False, "field 'name' must be a string or a list of strings" )
            
        #   save data
        with open( _statistic_file, "w" ) as f:
            json.dump( data, f, indent="\t" )
            
    return  format_output( data )


if __name__ == "__main__":
    run_main( run_statistic )


