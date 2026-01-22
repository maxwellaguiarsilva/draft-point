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
#   File:   tools/lib/verifier.py
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-22 13:50:05
#

import re
from lib.common import ensure, validate_params
from lib.fso import text_file
from lib import file_info
from lib.template import template



class base_verifier:
    def __init__( self, content: str, file_path: str = None, flg_auto_fix: bool = True ):
        self.content = content
        self.file_path = file_path
        self.messages = [ ]
        self.flg_auto_fix = flg_auto_fix
        self.m_rules = self._get_rules( )

    def _get_rules( self ):
        return  {
             "newline_2": "\n\n"
            ,"newline_3": "\n\n\n"
            ,"trailing_msg": "file must end with exactly 2 empty lines and no trailing whitespace"
        }

    def _get_comment_string( self ):
        return  "#   "

    def _get_shebang( self ):
        return  ""

    def run( self ):
        self._validate_license( )
        self._trailing_newlines( )
        return  self.content

    def verify( self ):
        self.flg_auto_fix = False
        self.run( )
        return  self.messages

    def _apply( self, pattern: str, replacement: str, message: str, flags: int = 0 ):
        if self.flg_auto_fix:
            new_content = re.sub( pattern, replacement, self.content, flags = flags )
            if new_content != self.content:
                self.content = new_content
                self.messages.append( message )
        else:
            for match in re.finditer( pattern, self.content, flags = flags ):
                old_text = match.group( 0 )
                new_text = re.sub( pattern, replacement, old_text, flags = flags )
                if old_text != new_text:
                    line_no = self.content.count( "\n", 0, match.start( ) ) + 1
                    self.messages.append( ( line_no, message ) )

    def _validate_license( self ):
        if not self.file_path:
            return
        
        comment_string = self._get_comment_string( )
        model   =   template( "file-header" )
        ideal_header = model.render( file_info.get_info( self.file_path ) | { "comment_string": comment_string } ).strip( " \n\r" )
        
        shebang, body = file_info.strip_header( self.content, comment_string )
        if not shebang:
            shebang = self._get_shebang( )
        shebang = shebang.strip( )
        
        sep = self.m_rules[ "newline_2" ]
        new_content = shebang + ( sep if shebang else "" ) + ideal_header + sep + body.lstrip( "\n" )
        
        if self.content.strip( " \n\r" ) != new_content.strip( " \n\r" ):
            if self.flg_auto_fix:
                self.content = new_content
                self.messages.append( f"restored canonical license header for {self.file_path}" )
            else:
                self.messages.append( ( 1, f"license header mismatch in {self.file_path}" ) )

    def _trailing_newlines( self ):
        new_content = self.content.rstrip( ) + self.m_rules[ "newline_3" ]
        if new_content != self.content:
            msg = self.m_rules[ "trailing_msg" ]
            if self.flg_auto_fix:
                self.content = new_content
                self.messages.append( msg )
            else:
                line_no = self.content.count( "\n" ) + 1
                self.messages.append( ( line_no, msg ) )


def run_verifier( params: dict, verifier_class, allowed_extensions, language_name ) -> str:
    validate_params( params, required = [ "files" ], optional = [ "flg_auto_fix" ] )
    files = params.get( "files", [ ] )
    flg_auto_fix = params.get( "flg_auto_fix", False )
    
    results = [ ]
    for file_path in files:
        f = text_file( file_path )
        ensure( f.exists, f"file not found: {file_path}" )
        
        ext_msg = f"this tool is exclusively for {language_name} files"
        if isinstance( allowed_extensions, list ):
            ensure( f.extension in allowed_extensions, ext_msg )
        else:
            ensure( f.extension == allowed_extensions, ext_msg )
        
        fmt = verifier_class( f.content, file_path = f.path, flg_auto_fix = flg_auto_fix )
        if flg_auto_fix:
            new_content = fmt.run( )
            if f.content != new_content:
                f.write( new_content )
        else:
            fmt.verify( )
        
        if fmt.messages:
            message = f"file: {file_path}\n"
            for violation in fmt.messages:
                if isinstance( violation, ( list, tuple ) ):
                    line, text = violation
                    message += f"  line {line}: {text}\n"
                else:
                    message += f"  {violation}\n"
            results.append( message )
    
    res = "\n".join( results ).strip( )
    if res and flg_auto_fix:
        res += f"\n\nthe files were adjusted automatically, no action necessary"

    return  res or f"no formatting violations found in the provided files."


