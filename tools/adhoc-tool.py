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
#   File:   adhoc-tool
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-04 20:18:54


import json
import sys

def run_adhoc( params ):
    """Placeholder for experimental logic.
    After the logic is validated and moved to a production script,
    this function must be cleaned up to maintain a minimal state.
    """
    action = params.get( "action" )
    
    if action == "hello":
        return f"Hello, {params.get( 'name', 'world' )}!"
        
    return "Error: No valid action or parameters provided to adhoc_tool."

if __name__ == "__main__":
    if len( sys.argv ) > 1:
        try:
            params = json.loads( sys.argv[ 1 ] )
            print( run_adhoc( params ) )
        except json.JSONDecodeError:
            print( "Error: Invalid JSON parameters." )
    else:
        print( "Error: No parameters provided." )