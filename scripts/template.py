#!/usr/bin/python3


import re

r_import        =   r"\{\{import\s+([a-zA-Z0-9_-]+)\}\}"
r_list_open     =   r"\{\{list_open\s+([a-zA-Z0-9_-]+)\}\}"
r_list_close    =   r"\{\{list_close\s+([a-zA-Z0-9_-]+)\}\}"
r_list_item     =   r"\{\{list_item\s+([a-zA-Z0-9_-]+)\}\}"

class template:
    def __init__( self, name, path = "docs/templates" ):
        assert( self != "" )
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
                    ,lambda match: "".join( [ re.sub( r_item, item, match.group( 1 ) ) for item in value ] )
                    ,text
                    ,flags = re.DOTALL
                )
        return  text
    


