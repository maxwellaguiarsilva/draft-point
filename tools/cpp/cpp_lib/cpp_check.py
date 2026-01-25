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
#   File:   tools/cpp/cpp_lib/cpp_check.py
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-19 17:21:50
#


class cpp_check:
    default_config = {
         "executable": "cppcheck"
        ,"strictness": "exhaustive"
        ,"suppressions": [
             "missingIncludeSystem"
            ,"checkersReport"
        ]
    }

    def __init__( self, config, paths ):
        self.config = config
        self.paths = paths

    @property
    def command( self ):
        paths_str = " ".join( f'"{p}"' for p in self.paths )
        executable = self.default_config[ 'executable' ]
        return  f"{executable} {self.params} {paths_str}"

    @property
    def params( self ):
        config = self.config
        
        params = [
            "--quiet"
            ,"--enable=all"
            ,f"--cppcheck-build-dir={config[ 'paths' ][ 'build' ]}"
            ,"--inline-suppr"
            ,f"--std={config[ 'compiler' ][ 'standard' ]}"
            ,"--error-exitcode=1"
            ,f"-j {config[ 'max_threads' ]}"
        ]
        
        params.append( f"--check-level={self.default_config[ 'strictness' ]}" )
            
        for suppression in self.default_config[ 'suppressions' ]:
            params.append( f"--suppress={suppression}" )

        params.append( f"-I{config[ 'paths' ][ 'include' ]}" )
        for d in config[ 'dependencies' ][ 'include_dirs' ]:
            params.append( f"-I{d}" )
            
        return  " ".join( params )


