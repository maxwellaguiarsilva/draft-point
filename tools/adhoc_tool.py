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
#   File:   adhoc_tool
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-04 20:18:54


from lib.common import run_main


def run_adhoc_tool( params ):
    """executes experimental logic defined in tools/adhoc-tool.py
    this tool is used for prototyping new functionalities
    the 'params' dictionary is passed to the script
    after the logic is validated and moved to a production script,
    this function must be cleaned up to maintain a minimal state
    """
    action = params.get( "action" )
    
    if action == "hello":
        return f"hello, {params.get( 'name', 'world' )}!"

    raise Exception( "no valid action or parameters provided to adhoc_tool" )


if __name__ == "__main__":
    run_main( run_adhoc_tool )


