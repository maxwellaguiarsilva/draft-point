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
#   File:   tools/cpp/cpp_lib/verifier.py
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-22 13:50:05
#

import re
from lib.verifier import base_verifier, run_verifier
from cpp_lib.config import default_cpp_config


newline_3 = "\n\n\n"
include_sep = r"\1\n\n\n\2"
bracket_ignore = r"//.*|/\*[\s\S]*?\*/|\"(?:\\.|[^\"\\])*\"|'(?:\\.|[^'\\])*'"
bracket_fix = "fixed bracket spacing ( ( space ) and [ space ] rules )"


class formatter( base_verifier ):
    def _get_rules( self ):
        return  super( )._get_rules( ) | {
             "consecutive_newlines": ( r"\n{4,}", newline_3, "too many consecutive empty lines (maximum 2 allowed)" )
            ,"return_spacing": ( r"([ \t])return\b(?![ \t]*;)[ \t]*", r"\1return\1", "return must be followed by exactly one space or tab (matching the preceding indentation character)" )
            ,"include_no_empty": ( r"(^#include\s+.*)\n(?:[ \t]*\n)+(?=#include\s+.*)", r"\1\n", "include directives must not be separated by empty lines" )
            ,"include_before": ( r"^((?!#include).+)\n+(#include\s+.*)", include_sep, "there must be exactly two empty lines before the first include" )
            ,"include_after": ( r"(^#include\s+.*)\n+((?!#include).+)", include_sep, "there must be exactly two empty lines after the last include" )
            ,"bracket_ignore": bracket_ignore
            ,"bracket_fix": bracket_fix
            ,"brackets": [
                 ( r"\((?![ \t\n\)])", r"( ", "missing space after '('" )
                ,( r"(?<![ \t\n\(])\)", r" )", "missing space before ')'" )
                ,( r"\[(?![ \t\n\]])", r"[ ", "missing space after '['" )
                ,( r"(?<![ \t\n\[])\]", r" ]", "missing space before ']'" )
            ]
        }

    def _get_comment_string( self ):
        return  default_cpp_config[ "language" ][ "comment_string" ]

    def run( self ):
        super( ).run( )
        self._consecutive_newlines( )
        self._return_spacing( )
        self._include_spacing( )
        self._bracket_spacing( )
        return  self.content

    def _consecutive_newlines( self ):
        rule = self.m_rules[ "consecutive_newlines" ]
        self._apply( rule[ 0 ], rule[ 1 ], rule[ 2 ] )

    def _return_spacing( self ):
        rule = self.m_rules[ "return_spacing" ]
        self._apply( rule[ 0 ], rule[ 1 ], rule[ 2 ] )

    def _include_spacing( self ):
        for name in [ "include_no_empty", "include_before", "include_after" ]:
            rule = self.m_rules[ name ]
            self._apply( rule[ 0 ], rule[ 1 ], rule[ 2 ], flags = re.MULTILINE )

    def _bracket_spacing( self ):
        split_index = self.content.find( "\n\n\n" )
        if split_index == -1:
            return

        header = self.content[ :split_index + 3 ]
        body = self.content[ split_index + 3 : ]

        ignore_pattern = self.m_rules[ "bracket_ignore" ]
        patterns = self.m_rules[ "brackets" ]

        original_body = body
        for pattern, replacement, message in patterns:
            combined = f"({ignore_pattern})|({pattern})"
            if self.flg_auto_fix:
                def sub_func( m ):
                    if m.group( 1 ): return m.group( 1 )
                    return  replacement
                body = re.sub( combined, sub_func, body )
            else:
                for match in re.finditer( combined, body ):
                    if match.group( 1 ): continue
                    line_no = header.count( "\n" ) + original_body.count( "\n", 0, match.start( ) ) + 1
                    self.messages.append( ( line_no, message ) )

        if self.flg_auto_fix and body != original_body:
            self.content = header + body
            self.messages.append( self.m_rules[ "bracket_fix" ] )


def run_cpp_verifier( params: dict ) -> str:
    return  run_verifier( params, formatter, [ "hpp", "cpp" ], "cpp and hpp" )


