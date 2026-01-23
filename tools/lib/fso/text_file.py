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
#   File:   tools/lib/fso/text_file.py
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-23 15:54:56
#

import os
from lib.fso.file import file


class text_file( file ):
    #   do not display self.content in __repr__ or json

    def __init__( self, file_path ):
        super( ).__init__( file_path )
    
    def refresh( self ):
        self.content    =   self.read( )
    
    def read( self ):
        super( ).refresh( )
        if self.exists:
            with open( self.path, "r" ) as f:
                return  f.read( )
        return  None
    
    def write( self, content ):
        os.makedirs( os.path.dirname( self.path ), exist_ok=True )
        with open( self.path, "w" ) as f:
            f.write( content )
        self.refresh( )
        return  f"created file: {self.path}\n"


