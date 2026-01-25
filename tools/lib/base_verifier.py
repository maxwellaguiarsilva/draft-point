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
#   File:   tools/lib/base_verifier.py
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-22 19:12:03
#


import re
from dataclasses import dataclass, field
from lib.common import ensure, validate_params, ensure_list 
from lib.fso import text_file
from lib.project_file import generic_project_file
from lib.template import template, get_metadata


def strip_header( content, comment_string ):
    lines = content.splitlines( keepends = True )
    shebang = ""
    start_index = 0
    if lines and lines[ 0 ].startswith( "#!" ):
        shebang = lines[ 0 ]
        start_index = 1
    
    comment_prefix = comment_string.rstrip( )
    body_start = start_index
    for i in range( start_index, len( lines ) ):
        line = lines[ i ]
        if not line.strip( ) or ( comment_prefix and line.startswith( comment_prefix ) ):
            body_start = i + 1
        else:
            break
            
    return  shebang, "".join( lines[ body_start: ] )


@dataclass
class rule:
    pattern: str
    replacement: str
    message: str
    ignore_pattern: str = None
    flags: int = 0

    def __post_init__( self ):
        re.compile( self.pattern, flags = self.flags )
        if self.ignore_pattern:
            re.compile( self.ignore_pattern, flags = self.flags )


class base_verifier:
    config = {
         "newline_2": "\n\n"
        ,"newline_3": "\n\n\n"
        ,"trailing_msg": "file must end with exactly 2 empty lines and no trailing whitespace"
    }

    def __init__( self, content: str, file_path: str = None ):
        self.content = content
        self.file_path = file_path
        self.messages = [ ]
        self.m_rules = self._get_rules( )

    def _get_rules( self ):
        return  { }

    @property
    def comment_string( self ) -> str:
        return  "#   "

    @property
    def shebang_string( self ) -> str:
        return  ""

    def run( self ):
        header, body = self._restore_header( )
        header_lines = header.count( "\n" )
        body = self._run_body_rules( body, header_lines )
        self.content = header + body
        return  self.content

    def _restore_header( self ):
        if not self.file_path:
            return  "", self.content

        comment_string = self.comment_string
        shebang_string = self.shebang_string
        model = template( "file-header" )
        item  = generic_project_file( self.file_path, shebang = shebang_string, comment_string = comment_string )
        ideal_header = model.render( get_metadata( item ) | { "comment_string": comment_string } ).strip( " \n\r" )
        
        shebang, body = strip_header( self.content, comment_string )
        if not shebang:
            shebang = shebang_string
        shebang = shebang.strip( )
        
        sep_shebang = self.config[ "newline_2" ]
        sep_body    = self.config[ "newline_3" ]
        
        header = shebang + ( sep_shebang if shebang else "" ) + ideal_header + sep_body
        
        #   check if header changed significantly ( ignoring exact whitespace in body transition )
        old_split_index = self.content.find( sep_body )
        if old_split_index == -1:
            self.messages.append( f"line 1: restored canonical license header for {self.file_path}" )
        else:
            old_header = self.content[ :old_split_index + len( sep_body ) ]
            if old_header.strip( " \n\r" ) != header.strip( " \n\r" ):
                self.messages.append( f"line 1: restored canonical license header for {self.file_path}" )

        return  header, body.lstrip( "\n" )

    def _run_body_rules( self, body, header_lines ):
        body = self._trailing_newlines( body, header_lines )
        body = self.run_rules( body, header_lines )
        return  body

    def verify( self ):
        self.run( )
        return  self.messages

    def run_rules( self, body, line_offset ):
        for name, r in self.m_rules.items( ):
            body = self._apply_rule( body, r, line_offset )
        
        return  body

    def _apply_rule( self, body, rule_obj: rule, line_offset: int = 0 ):
        pattern = rule_obj.pattern
        if rule_obj.ignore_pattern:
            pattern = f"({rule_obj.ignore_pattern})|({rule_obj.pattern})"

        def sub_func( match ):
            if rule_obj.ignore_pattern and match.group( 1 ):
                return  match.group( 0 )
            
            new_text = match.expand( rule_obj.replacement )
            if match.group( 0 ) != new_text:
                line_no = line_offset + body.count( "\n", 0, match.start( ) ) + 1
                self.messages.append( f"line {line_no}: {rule_obj.message}" )
            return  new_text
        
        return  re.sub( pattern, sub_func, body, flags = rule_obj.flags )


    def _trailing_newlines( self, body, line_offset ):
        new_body = body.rstrip( ) + self.config[ "newline_3" ]
        if new_body != body:
            line_no = line_offset + body.count( "\n" ) + 1
            self.messages.append( f"line {line_no}: {self.config[ 'trailing_msg' ]}" )
        return  new_body



def run_verifier( params: dict, verifier_class, allowed_extensions, language_name ) -> str:
    validate_params( params, required = [ "files" ], optional = [ "flg_auto_fix" ] )
    files = params.get( "files", [ ] )
    flg_auto_fix = params.get( "flg_auto_fix", False )
    
    results = [ ]
    for file_path in files:
        f = text_file( file_path )
        
        ensure( f.exists, f"file not found: {file_path}" )
        ensure( f.extension in ensure_list( allowed_extensions, str ), f"this tool is exclusively for {language_name} files" )
        
        fmt = verifier_class( f.content, file_path = f.path )
        new_content = fmt.run( )
        
        if flg_auto_fix and f.content != new_content:
            f.write( new_content )
        
        if fmt.messages:
            message = f"file: {file_path}\n"
            for violation in fmt.messages:
                message += f"  {violation}\n"
            results.append( message )
    
    res = "\n".join( results ).strip( )
    if res and flg_auto_fix:
        res += f"\n\nthe files were adjusted automatically, no action necessary"

    return  res or f"no formatting violations found in the provided files."


