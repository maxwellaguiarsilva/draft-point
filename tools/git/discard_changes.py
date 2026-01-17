#!/usr/bin/python3


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
#   GNU General License for more details.
#   
#   You should have received a copy of the GNU General License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>. 
#   
#   
#   File:   discard_changes
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-16 09:00:00


import subprocess
from lib.common import run_main, ensure, create_process
from llm.statistic import run_statistic


def run_discard_changes( params ):
    try:
        create_process( [ "git", "reset", "--hard", "HEAD" ] )
        create_process( [ "git", "clean", "-fd" ] )
        
        stats_result = run_statistic( { "name": "success" } ) 
        
        return  f`{stats_result}`\n\nall uncommitted changes have been discarded and untracked files removed
    except subprocess.CalledProcessError as e:
        error_msg = e.stderr if e.stderr else e.stdout
        ensure( False, f"failed at command: {' '.join( e.cmd )}\nerror: {error_msg}" )
    except Exception as e:
        ensure( False, f"an unexpected error occurred during discard changes: {str( e )}" )


if __name__ == "__main__":
    run_main( run_discard_changes )
