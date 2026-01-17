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
#   File:   tools/cpp/code_verifier.py
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-06 14:06:09


import re
import os
from lib.common import run_mcp_tool, ensure, get_path_parts, write_file, read_file
from cpp_lib.config import default_cpp_config
from lib import metadata_provider
from lib import template_engine


class formatter:
    def __init__( self, content: str, file_path: str = None, flg_auto_fix: bool = True ):
        self.content = content
        self.file_path = file_path
        self.messages = []
        self.flg_auto_fix = flg_auto_fix

    def _strip_project_prefix( self, path ):
        """c++ specific prefix stripping for license header."""
        paths = default_cpp_config[ "paths" ]
        prefixes = [ f"{paths['include']}/", f"{paths['source']}/", f"{paths['tests']}/" ]
        for p in prefixes:
            if path.startswith( p ):
                return path[ len( p ) : ]
        return path

    def run( self ):
        self._validate_license( )
        self._consecutive_newlines( )
        self._return_spacing( )
        self._trailing_newlines( )
        self._include_spacing( )
        self._bracket_spacing( )
        return self.content

    def verify( self ):
        self.flg_auto_fix = False
        self.run( )
        return self.messages

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

        data = metadata_provider.get_canonical_metadata( self.file_path ) | {
            "des_file_path": self._strip_project_prefix( self.file_path )
        }
        ideal_header = template_engine.render( "file-header", data ).strip( )
        
        parts = self.content.split( "\n\n", 1 )
        actual_header = parts[ 0 ].strip( )
        
        if actual_header != ideal_header:
            if self.flg_auto_fix:
                body = parts[ 1 ] if len( parts ) > 1 else ""
                self.content = ideal_header + "\n\n" + body
                self.messages.append( f"restored canonical license header for {self.file_path}" )
            else:
                self.messages.append( ( 1, f"license header mismatch in {self.file_path}" ) )

    def _consecutive_newlines( self ):
        self._apply( 
            r"\n{4,}", 
            "\n\n\n", 
            "too many consecutive empty lines (maximum 2 allowed)" 
        )

    def _return_spacing( self ):
        self._apply( 
            r"([ \t])return\b(?![ \t]*;)[ \t]*", 
            r"\1return\1", 
            "return must be followed by exactly one space or tab (matching the preceding indentation character)" 
        )

    def _trailing_newlines( self ):
        new_content = self.content.rstrip( ) + "\n\n\n"
        if new_content != self.content:
            if self.flg_auto_fix:
                self.content = new_content
                self.messages.append( "file must end with exactly 2 empty lines and no trailing whitespace" )
            else:
                line_no = self.content.count( "\n" ) + 1
                self.messages.append( ( line_no, "file must end with exactly 2 empty lines and no trailing whitespace" ) )

    def _include_spacing( self ):
        self._apply( 
            r"(^#include\s+.*)\n(?:[ \t]*\n)+(?=#include\s+.*)", 
            r"\1\n", 
            "include directives must not be separated by empty lines" ,
            flags = re.MULTILINE 
        )

        self._apply(
            r"^((?!#include).+)\n+(#include\s+.*)",
            r"\1\n\n\n\2",
            "there must be exactly two empty lines before the first include",
            flags = re.MULTILINE
        )

        self._apply(
            r"(^#include\s+.*)\n+((?!#include).+)",
            r"\1\n\n\n\2",
            "there must be exactly two empty lines after the last include",
            flags = re.MULTILINE
        )

    def _bracket_spacing( self ):
        split_index = self.content.find( "\n\n" )
        if split_index == -1:
            return

        header = self.content[ :split_index + 2 ]
        body = self.content[ split_index + 2 : ]

        ignore_pattern = r"//.*|/\*[\s\S]*?\*/|\"(?:\\.|[^\"\\])*\"|'(?:\\.|[^'\\])*'"
        
        patterns = [
             ( r"\((?![ \t\n\)])", r"( ", "missing space after '('" )
            ,( r"(?<![ \t\n\(])\)", r" )", "missing space before ')'" )
            ,( r"\[(?![ \t\n\]])", r"[ ", "missing space after '['" )
            ,( r"(?<![ \t\n\[])\]", r" ]", "missing space before ']" )
        ]

        original_body = body
        for pattern, replacement, message in patterns:
            combined = f"({ignore_pattern})|({pattern})"
            if self.flg_auto_fix:
                def sub_func( m ):
                    if m.group( 1 ): return m.group( 1 )
                    return replacement
                body = re.sub( combined, sub_func, body )
            else:
                for match in re.finditer( combined, body ):
                    if match.group( 1 ): continue
                    line_no = header.count( "\n" ) + original_body.count( "\n", 0, match.start( ) ) + 1
                    self.messages.append( ( line_no, message ) )

        if self.flg_auto_fix and body != original_body:
            self.content = header + body
            self.messages.append( "fixed bracket spacing ( ( space ) and [ space ] rules )" )


def run_code_verifier( params: dict ) -> str:
    files = params.get( "files", [ ] )
    flg_auto_fix = params.get( "flg_auto_fix", False )
    
    results = [ ]
    for file_path in files:
        ensure( get_path_parts( file_path )[ "extension" ] in [ "hpp", "cpp" ], "this tool is exclusively for cpp and hpp files" )
        content = read_file( file_path )
        
        fmt = formatter( content, file_path = file_path, flg_auto_fix = flg_auto_fix )
        if flg_auto_fix:
            new_content = fmt.run( )
            if content != new_content:
                write_file( file_path, new_content )
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
    
    return "\n".join( results ).strip( ) or f"no formatting violations found in the provided files."



if __name__ == "__main__":
    run_mcp_tool( run_code_verifier )
