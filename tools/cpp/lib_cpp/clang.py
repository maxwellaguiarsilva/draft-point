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
#   File:   tools/cpp/lib_cpp/clang.py
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-19 16:20:40
#


class clang:
    optimization_levels = {
        "none": "-O0"
        ,"balanced": "-O2"
        ,"aggressive": "-O3"
        ,"debug": "-Og"
    }

    warning_levels = {
        "minimal": ["-Wall"]
        ,"high": ["-Wall", "-Wextra"]
        ,"pedantic": ["-Wall", "-Wextra", "-Wpedantic"]
    }

    def __init__( self, config ):
        self.config = config

    def get_compile_command( self, source_path, object_path ):
        params = self._get_compile_params( )
        executable = self.config[ "compiler" ][ "executable" ]
        return  f"{executable} {params} -c {source_path} -o {object_path}"

    def get_link_command( self, object_files, binary_path ):
        params = self._get_link_params( )
        executable = self.config[ "compiler" ][ "executable" ]
        object_files_str = " ".join( object_files )
        return  f"{executable} {object_files_str} {params} -o {binary_path}"

    def _get_compile_params( self ):
        config = self.config
        params = [ ]
        
        params.append( f"-std={config[ 'compiler' ][ 'standard' ]}" )
        if config[ 'compiler' ][ 'use_64_bits' ]:
            params.append( "-m64" )
            
        opt_level = config[ 'build_behavior' ][ 'optimization' ]
        params.append( self.optimization_levels.get( opt_level, opt_level ) )
        
        if config[ 'build_behavior' ][ 'debug_symbols' ]:
            params.append( "-g" )
        if config[ 'build_behavior' ][ 'generate_dependencies' ]:
            params.append( "-MMD -MP" )
        if config[ 'build_behavior' ][ 'experimental_library' ]:
            params.append( "-fexperimental-library" )
            
        warn_level = config[ 'quality_control' ][ 'warning_level' ]
        params.extend( self.warning_levels.get( warn_level, [ warn_level ] ) )
        
        if config[ 'quality_control' ][ 'treat_warnings_as_errors' ]:
            params.append( "-Werror" )
        if config[ 'quality_control' ][ 'stop_on_first_error' ]:
            params.append( "-Wfatal-errors" )
            
        params.append( f"-I{config[ 'paths' ][ 'include' ]}" )
        for d in config[ 'dependencies' ][ 'include_dirs' ]:
            params.append( f"-I{d}" )
            
        params.extend( config[ 'compiler' ].get( 'extra_compile_flags', [ ] ) )

        return  " ".join( params )

    def _get_link_params( self ):
        config = self.config
        params = [ ]
        
        if config[ 'compiler' ][ 'use_64_bits' ]:
            params.append( "-m64" )
            
        opt_level = config[ 'build_behavior' ][ 'optimization' ]
        params.append( self.optimization_levels.get( opt_level, opt_level ) )

        if config[ 'build_behavior' ][ 'debug_symbols' ]:
            params.append( "-g" )

        for d in config[ 'dependencies' ].get( 'library_dirs', [ ] ):
            params.append( f"-L{d}" )

        for opt in config[ 'compiler' ].get( 'linker_direct_options', [ ] ):
            params.append( f"-Wl,{opt}" )

        params.extend( config[ 'compiler' ].get( 'extra_link_flags', [ ] ) )

        for lib in config[ 'dependencies' ][ 'libraries' ]:
            params.append( f"-l{lib}" )

        return  " ".join( params )


