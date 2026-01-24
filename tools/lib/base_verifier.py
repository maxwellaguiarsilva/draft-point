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
from lib.common import ensure, validate_params
from lib.fso import text_file
from lib import file_info
from lib.template import template



@dataclass
class rule:
    pattern: str
    replacement: str
    message: str
    flags: int = 0
    is_exclusive: bool = False
    description: str = ""

    def __post_init__( self ):
        re.compile( self.pattern, flags = self.flags )


class base_verifier:
    def __init__( self, content: str, file_path: str = None ):
        self.content = content
        self.file_path = file_path
        self.messages = [ ]
        self.m_rules = self._get_rules( )

    @property
    def common_rules( self ):
        return  {
             "newline_2": "\n\n"
            ,"newline_3": "\n\n\n"
        }

    def _get_rules( self ):
        return  self.common_rules | {
            "trailing_msg": "file must end with exactly 2 empty lines and no trailing whitespace"
        }

    def _get_comment_string( self ):
        return  "#   "

    def _get_shebang( self ):
        return  ""

    def run( self ):
        self._validate_license( )
        self._trailing_newlines( )
        self.run_rules( )
        return  self.content

    def verify( self ):
        self.run( )
        return  self.messages

    def run_rules( self ):
        ignore_pattern = self.m_rules.get( "bracket_ignore" )
        fix_message = self.m_rules.get( "bracket_fix" )

        for name, item in self.m_rules.items( ):
            if not isinstance( item, ( rule, list ) ):
                continue

            if isinstance( item, list ):
                if ignore_pattern:
                    self._apply_with_exclusion( item, ignore_pattern, fix_message )
                else:
                    for r in item:
                        self._apply( r )
            elif item.is_exclusive and ignore_pattern:
                self._apply_with_exclusion( [ item ], ignore_pattern, fix_message )
            else:
                self._apply( item )

    def _apply( self, rule_obj: rule ):
        original_content = self.content
        def sub_func( match ):
            new_text = match.expand( rule_obj.replacement )
            if match.group( 0 ) != new_text:
                line_no = original_content.count( "\n", 0, match.start( ) ) + 1
                self.messages.append( f"line {line_no}: {rule_obj.message}" )
            return  new_text
        self.content = re.sub( rule_obj.pattern, sub_func, self.content, flags = rule_obj.flags )

    def _apply_with_exclusion( self, rules: list[ rule ], ignore_pattern: str, fix_message: str = None ):
        split_index = self.content.find( "\n\n\n" )
        if split_index == -1:
            return

        header = self.content[ :split_index + 3 ]
        body = self.content[ split_index + 3 : ]
        header_lines = header.count( "\n" )
        
        original_body = body
        modified_body = body
        any_actual_change = False
        first_line = None

        for rule_obj in rules:
            combined = f"({ignore_pattern})|({rule_obj.pattern})"
            current_body = modified_body

            def sub_func( m ):
                nonlocal any_actual_change, first_line
                if m.group( 1 ): return m.group( 0 )

                line_no = header_lines + current_body.count( "\n", 0, m.start( ) ) + 1
                if first_line is None: first_line = line_no
                self.messages.append( f"line {line_no}: {rule_obj.message}" )
                any_actual_change = True
                return  rule_obj.replacement

            modified_body = re.sub( combined, sub_func, modified_body, flags = rule_obj.flags )

        if any_actual_change:
            self.content = header + modified_body
            if fix_message:
                self.messages.append( f"line {first_line}: {fix_message}" )

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
        
        sep_shebang = self.m_rules[ "newline_2" ]
        sep_body    = self.m_rules[ "newline_3" ]
        
        new_content = shebang + ( sep_shebang if shebang else "" ) + ideal_header + sep_body + body.lstrip( "\n" )
        
        if self.content.strip( " \n\r" ) != new_content.strip( " \n\r" ):
            self.content = new_content
            self.messages.append( f"line 1: restored canonical license header for {self.file_path}" )

    def _trailing_newlines( self ):
        new_content = self.content.rstrip( ) + self.m_rules[ "newline_3" ]
        if new_content != self.content:
            line_no = self.content.count( "\n" ) + 1
            self.content = new_content
            self.messages.append( f"line {line_no}: {self.m_rules[ 'trailing_msg' ]}" )


def run_verifier( params: dict, verifier_class, allowed_extensions, language_name ) -> str:
    validate_params( params, required = [ "files" ], optional = [ "flg_auto_fix" ] )
    files = params.get( "files", [ ] )
    flg_auto_fix = params.get( "flg_auto_fix", False )
    allowed_extensions = allowed_extensions if isinstance( allowed_extensions, list ) else [ allowed_extensions ]
    
    results = [ ]
    for file_path in files:
        f = text_file( file_path )
        
        ensure( f.exists, f"file not found: {file_path}" )
        ensure( f.extension in allowed_extensions, f"this tool is exclusively for {language_name} files" )
        
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


