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
#   GNU General License for more details.
#   
#   You should have received a copy of the GNU General License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#   
#   
#   File:   quick-upload
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-13 18:00:00


import json
import subprocess
from lib.common import run_main


def quick_upload( params ):
    message = params.get( "message" )
    if not message:
        return "error: 'message' parameter is required for quick_upload"

    try:
        #   1. Pull latest changes
        subprocess.run( [ "git", "pull" ], check=True, capture_output=True, text=True )
        
        #   2. Add all changes
        subprocess.run( [ "git", "add", "." ], check=True, capture_output=True, text=True )
        
        #   3. Commit with the provided message
        subprocess.run( [ "git", "commit", "-m", message ], check=True, capture_output=True, text=True )
        
        #   4. Push to remote
        subprocess.run( [ "git", "push" ], check=True, capture_output=True, text=True )
        
        #   5. Record success statistic
        stats_process = subprocess.run( [ "python3", "tools/agent-statistic.py", json.dumps( { "name": "success" } ) ], check=True, capture_output=True, text=True )
        
        return f"quick upload successful: `{message}`\n\n{stats_process.stdout}"
    except subprocess.CalledProcessError as e:
        error_msg = e.stderr if e.stderr else e.stdout
        if "nothing to commit" in error_msg.lower( ):
             return "quick upload aborted: nothing to commit"
        return f"quick upload failed at command: {'. '.join( e.cmd )}\nerror: {error_msg}"
    except Exception as e:
        return f"an unexpected error occurred during quick upload: {str( e )}"


if __name__ == "__main__":
    run_main( quick_upload )
