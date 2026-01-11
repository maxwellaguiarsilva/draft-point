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
#   File:   adhoc_tool
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-04 20:18:54


import json
import sys
import subprocess

def quick_upload( message ):
    try:
        #   Executing git commands in sequence
        #   If any command fails, it will raise a CalledProcessError
        
        #   1. Pull latest changes
        subprocess.run( [ "git", "pull" ], check=True, capture_output=True, text=True )
        
        #   2. Add all changes
        subprocess.run( [ "git", "add", "." ], check=True, capture_output=True, text=True )
        
        #   3. Commit with the provided message
        #   We use a list to avoid shell injection and handle spaces correctly
        subprocess.run( [ "git", "commit", "-m", message ], check=True, capture_output=True, text=True )
        
        #   4. Push to remote
        subprocess.run( [ "git", "push" ], check=True, capture_output=True, text=True )
        
        #   5. Record success statistic
        stats_process = subprocess.run( [ "python3", "tools/agent_statistic.py", json.dumps( { "name": "success" } ) ], check=True, capture_output=True, text=True )
        
        return f"Quick upload successful: `{message}`\n\n{stats_process.stdout}"
    except subprocess.CalledProcessError as e:
        error_msg = e.stderr if e.stderr else e.stdout
        return f"Quick upload failed at command: {" ".join(e.cmd)}\nError: {error_msg}"
    except Exception as e:
        return f"An unexpected error occurred during quick upload: {str(e)}"

def run_adhoc( params ):
    action = params.get( "action" )
    
    if action == "quick_upload":
        message = params.get( "message" )
        if not message:
            return "Error: 'message' parameter is required for quick_upload action."
        return quick_upload( message )
    
    #   Keep support for existing adhoc functionality if needed
    files = params.get( "files", [ ] )
    if files:
        from tools.adhoc_tool_legacy import verify_rules_batch
        return verify_rules_batch( files )
        
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
