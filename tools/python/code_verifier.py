#!/usr/bin/python3


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
#   You should have received a copy of the GNU General License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#   
#   
#   File:   tools/python/code_verifier.py
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-18 14:00:00


import re
import os
from lib.common import run_mcp_tool, ensure
from lib.fso import text_file
from lib import file_info


class formatter:
    def __init__( self, content: str, file_path: str = None, flg_auto_fix: bool = True ):
        self.content = content
        self.file_path = file_path
        self.messages = []
        self.flg_auto_fix = flg_auto_fix

    def run( self ):
        self._trailing_newlines( )
        self._return_spacing( )
        return  self.content

    def verify( self ):
        self.flg_auto_fix = False
        self.run( )
        return  self.messages

    def _return_spacing( self ):
        pattern = r'^(\s*return) +(\S)'
        replacement = r'\1  \2'
        
        if self.flg_auto_fix:
            new_content = re.sub( pattern, replacement, self.content, flags = re.MULTILINE )
            if new_content != self.content:
                self.content = new_content
                self.messages.append( "return must be followed by exactly 2 spaces" )
        else:
            for i, line in enumerate( self.content.splitlines( ) ):
                if re.match( r'^\s*return +\S', line ) and not re.match( r'^\s*return  \S', line ):
                    self.messages.append( ( i + 1, "return must be followed by exactly 2 spaces" ) )

    def _trailing_newlines( self ):
        new_content = self.content.rstrip( ) + "\n\n\n"
        if new_content != self.content:
            if self.flg_auto_fix:
                self.content = new_content
                self.messages.append( "file must end with exactly 2 empty lines and no trailing whitespace" )
            else:
                line_no = self.content.count( "\n" ) + 1
                self.messages.append( ( line_no, "file must end with exactly 2 empty lines and no trailing whitespace" ) )


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


