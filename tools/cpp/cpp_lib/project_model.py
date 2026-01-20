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
#   File:   project_model
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-20 16:23:01


from lib.fso import text_file
import re


class project_file( text_file ):
    include_regex   =   re.compile( r'#include\s*(?P<full>(?P<open>[<"])(?P<path>[^>"]+)(?P<close>[>"]))' )

    def __init__( self, file_path ):
        super( ).__init__( file_path )
        self.includes   =   set( )
    
    def refresh( self ):
        super( ).refresh( )


class hpp( project_file ):
    def __init__( self, path ):
        super( ).__init__( path )


class cpp( project_file ):
    def __init__( self, path ):
        super( ).__init__( path )


class project_model:

    def __init__( self, config ):
        self.config =   config
        self.files  =   { }
        
        self.scan_dir( [ 
             self.config[ "paths" ][ "include" ]
            ,self.config[ "paths" ][ "source" ]
            ,self.config[ "paths" ][ "tests" ]
        ] )

    def scan_dir( self, dir_path ):
        if isinstance( dir_path, list ):
            for path in dir_path:
                self._scan_dir( path )
            return

        source_ext = self.config[ "language" ][ "source_extension" ]
        header_ext = self.config[ "language" ][ "header_extension" ]

        exts = ( f".{header_ext}", f".{source_ext}" )
        pattern = os.path.join( dir_path, "**", "*" )
        for path in glob.glob( pattern, recursive = True ):
            if os.path.isfile( path ) and path.endswith( exts ):
                self.files[ path ]  =   cpp( path ) if path.endswith( source_ext ) else hpp( path )


