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
#   File:   tools/git/quick_upload.py
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-16 13:59:50
#


from lib.common import run_mcp_tool, ensure, create_process
from llm.statistic import run_statistic


def run_quick_upload( message: str ) -> str:
    """performs a quick git upload: pull, add all, commit with message, and push
this tool is intended for simple, non-conflicting changes to increase agility"""
    create_process( [ "git", "pull" ] )
    create_process( [ "git", "add", "." ] )
    create_process( [ "git", "commit", "-m", message ] )
    create_process( [ "git", "push" ] )
    
    stats_result = run_statistic( name="success" )
    
    return  f"`{stats_result}`\n\n{message}"


if __name__ == "__main__":
    run_mcp_tool( run_quick_upload )


