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
#   File:   tools/cpp/cpp_lib/cpp_verifier.py
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-22 19:12:03
#

import re
from lib.base_verifier import base_verifier, run_verifier, rule, rule_group
from cpp_lib.cpp_config import project_cpp_config


include_sep = r"\1\n\n\n\2"
bracket_ignore = r"//.*|/\*[\s\S]*?\*/|\"(?:\\.|[^\"\\])*\"|'(?:\\.|[^'\\])*'"
bracket_fix = "fixed bracket spacing ( ( space ) and [ space ] rules )"


class formatter( base_verifier ):
    def _get_rules( self ):
        rules = super( )._get_rules( )
        nl3 = rules[ "newline_3" ]
        return  rules | {
             "consecutive_newlines": rule( r"\n{4,}", nl3, "too many consecutive empty lines (maximum 2 allowed)" )
            ,"return_spacing": rule( r"([ \t])return\b(?![ \t]*;)[ \t]*", r"\1return\1", "return must be followed by exactly one space or tab (matching the preceding indentation character)" )
            ,"include_no_empty": rule( r"(^#include\s+.*)\n(?:[ \t]*\n)+(?=#include\s+.*)", r"\1\n", "include directives must not be separated by empty lines", flags = re.MULTILINE )
            ,"include_before": rule( r"^((?!#include).+)\n+(#include\s+.*)", r"\1" + nl3 + r"\2", "there must be exactly two empty lines before the first include", flags = re.MULTILINE )
            ,"include_after": rule( r"(^#include\s+.*)\n+((?!#include).+)", r"\1" + nl3 + r"\2", "there must be exactly two empty lines after the last include", flags = re.MULTILINE )
            ,"brackets": rule_group(
                 rules = [
                     rule( r"\((?![ \t\n\)])", r"( ", "missing space after '('" )
                    ,rule( r"(?<![ \t\n\(])\)", r" )", "missing space before ')'" )
                    ,rule( r"\[(?![ \t\n\]])", r"[ ", "missing space after '['" )
                    ,rule( r"(?<![ \t\n\[])\]", r" ]", "missing space before ']'" )
                ]
                ,ignore_pattern = bracket_ignore
                ,summary_message = bracket_fix
            )
        }

    def _get_comment_string( self ):
        return  project_cpp_config[ "language" ][ "comment_string" ]

    def run( self ):
        return  super( ).run( )


def run_cpp_verifier( params: dict ) -> str:
    return  run_verifier( params, formatter, [ "hpp", "cpp" ], "cpp and hpp" )


