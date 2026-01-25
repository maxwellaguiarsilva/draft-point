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
#   File:   tools/lib/project_config.py
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-24 15:51:01
#


from lib.common import get_cpu_count, deep_update, get_git_config, ensure


project_config = {
    "max_threads": get_cpu_count( )
    #   folder organization
    ,"paths": {
         "tools": "tools"              #   project tools directory
        ,"docs": "docs"                #   project documentation directory
        ,"templates": "docs/templates" #   project templates directory
    }
    ,"author": {
         "name": get_git_config( "user.name" ) or ensure( False, "failed to retrieve user.name from git" )
        ,"email": get_git_config( "user.email" ) or ensure( False, "failed to retrieve user.email from git" )
    }
    ,"locale": {
        "datetime-format": "%Y-%m-%d %H:%M:%S"
    }

}


