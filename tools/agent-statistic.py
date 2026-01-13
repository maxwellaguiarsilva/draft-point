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
#   File:   agent-statistic
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-10 00:01:07



import json
import os
import sys


STATISTIC_FILE = "/home/.gemini/statistic.json"


def format_output( data ):
    if not data:
        return "No statistics recorded yet."
    
    # Sort by count descending
    sorted_data = sorted( data, key=lambda x: x[ "count" ], reverse=True )
    
    lines = [ ]
    for item in sorted_data:
        lines.append( f"{item[ 'name' ]}( {item[ 'count' ]} ): {item[ 'short-description' ]}" )
    
    return "\n".join( lines )


def run_statistic( params ):
    # Validate allowed fields
    allowed_fields = { "name", "short-description" }
    for key in params:
        if key not in allowed_fields:
            return f"Error: field '{key}' is not allowed. Only 'name' and 'short-description' are accepted."

    # Ensure directory exists
    os.makedirs( os.path.dirname( STATISTIC_FILE ), exist_ok=True )
    
    if os.path.exists( STATISTIC_FILE ):
        try:
            with open( STATISTIC_FILE, "r" ) as f:
                data = json.load( f )
        except ( json.JSONDecodeError, IOError ):
            data = [ ]
    else:
        data = [ ]

    name = params.get( "name" )
    short_description = params.get( "short-description" )
    
    if name:
        # Normalize name: lower, trim, spaces to hyphens
        name = name.lower( ).strip( ).replace( " ", "-" )
        
        # Normalize description if provided: lower, trim, check for "."
        if short_description:
            short_description = short_description.lower( ).strip( )
            if "." in short_description:
                return "Error: short-description cannot contain periods ('.')."
        
        # Find entry
        entry = next( ( item for item in data if item[ "name" ] == name ), None )
        
        if entry:
            entry[ "count" ] += 1
            if short_description:
                entry[ "short-description" ] = short_description
        else:
            if not short_description:
                return f"Error: entry '{name}' does not exist. Please provide 'short-description' to create it."
            
            new_entry = {
                "name": name,
                "short-description": short_description,
                "count": 1
            }
            data.append( new_entry )
            
        # Save data
        with open( STATISTIC_FILE, "w" ) as f:
            json.dump( data, f, indent="\t" )
            
    return format_output( data )


if __name__ == "__main__":
    try:
        if len( sys.argv ) > 1:
            params = json.loads( sys.argv[ 1 ] )
            print( run_statistic( params ) )
        else:
            print( run_statistic( { } ) )
    except Exception as e:
        print( f"Error: {str( e )}" )


