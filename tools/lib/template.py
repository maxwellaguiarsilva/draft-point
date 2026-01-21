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
#   File:   template
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-14 18:00:00


import re
import os


from lib.config import default_config
from lib.common import read_file, write_file
from lib import file_info


r_import        =   r"\{\{import\s+([a-zA-Z0-9_/-]+)\}\}"
r_list_open     =   r"\{\{list_open\s+([a-zA-Z0-9_/-]+)\}\}"
r_list_close    =   r"\{\{list_close\s+([a-zA-Z0-9_/-]+)\}\}"
r_list_item     =   r"\{\{list_item\s+([a-zA-Z0-9_/-]+)\}\}"


class template:
    def __init__( self, name, path = None, comment_string = None ):
        assert( name != "" )
        self.path = path if path is not None else default_config[ "paths" ][ "templates" ]
        self.comment_string = comment_string
        self.text = self.load( name )
    
    def load( self, name ):
        text = read_file( f"{self.path}/{name}.txt" )
        
        def resolve_imports( content ):
            return re.sub( r_import, lambda match: resolve_imports( self.load_raw( match.group( 1 ) ) ), content )
        
        return resolve_imports( text )

    def load_raw( self, name ):
        return read_file( f"{self.path}/{name}.txt" )

    def _render_dict( self, data: dict, text: str ) -> str:
        if self.comment_string:
            data = { "comment_string": self.comment_string } | data
        for key, value in data.items( ):
            if isinstance( value, str ):
                text = text.replace( f"{{{{{key}}}}}", str( value ) )
            elif isinstance( value, list ):
                r_key = (
                        r"\{\{list_open\s+"
                    +   re.escape( key )
                    +   r"\}\}(.*?)\{\{list_close\s+"
                    +   re.escape( key )
                    +   r"\}\}"
                )
                r_item = (
                        r"\{\{list_item\s+"
                    +   re.escape( key )
                    +   r"\}\}"
                )
                text = re.sub(
                     r_key
                    ,lambda match: "".join( [ re.sub( r_item, str( item ), match.group( 1 ) ) for item in value ] )
                    ,text
                    ,flags = re.DOTALL
                )
            elif isinstance( value, dict ):
                text = self._render_dict( value, text )
        return  text

    
    def render( self, data: dict, file_path: str = None ) -> str:
        if file_path:
            data    =   file_info.get_info( file_path ) | data
        return  self._render_dict( data, self.text )
    
    def create_file( self, file_path, data ):
        return  write_file( file_path, self.render( data, file_path ) )


