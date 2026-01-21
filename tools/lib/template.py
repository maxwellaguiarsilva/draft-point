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


from lib.common import ensure


from lib.fso import text_file


from lib import file_info


# ... (regex constants remain same)


class template:


    def __init__( self, name, path = None, comment_string = None ):


        assert( name != "" )


        self.path = path if path is not None else default_config[ "paths" ][ "templates" ]


        self.comment_string = comment_string


        self.text = self.load( name )


    


    def load( self, name ):


        text = text_file( f"{self.path}/{name}.txt" ).content


        


        def resolve_imports( content ):


            return re.sub( r_import, lambda match: resolve_imports( self.load_raw( match.group( 1 ) ) ), content )


        


        return resolve_imports( text )





    def load_raw( self, name ):


        return text_file( f"{self.path}/{name}.txt" ).content


# ... (intermediate methods remain same)


    def create_file( self, file_path, data ):


        return  text_file( file_path ).write( self.render( data, file_path ) )


