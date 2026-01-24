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
#   File:   tools/python/python_lib/python_verifier.py
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-22 19:00:22
#

import re
from lib.base_verifier import base_verifier, run_verifier, rule
from python_lib.python_config import project_python_config


class formatter( base_verifier ):
    def _get_shebang( self ):
        return  "#!/usr/bin/python3"

    def _get_comment_string( self ):
        return  project_python_config[ "language" ][ "comment_string" ]

    def _get_rules( self ):
        return  super( )._get_rules( ) | {
            "return_spacing": rule( r"^(\s*return) +(\S)", r"\1  \2", "return must be followed by exactly 2 spaces", flags = re.MULTILINE )
        }

    def run( self ):
        return  super( ).run( )


def run_python_verifier( params: dict ) -> str:
    return  run_verifier( params, formatter, "py", "python" )


