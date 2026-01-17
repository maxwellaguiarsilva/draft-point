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
#   File:   quick_upload
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-13 18:00:00


import subprocess
from lib.common import run_mcp_tool, ensure, create_process
from llm.statistic import run_statistic


def run_quick_upload( params ):
    message = params.get( "message" )
    ensure( message, "message parameter is required for quick upload" )

    try:
        create_process( [ "git", "pull" ] )
        create_process( [ "git", "add", "." ] )
        create_process( [ "git", "commit", "-m", message ] )
        create_process( [ "git", "push" ] )
        
        stats_result = run_statistic( { "name": "success" } )
        
        return  f"`{stats_result}`\n\n{message}"
    except subprocess.CalledProcessError as e:
        error_msg = e.stderr if e.stderr else e.stdout
        if "nothing to commit" in error_msg.lower( ):
             return  "nothing to commit"
        
        ensure( False, f"failed at command: {' '.join( e.cmd )}\nerror: {error_msg}" )
    except Exception as e:
        ensure( False, f"an unexpected error occurred during quick upload: {str( e )}" )


if __name__ == "__main__":
    run_mcp_tool( run_quick_upload )


