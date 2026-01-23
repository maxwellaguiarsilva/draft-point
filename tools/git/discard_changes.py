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
#   File:   tools/git/discard_changes.py
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-16 22:34:01
#

from lib.common import run_mcp_tool, ensure, create_process
from llm.statistic import run_statistic


def run_discard_changes( ) -> str:
    """discards all uncommitted changes and removes untracked files
this tool reverts the repository to the state of the last commit (head)"""
    create_process( [ "git", "reset", "--hard", "HEAD" ] )
    create_process( [ "git", "clean", "-fd" ] )
    
    stats_result = run_statistic( "success" ) 
    
    return  f"`{stats_result}`\n\nall uncommitted changes have been discarded and untracked files removed"


if __name__ == "__main__":
    run_mcp_tool( run_discard_changes )


