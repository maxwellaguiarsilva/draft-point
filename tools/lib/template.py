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
#   File:   tools/lib/template.py
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-18 15:27:14
#


import re
import os
from datetime import datetime


from lib.project_config import project_config
from lib.common import ensure
from lib.fso import text_file
from lib.project_file import generic_project_file


r_import        =   r"\{\{import\s+([a-zA-Z0-9_/-]+)\}\}"
r_list_open     =   r"\{\{list_open\s+([a-zA-Z0-9_/-]+)\}\}"
r_list_close    =   r"\{\{list_close\s+([a-zA-Z0-9_/-]+)\}\}"
r_list_item     =   r"\{\{list_item\s+([a-zA-Z0-9_/-]+)\}\}"


def get_metadata( item ):
    info = item.license_header_info
    created_at = datetime.fromtimestamp( info[ "created_at" ] )
    
    return  {
         "file_created_at_year": created_at.strftime( "%Y" )
        ,"file_author_name": info[ "name" ]
        ,"file_author_email": info[ "email" ]
        ,"file_created_at": created_at.strftime( project_config[ "locale" ][ "datetime-format" ] )
        ,"file_path": item.path
    }


class template:
    def __init__( self, name, path = None ):
        ensure( name != "", "template name cannot be empty" )
        self.path           =   path if path is not None else project_config[ "paths" ][ "templates" ]
        self.text           =   self.load( name )
    
    def load( self, name ):
        text    =   text_file( f"{self.path}/{name}.txt" ).content
        
        def resolve_imports( content ):
            return  re.sub( r_import, lambda match: resolve_imports( self.load_raw( match.group( 1 ) ) ), content )
        
        return  resolve_imports( text )

    def load_raw( self, name ):
        return  text_file( f"{self.path}/{name}.txt" ).content

    def _render_dict( self, data: dict, text: str ) -> str:
        for key, value in data.items( ):
            if isinstance( value, str ):
                text    =   text.replace( f"{{{{{key}}}}}", str( value ) )
            elif isinstance( value, list ):
                r_key   =   (
                        r"\{\{list_open\s+"
                    +   re.escape( key )
                    +   r"\}\}(.*?)\{\{list_close\s+"
                    +   re.escape( key )
                    +   r"\}\}"
                )
                r_item  =   (
                        r"\{\{list_item\s+"
                    +   re.escape( key )
                    +   r"\}\}"
                )
                text    =   re.sub(
                     r_key
                    ,lambda match: "".join( [ re.sub( r_item, str( item ), match.group( 1 ) ) for item in value ] )
                    ,text
                    ,flags = re.DOTALL
                )
            elif isinstance( value, dict ):
                text    =   self._render_dict( value, text )
        return  text

    def render( self, data: dict, file_path: str = None ) -> str:
        if file_path:
            comment_string = data.get( "comment_string", "#\t" )
            item    =   generic_project_file( file_path, comment_string = comment_string )
            data    =   get_metadata( item ) | data
        return  self._render_dict( data, self.text )

    def create_file( self, file_path, data ):
        return  text_file( file_path ).write( self.render( data, file_path ) )


