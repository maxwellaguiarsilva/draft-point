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
#   File:   tools/lib/project_file.py
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-25 15:40:46
#


from abc import ABC, abstractmethod
from datetime import datetime
from itertools import takewhile
from os.path import getctime
import re
from lib.fso import text_file
from lib.project_config import project_config


line_break = project_config[ "system" ][ "pattern" ][ "line_break" ]


class project_file( text_file, ABC ):
    @property
    @abstractmethod
    def shebang( self ) -> str:
        pass

    @property
    @abstractmethod
    def comment_string( self ) -> str:
        pass

    @property
    def license_header_info( self ) -> dict:
        info = {
             "name": project_config[ "author" ][ "name" ]
            ,"email": project_config[ "author" ][ "email" ]
            ,"created_at": getctime( self.path ) if self.exists else datetime.now( ).timestamp( )
        }
        
        if not self.license_header:
            return  info
        
        comment = re.escape( self.comment_string )
        author_match = re.search( rf"{comment}.*Author:\s*(.*?)\s*<([^>]+)>", self.license_header )
        date_match = re.search( rf"{comment}.*Created on\s*(.*)", self.license_header )
        
        if author_match:
            info[ "name" ] = author_match.group( 1 ).strip( )
            info[ "email" ] = author_match.group( 2 ).strip( )
        
        if date_match:
            try:
                info[ "created_at" ] = datetime.strptime(
                     date_match.group( 1 ).strip( )
                    ,project_config[ "locale" ][ "datetime-format" ]
                ).timestamp( )
            except ValueError:
                pass
        
        return  info

    @property
    def license_header( self ) -> str:
        if not self.content:
            return  None
        
        comment = self.comment_string
        start_index = 0
        
        if self.shebang:
            if not self.content.startswith( self.shebang + line_break * 2 + comment ):
                return  None
            start_index = 2
        elif not self.content.startswith( comment ):
            return  None
        
        return  "".join( takewhile(
            lambda line: comment and line.startswith( comment )
            ,self.content.splitlines( keepends = True )[ start_index: ]
        ) )


class generic_project_file( project_file ):
    def __init__( self, file_path, shebang = "", comment_string = "#\t" ):
        self._shebang = shebang
        self._comment_string = comment_string
        super( ).__init__( file_path )

    @property
    def shebang( self ):
        return  self._shebang

    @property
    def comment_string( self ):
        return  self._comment_string