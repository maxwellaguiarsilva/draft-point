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
#   Created on 2025-10-04 15:55:17


import re
import sys
import json

r_import        =   r"\{\{import\s+([a-zA-Z0-9_-]+)\}\}"
r_list_open     =   r"\{\{list_open\s+([a-zA-Z0-9_-]+)\}\}"
r_list_close    =   r"\{\{list_close\s+([a-zA-Z0-9_-]+)\}\}"
r_list_item     =   r"\{\{list_item\s+([a-zA-Z0-9_-]+)\}\}"

class template:
    def __init__( self, name, path = "docs/templates" ):
        assert( name != "" )
        self.path = path
        self.text = self.load( name )
    
    def load( self, name ):
        with open( f"{self.path}/{name}.txt", "r" ) as file:
            text = file.read( )
        text = re.sub( r_import, lambda match: self.load( match.group( 1 ) ), text )
        return  text
    
    def run( self, data: dict ) -> str:
        text = self.text
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
        return  text


if __name__ == "__main__":
    if len( sys.argv ) > 2:
        template_name = sys.argv[ 1 ]
        try:
            data = json.loads( sys.argv[ 2 ] )
            t = template( template_name )
            print( t.run( data ), end="" )
        except Exception as e:
            print( f"Error: {e}", file=sys.stderr )
            sys.exit( 1 )
    


