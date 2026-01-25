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
#   File:   tools/lib/fso/file.py
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-23 15:56:51
#


from datetime import datetime
from os.path import basename, dirname, exists, getmtime, splitext
from lib.common import to_dict, to_json


class file:
    def __init__( self, file_path ):
        self.path   =   file_path
    
    @property
    def path( self ):
        return  self.file_path

    @path.setter
    def path( self, path ):
        name, extension =   splitext( basename( path ) )
        self.file_path  =   path
        self.base       =   dirname( path )
        self.folder     =   basename( self.base )
        self.name       =   name
        self.extension  =   extension[ 1: ]
        self.refresh( )
    
    def refresh( self ):
        self.exists         =   exists( self.path )
        self.modified_at    =   datetime.fromtimestamp( getmtime( self.path ) ) if self.exists else None

    @property
    def to_dict( self ):
        return  to_dict( self, {
             "path"
            ,"base"
            ,"folder"
            ,"name"
            ,"extension"
            ,"exists"
            ,"modified_at"
        } )

    def __repr__( self ):
        return  to_json( self.to_dict )


