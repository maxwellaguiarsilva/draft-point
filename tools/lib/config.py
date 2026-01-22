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
#   File:   config
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-16 11:00:00


#   this file must store data that orchestrates how the process behaves (machine logic)
#   it must not be used as a repository for string or regex constants (string externalization)


from lib.common import get_cpu_count, deep_update


default_config = {
    #   folder organization
    "paths": {
         "tools": "tools"              #   project tools directory
        ,"docs": "docs"                #   project documentation directory
        ,"templates": "docs/templates" #   project templates directory
    }
}


