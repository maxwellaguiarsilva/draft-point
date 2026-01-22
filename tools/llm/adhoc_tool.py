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
#   File:   tools/llm/adhoc_tool.py
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-16 00:05:41
#

from lib.common import run_mcp_tool, validate_params


def run_adhoc_tool( params: dict ) -> str:
    """executes experimental logic defined in tools/llm/adhoc_tool.py
this tool is used for prototyping new functionalities
the 'params' dictionary is passed to the script"""
    action = params.get( "action" )
    
    if action == "hello":
        return  f"hello, {params.get( 'name', 'world' )}!"

    raise Exception( "no valid action or parameters provided to adhoc_tool" )


if __name__ == "__main__":
    run_mcp_tool( run_adhoc_tool )


