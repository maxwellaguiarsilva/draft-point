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
#   File:   tools/lib/fso/get_file_list.py
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-24 16:29:35
#


from pathlib import Path
from lib.common import ensure, ensure_list 


def get_file_list( path: str, extensions = None, flg_recursive = True ):
    p = Path( path )
    ext_list = [ ext if ext.startswith( "." ) else f".{ext}" for ext in ensure_list( extensions, str ) ]
    return  [
        str( f ) for f in p.glob( "**/*" if flg_recursive else "*" ) 
        if f.is_file( ) and ( extensions is None or f.suffix in ext_list )
    ]


