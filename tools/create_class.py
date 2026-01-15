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
#   File:   create_class
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-15 15:00:00


from lib.common import run_main, ensure
from file_generator import create_class


def run_create_class( params ):
    ensure( "class_hierarchy" in params, "missing 'class_hierarchy' parameter" )
    
    return create_class( 
         params[ "class_hierarchy" ]
        ,params.get( "include_list", [ ] )
        ,params.get( "using_list", [ ] )
        ,params.get( "create_header_only", False )
    )


if __name__ == "__main__":
    run_main( run_create_class )
