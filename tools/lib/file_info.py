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
#   File:   tools/lib/file_info.py
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-18 10:35:16
#


import os
import datetime
from lib.common import create_process
from lib.project_config import project_config


def get_git_config_value( configuration_name ):
    return  create_process( [ "git", "config", "--global", configuration_name ] ).stdout.strip( )


def get_info( file_path ):
    """generates the official metadata set for a file (existing or new)."""
    #   fallback
    data    =   {
         "num_year": datetime.datetime.now( ).strftime( "%Y" )
        ,"des_full_name": project_config[ "author" ][ "name" ]
        ,"des_email": project_config[ "author" ][ "email" ]
        ,"des_formatted_datetime": datetime.datetime.now( ).strftime( "%Y-%m-%d %H:%M:%S" )
        ,"des_file_path": file_path
    }

    if not os.path.exists( file_path ):
        return  data
    process =   create_process( [
        "git", "log", "--follow", "--reverse", 
        "--date=format:%Y-%m-%d %H:%M:%S", 
        "--format=%ad|%an|%ae", "--", file_path
    ] )
    if not process.stdout.strip( ):
        return  data

    dat_full, des_full_name, des_email  =   process.stdout.splitlines( )[ 0 ].split( "|" )
    
    return  data | {
         "num_year": dat_full.split( "-" )[ 0 ]
        ,"des_full_name": des_full_name
        ,"des_email": des_email
        ,"des_formatted_datetime": dat_full
    }


def strip_header( content, comment_string ):
    lines = content.splitlines( keepends = True )
    shebang = ""
    start_index = 0
    if lines and lines[ 0 ].startswith( "#!" ):
        shebang = lines[ 0 ]
        start_index = 1
    
    comment_prefix = comment_string.rstrip( )
    body_start = start_index
    for i in range( start_index, len( lines ) ):
        line = lines[ i ]
        if not line.strip( ) or ( comment_prefix and line.startswith( comment_prefix ) ):
            body_start = i + 1
        else:
            break
            
    return  shebang, "".join( lines[ body_start: ] )


