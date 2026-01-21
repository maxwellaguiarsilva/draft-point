#!/usr/bin/python3


#   Copyright (C)   2025 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
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
#   File:   tools/cpp/compile.py
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-14 18:00:00


import datetime
import os
import concurrent.futures
from lib.common import run_mcp_tool, ensure, get_cpu_count
from cpp_lib.project_core import project_core


def run_compile( params ):
    start_time = datetime.datetime.now( )
    core = project_core( params.get( "config", { } ) )

    core.print( f"build started at: {start_time.strftime( '%Y-%m-%d %H:%M:%S' )}" )

    #   1. collect all unique cpp files to build
    all_cpps = { }
    for b in core.binary_list:
        for c in b.dependencies_list:
            all_cpps[ c.path ] = c

    #   2. ensure build directories exist
    for c in all_cpps.values( ):
        os.makedirs( os.path.dirname( c.object.path ), exist_ok = True )

    #   3. parallel compilation
    max_workers = core.config[ "build_behavior" ].get( "max_threads", get_cpu_count( ) )
    core.print( f"\ncompiling {len(all_cpps)} files using {max_workers} threads..." )
    
    with concurrent.futures.ThreadPoolExecutor( max_workers = max_workers ) as executor:
        futures = [ executor.submit( core.build, c ) for c in all_cpps.values( ) ]
        try:
            for future in concurrent.futures.as_completed( futures ):
                future.result( )
        except Exception as e:
            core.stop( )
            raise e

    #   4. parallel linking
    core.print( f"\nlinking {len(core.binary_list)} binaries using {max_workers} threads..." )
    with concurrent.futures.ThreadPoolExecutor( max_workers = max_workers ) as executor:
        futures = [ executor.submit( b.link ) for b in core.binary_list ]
        try:
            for future in concurrent.futures.as_completed( futures ):
                future.result( )
        except Exception as e:
            core.stop( )
            raise e

    end_time = datetime.datetime.now( )
    elapsed_time = end_time - start_time
    core.print( f"\nbuild ended at: {end_time.strftime( '%Y-%m-%d %H:%M:%S' )}" )
    core.print( f"elapsed time: {elapsed_time}" )
    
    return  "compilation completed successfully"


if __name__ == "__main__":
    run_mcp_tool( run_compile )


