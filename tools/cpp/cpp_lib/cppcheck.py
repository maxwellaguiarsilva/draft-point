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
#   File:   tools/cpp/cpp_lib/cppcheck.py
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-24 22:08:36
#


class cppcheck:
    def __init__( self, config ):
        self.config = config

    def get_command( self, paths ):
        params = self._get_params( )
        paths_str = " ".join( f'"{p}"' for p in paths )
        return  f"cppcheck {params} {paths_str}"

    def _get_params( self ):
        config = self.config
        analysis_config = config[ 'quality_control' ][ 'static_analysis' ]
        
        params = [
            "--quiet"
            ,"--enable=all"
            ,f"--cppcheck-build-dir={config[ 'paths' ][ 'build' ]}"
            ,"--inline-suppr"
            ,f"--std={config[ 'compiler' ][ 'standard' ]}"
            ,"--error-exitcode=1"
            ,f"-j {config[ 'max_threads' ]}"
        ]
        
        params.append( f"--check-level={analysis_config[ 'strictness' ]}" )
            
        for suppression in analysis_config.get( 'suppressions', [ ] ):
            params.append( f"--suppress={suppression}" )

        params.append( f"-I{config[ 'paths' ][ 'include' ]}" )
        for d in config[ 'dependencies' ][ 'include_dirs' ]:
            params.append( f"-I{d}" )
            
        return  " ".join( params )


