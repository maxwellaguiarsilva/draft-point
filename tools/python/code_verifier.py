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


from lib.common import run_mcp_tool
from python_lib.python_verifier import run_python_verifier


def run_code_verifier( files: list[ str ], flg_auto_fix: bool = False ) -> str:
    """verifies if a list of python files follows the project's formatting rules
if flg_auto_fix is true, allows the tool to attempt to adjust automatically ( false as default )
returns a consolidated list of violations"""
    return  run_python_verifier( { "files": files, "flg_auto_fix": flg_auto_fix } )


if __name__ == "__main__":
    run_mcp_tool( run_code_verifier )


