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
#   File:   project_core
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-14 18:00:00


import copy
import datetime
import os
import re
import threading
import subprocess
from lib.common import ensure
from lib.common import deep_update
from cpp_lib.config import DEFAULT_CPP_CONFIG
from cpp_lib.project_tree import project_tree


class project_file:
    def __init__( self, node, project ):
        self.project = project
        self.node = node
        self.path = node.path
        self.modified_at = node.modified_at
        self.hierarchy = node.hierarchy
        self.content = node.content
        
        self.dependencies_modified_at = self.modified_at
        if self.node.closure:
            self.dependencies_modified_at = max( 
                [ self.modified_at ] + 
                [ dep.modified_at for dep in self.node.closure ] 
            )


class cpp( project_file ):
    def __init__( self, node, project ):
        source_folder = project.config["paths"]["source"]
        tests_folder  = project.config["paths"]["tests"]
        build_folder  = project.config["paths"]["build"]
        main_regexp   = project.config["patterns"]["main_function"]

        self.is_test = node.path.startswith( tests_folder )
        base_folder  = tests_folder if self.is_test else source_folder

        super().__init__( node, project )
        
        build_base = os.path.join( build_folder, base_folder )
        self.object_path = os.path.join( build_base, self.hierarchy + ".o" )

        self.is_main = bool( re.search( main_regexp, self.content ) )
        self.compiled_at = self._get_compiled_at( )
 
    def _get_compiled_at( self ):
        if os.path.exists( self.object_path ):
            return  datetime.datetime.fromtimestamp( os.path.getmtime( self.object_path ) )
        return  None

    def build( self ):
        if self.project._stop_event.is_set( ):
            return

        if self.compiled_at and self.dependencies_modified_at <= self.compiled_at:
            self.project.print( f"    [cached]: {self.hierarchy}", flg_check_stop = True )
            return

        compile_params = self.project._get_compile_params
        compiler_command = f"{self.project.config['compiler']['executable']} {compile_params} -c {self.path} -o {self.object_path}"

        result = subprocess.run( compiler_command, shell=True, capture_output=True, text=True )

        lines = [ f"    [build]: {self.hierarchy}" ]
        if result.returncode != 0:
            lines[ 0 ] += " (failed)"
        
        if result.stderr: lines.append( result.stderr.rstrip( "\n" ) )
        if result.stdout: lines.append( result.stdout.rstrip( "\n" ) )
            
        if result.returncode != 0:
            lines.append( f"compiler: {compiler_command}" )
            
        self.project.print( *lines, sep = "\n", flg_check_stop = True, flg_set_stop = ( result.returncode != 0 ) )
        
        ensure( result.returncode == 0, f"compilation failed for {self.path}" )
        self.compiled_at = self._get_compiled_at( )


class hpp( project_file ):
    def __init__( self, node, project ):
        super().__init__( node, project )


class binary_builder:
    def __init__( self, cpp ):
        ensure( cpp.is_main, f"file {cpp.path} does not contain a main function" )
        
        self.cpp = cpp
        dist_folder  = self.cpp.project.config["paths"]["output"]
        
        binary_name = os.path.basename( self.cpp.path )
        binary_name = os.path.splitext( binary_name )[0]
        self.binary_path = os.path.join( dist_folder, binary_name )
        
        if os.path.exists( self.binary_path ):
            self.modified_at = datetime.datetime.fromtimestamp( os.path.getmtime( self.binary_path ) )
        else:
            self.modified_at = None

        self.dependencies_list = []
        self._resolve_dependencies()

    def _resolve_dependencies( self ):
        self.dependencies_list = [ self.cpp ]
        visited_hierarchies = { self.cpp.hierarchy }
        
        for dep_node in self.cpp.node.closure:
            if dep_node.hierarchy in visited_hierarchies:
                continue
                
            visited_hierarchies.add( dep_node.hierarchy )
            item = self.cpp.project.hierarchy_items.get( dep_node.hierarchy )
            if item and item[ "cpp" ]:
                self.dependencies_list.append( item[ "cpp" ] )

    def link( self ):
        if self.cpp.project._stop_event.is_set( ):
            return

        os.makedirs( os.path.dirname( self.binary_path ), exist_ok=True )
        
        object_files = []
        flg_link = False
        for c in self.dependencies_list:
            object_files.append( c.object_path )
            if c.compiled_at and ( ( not self.modified_at ) or self.modified_at < c.compiled_at ):
                flg_link = True
        
        if flg_link:
            object_files_str = " ".join( object_files )
            linker_command = f"{self.cpp.project.config['compiler']['executable']} {object_files_str} {self.cpp.project._get_link_params} -o {self.binary_path}"
            
            result = subprocess.run( linker_command, shell=True, capture_output=True, text=True )

            if result.returncode != 0:
                self.cpp.project.print( f"    [link]: {os.path.basename( self.binary_path )} (failed)", flg_check_stop = True, flg_set_stop = True )
                if result.stderr: self.cpp.project.print( result.stderr.rstrip( "\n" ) )
                if result.stdout: self.cpp.project.print( result.stdout.rstrip( "\n" ) )
                self.cpp.project.print( f"linker: {linker_command}" )
                ensure( False, f"linking failed for {self.binary_path}" )
            else:
                self.cpp.project.print( f"    [link]: {os.path.basename( self.binary_path )}", flg_check_stop = True )
                if result.stderr: self.cpp.project.print( result.stderr.rstrip( "\n" ) )
                if result.stdout: self.cpp.project.print( result.stdout.rstrip( "\n" ) )


