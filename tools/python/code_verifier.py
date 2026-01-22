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
#   File:   tools/python/code_verifier.py
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-18 16:21:38
#


import re
import os
from lib.common import run_mcp_tool, ensure
from lib.fso import text_file
from lib import file_info
from lib.template import template


newline_3 = "\n\n\n"
trailing_msg = "file must end with exactly 2 empty lines and no trailing whitespace"


class formatter:
    m_rules = {
         "return_spacing": ( r"^(\s*return) +(\S)", r"\1  \2", "return must be followed by exactly 2 spaces" )
        ,"trailing_msg": trailing_msg
        ,"newline_3": newline_3
    }

    def __init__( self, content: str, file_path: str = None, flg_auto_fix: bool = True ):
        self.content = content
        self.file_path = file_path
        self.messages = []
        self.flg_auto_fix = flg_auto_fix

    def run( self ):
        self._validate_license( )
        self._trailing_newlines( )
        self._return_spacing( )
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
        
        #   check for shebang
        shebang = ""
        content_to_check = self.content
        if self.content.startswith( "#!" ):
            parts = self.content.split( "\n", 1 )
            shebang = parts[ 0 ]
            content_to_check = parts[ 1 ] if len( parts ) > 1 else ""

        #   ideal header
        model = template( "file-header", comment_string = "#   " )
        ideal_header = model.render( file_info.get_info( self.file_path ) ).strip( " \n\r" )
        
        #   extract actual header
        parts = content_to_check.lstrip( "\n" ).split( "\n\n", 1 )
        first_block = parts[ 0 ]
        body = parts[ 1 ] if len( parts ) > 1 else ""

        #   check if the first block is actually a header (only comments)
        lines = first_block.strip( ).split( "\n" )
        is_header = all( line.strip( ).startswith( "#" ) for line in lines ) if lines else False

        if is_header:
            actual_header = first_block.strip( " \n\r" )
            content_to_restore = body.lstrip( "\n" )
        else:
            actual_header = ""
            content_to_restore = content_to_check.lstrip( "\n" )
        
        if actual_header != ideal_header:
            if self.flg_auto_fix:
                new_content = ""
                if shebang:
                    new_content = shebang + self.m_rules[ "newline_3" ]
                
                new_content += ideal_header + self.m_rules[ "newline_3" ] + content_to_restore
                self.content = new_content
                self.messages.append( f"restored canonical license header for {self.file_path}" )
            else:
                self.messages.append( ( 1, f"license header mismatch in {self.file_path}" ) )

    def _return_spacing( self ):
        rule = self.m_rules[ "return_spacing" ]
        self._apply( rule[ 0 ], rule[ 1 ], rule[ 2 ], flags = re.MULTILINE )

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


def run_code_verifier( params: dict ) -> str:
    files = params.get( "files", [ ] )
    flg_auto_fix = params.get( "flg_auto_fix", False )
    
    results = [ ]
    for file_path in files:
        f = text_file( file_path )
        ensure( f.exists, f"file not found: {file_path}" )
        ensure( f.extension == "py", "this tool is exclusively for python files" )
        
        fmt = formatter( f.content, file_path = f.path, flg_auto_fix = flg_auto_fix )
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
        res += "\n\nthe files were adjusted automatically, no action necessary"

    return  res or f"no formatting violations found in the provided files"



if __name__ == "__main__":
    run_mcp_tool( run_code_verifier )


