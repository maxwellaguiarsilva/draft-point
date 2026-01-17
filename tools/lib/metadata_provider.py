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
#   File:   metadata_provider
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-14 18:00:00


import os
import datetime
from lib.common import create_process


def fetch_git_first_commit( file_path ):
    """Retrieves date, name and email from the first commit of a file via Git."""
    if not os.path.exists( file_path ):
        return None
    #   Format: YYYY-MM-DD HH:MM|Name|Email
    cmd = [
        "git", "log", "--follow", "--reverse", 
        "--date=format:%Y-%m-%d %H:%M", 
        "--format=%ad|%an|%ae", "--", file_path
    ]
    result = create_process( cmd )
    if not result.stdout.strip( ):
        return None
    first_line = result.stdout.splitlines( )[ 0 ]
    dt, name, email = first_line.split( '|' )
    return {
        "year": dt.split( '-' )[ 0 ],
        "datetime": dt,
        "name": name,
        "email": email
    }


def get_git_config_value( configuration_name ):
    command = [ "git", "config", "--global", configuration_name ]
    process = create_process( command )
    return process.stdout.strip( )


def get_canonical_metadata( full_relative_path ):
    """Generates the official metadata set for a file (existing or new)."""
    git_info = fetch_git_first_commit( full_relative_path )
    
    if git_info:
        return {
             "num_year": git_info[ "year" ]
            ,"des_full_name": git_info[ "name" ]
            ,"des_email": git_info[ "email" ]
            ,"des_formatted_datetime": git_info[ "datetime" ]
        }
    
    #   Fallback for new files (not yet committed)
    return {
         "num_year": datetime.datetime.now( ).strftime( "%Y" )
        ,"des_full_name": get_git_config_value( "user.name" )
        ,"des_email": get_git_config_value( "user.email" )
        ,"des_formatted_datetime": datetime.datetime.now( ).strftime( "%Y-%m-%d %H:%M" )
    }