class project_core:
    def __init__( self, config = { } ):
        self.config = deep_update( copy.deepcopy( DEFAULT_CPP_CONFIG ), config )
        
        self.tree = project_tree( self.config )
        
        self.hpp_list = [ ]
        self.cpp_list = [ ]
        self.hierarchy_items = { }
        
        for node in self.tree.nodes.values( ):
            if not node.path:
                continue
            
            if node.path.endswith( ".hpp" ):
                obj = hpp( node, self )
                self.hpp_list.append( obj )
                self.hierarchy_items.setdefault( node.hierarchy, { "cpp": None, "hpp": None } )[ "hpp" ] = obj
            elif node.path.endswith( ".cpp" ):
                obj = cpp( node, self )
                self.cpp_list.append( obj )
                self.hierarchy_items.setdefault( node.hierarchy, { "cpp": None, "hpp": None } )[ "cpp" ] = obj

        self._lock = threading.Lock( )
        self._stop_event = threading.Event( )

        self.binary_list = [ binary_builder( c ) for c in self.cpp_list if c.is_main ]
        
        #   check for binary name collisions
        binaries_by_path = { }
        for b in self.binary_list:
            if b.binary_path in binaries_by_path:
                other_cpp = binaries_by_path[ b.binary_path ].cpp.path
                this_cpp = b.cpp.path
                ensure( False, f"binary name collision: '{os.path.basename( b.binary_path )}' is generated by both '{other_cpp}' and '{this_cpp}'" )
            binaries_by_path[ b.binary_path ] = b

    def print( self, *args, flg_check_stop = False, flg_set_stop = False, **kwargs ):
        with self._lock:
            if flg_check_stop and self._stop_event.is_set( ):
                return

            if flg_set_stop:
                self._stop_event.set( )

            print( *args, **kwargs )

    @property
    def _get_compile_params( self ):
        config = self.config
        params = []
        
        params.append( f"-std={config['compiler']['standard']}" )
        if config['compiler']['use_64_bits']:
            params.append( "-m64" )
            
        opt_map = config['build_behavior'].get( 'optimization_levels', { } )
        opt_level = config['build_behavior']['optimization']
        params.append( opt_map.get( opt_level, opt_level ) )
        
        if config['build_behavior']['debug_symbols']:
            params.append( "-g" )
        if config['build_behavior']['generate_dependencies']:
            params.append( "-MMD -MP" )
        if config['build_behavior']['experimental_library']:
            params.append( "-fexperimental-library" )
            
        warn_map = config['quality_control'].get( 'warning_levels', { } )
        warn_level = config['quality_control']['warning_level']
        params.extend( warn_map.get( warn_level, [ warn_level ] ) )
        
        if config['quality_control']['treat_warnings_as_errors']:
            params.append( "-Werror" )
        if config['quality_control']['stop_on_first_error']:
            params.append( "-Wfatal-errors" )
            
        params.append( f"-I{config['paths']['include']}" )
        for d in config['dependencies']['include_dirs']:
            params.append( f"-I{d}" )
            
        params.extend( config['compiler'].get( 'extra_compile_flags', [ ] ) )

        return " ".join( params )

    @property
    def _get_link_params( self ):
        config = self.config
        params = []
        
        if config['compiler']['use_64_bits']:
            params.append( "-m64" )
            
        opt_map = config['build_behavior'].get( 'optimization_levels', { } )
        opt_level = config['build_behavior']['optimization']
        params.append( opt_map.get( opt_level, opt_level ) )

        if config['build_behavior']['debug_symbols']:
            params.append( "-g" )

        for d in config['dependencies'].get( 'library_dirs', [ ] ):
            params.append( f"-L{d}" )

        for opt in config['compiler'].get( 'linker_direct_options', [ ] ):
            params.append( f"-Wl,{opt}" )

        params.extend( config['compiler'].get( 'extra_link_flags', [ ] ) )

        for lib in config['dependencies']['libraries']:
            params.append( f"-l{lib}" )

        return " ".join( params )

    @property
    def _get_cppcheck_params( self ):
        config = self.config
        analysis_config = config['quality_control']['static_analysis']
        
        params = [
            "--quiet",
            "--enable=all",
            f"--cppcheck-build-dir={config['paths']['build']}",
            "--inline-suppr",
            f"--std={config['compiler']['standard']}",
            "--error-exitcode=1",
            f"-j {config['build_behavior']['max_threads']}"
        ]
        
        params.append( f"--check-level={analysis_config['strictness']}" )
            
        for suppression in analysis_config.get( 'suppressions', [ ] ):
            params.append( f"--suppress={suppression}" )

        params.append( f"-I{config['paths']['include']}" )
        for d in config['dependencies']['include_dirs']:
            params.append( f"-I{d}" )
            
        return " ".join( params )

    def run_cppcheck( self ):
        if not self.config['quality_control']['static_analysis']['enabled']:
            return

        build_dir = self.config['paths']['build']
        os.makedirs( build_dir, exist_ok=True )
        
        cppcheck_params = self._get_cppcheck_params
        
        source_dir = self.config['paths']['source']
        tests_dir = self.config['paths']['tests']
        
        cppcheck_command = f"cppcheck {cppcheck_params} \"{source_dir}\" \"{tests_dir}\""
        
        self.print( "running static analysis (cppcheck)..." )
        result = subprocess.run( cppcheck_command, shell=True )
        if result.returncode != 0:
            self.print( f"cppcheck: {cppcheck_command}" )
        ensure( result.returncode == 0, "cppcheck failed for the project" )
        self.print( "static analysis completed successfully" )
