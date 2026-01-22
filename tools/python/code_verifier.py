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
from lib.common import run_mcp_tool
from lib.verifier import base_verifier, run_verifier



class formatter( base_verifier ):
    def _get_rules( self ):
        return  super( )._get_rules( ) | {
            "return_spacing": ( r"^(\s*return) +(\S)", r"\1  \2", "return must be followed by exactly 2 spaces" )
        }

    def run( self ):
        super( ).run( )
        self._return_spacing( )
        return  self.content

    def _return_spacing( self ):
        rule = self.m_rules[ "return_spacing" ]
        self._apply( rule[ 0 ], rule[ 1 ], rule[ 2 ], flags = re.MULTILINE )


def run_code_verifier( params: dict ) -> str:
    return  run_verifier( params, formatter, "py", "python" )



if __name__ == "__main__":
    run_mcp_tool( run_code_verifier )


