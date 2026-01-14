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
#   File:   project_core
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-14 18:00:00


import copy
import concurrent.futures
import datetime
import glob
import os
import re
import threading
import subprocess
from lib.common import ensure


def get_cpu_count( ):
    try:
        return  len( os.sched_getaffinity( 0 ) )
    except AttributeError:
        return  os.cpu_count( ) or 1


DEFAULT_CONFIG = {
    #   compiler information
    "compiler": {
        "executable": "clang++"        #   options: "g++", "clang++" or full path
        ,"standard": "c++23"           #   examples: "c++17", "c++20", "c++23"
        ,"use_64_bits": True           #   abstraction for -m64 (64-bit)
        ,"extra_compile_flags": [
            "-ffunction-sections"      #   place each function into its own section
            ,"-fdata-sections"         #   place each data item into its own section
            ,"-flto"                   #   enable link time optimization
        ]
        ,"extra_link_flags": [
            "-flto"                    #   enable link time optimization during linking
        ]
        ,"linker_direct_options": [
            "--as-needed"              #   only link libraries that satisfy undefined symbols
            ,"--gc-sections"           #   remove unused sections (dead code elimination)
        ]
    }

    #   folder organization
    ,"paths": {
        "source": "source"             #   project source directory containing .cpp files
        ,"include": "include"          #   project include directory containing .hpp files
        ,"tests": "tests"              #   test source directory containing .cpp files
        ,"build": "build"              #   build directory for object files (.o)
        ,"output": "dist"              #   output directory for the final binaries
    }

    #   build rules (how to compile)
    ,"build_behavior": {
        #   options: "none" (-O0), "balanced" (-O2), "aggressive" (-O3), "debug" (-Og)
        "optimization": "balanced"
        ,"optimization_levels": {
            "none": "-O0"
            ,"balanced": "-O2"
            ,"aggressive": "-O3"
            ,"debug": "-Og"
        }
        ,"debug_symbols": False         #   generates symbols for gdb (-g)
        ,"generate_dependencies": False #   generates .d files (intelligent recompilation)
        ,"experimental_library": True   #   enables -fexperimental-library
        ,"max_threads": get_cpu_count( )
    }

    #   quality control (warning and analysis flags)
    ,"quality_control": {
        #   options: "minimal", "high" (-Wall -Wextra), "pedantic"
        "warning_level": "high"
        ,"warning_levels": {
            "minimal": ["-Wall"]
            ,"high": ["-Wall", "-Wextra"]
            ,"pedantic": ["-Wall", "-Wextra", "-Wpedantic"]
        }
        ,"treat_warnings_as_errors": True #   -Werror
        ,"stop_on_first_error": True      #   -Wfatal-errors
        ,"static_analysis": {
            "enabled": True
            ,"tool": "cppcheck"
            ,"strictness": "exhaustive"    #   options: "normal", "exhaustive"
            ,"suppressions": [
                "missingIncludeSystem"
                ,"checkersReport"
            ]
        }
    }

    #   external dependencies (no -l or -I prefixes)
    ,"dependencies": {
        "libraries": []                #   example: ["ncurses", "pthread"]
        ,"include_dirs": []            #   additional paths for header search
        ,"library_dirs": []             #   additional paths for library search
    }

    #   file search patterns
    ,"patterns": {
        "source_extension": "cpp"
        ,"header_extension": "hpp"
        ,"main_function": r"\b(int|auto)\s+main\s*\("
    }
}


def deep_update( source, overrides ):
    for key, value in overrides.items( ):
        if isinstance( value, dict ) and key in source and isinstance( source[ key ], dict ):
            deep_update( source[ key ], value )
        else:
            source[ key ] = value
    return source


class project_file:
    def __init__( self, path, project, base_folder ):
        self.project = project
        self.path = path
        self.modified_at = datetime.datetime.fromtimestamp( os.path.getmtime( path ) )
        self.dependencies_modified_at = self.modified_at
        relative_path = os.path.relpath( path, base_folder )
        self.hierarchy = os.path.splitext( relative_path )[0]

        with open( path, "r" ) as f:
            self.content = f.read( )


class cpp( project_file ):
    def __init__( self, path, project ):
        source_folder = project.config["paths"]["source"]
        tests_folder  = project.config["paths"]["tests"]
        build_folder  = project.config["paths"]["build"]
        main_regexp   = project.config["patterns"]["main_function"]

        self.is_test = path.startswith( tests_folder )
        base_folder  = tests_folder if self.is_test else source_folder

        super().__init__( path, project, base_folder )
        
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
    def __init__( self, path, project ):
        base_folder = project.config["paths"]["include"]
        super().__init__( path, project, base_folder )


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
        visited = set()
        
        def walk( hierarchy_name ):
            if hierarchy_name in visited:
                return
            visited.add( hierarchy_name )
            
            item = self.cpp.project.hierarchy_items[ hierarchy_name ]
            cpp_obj = item[ "cpp" ]
            hpp_obj = item[ "hpp" ]
            
            if cpp_obj:
                self.dependencies_list.append( cpp_obj )
            
            all_includes = set()
            if cpp_obj:
                all_includes.update( cpp_obj.included_items.keys() )
            if hpp_obj:
                all_includes.update( hpp_obj.included_items.keys() )
                
            for incl_hierarchy in all_includes:
                walk( incl_hierarchy )

        walk( self.cpp.hierarchy )

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
        self.config = deep_update( copy.deepcopy( DEFAULT_CONFIG ), config )
        
        include_ext = self.config["patterns"]["header_extension"]
        source_ext  = self.config["patterns"]["source_extension"]

        include_list    =   glob.glob( os.path.join( self.config["paths"]["include"], f"**/*.{include_ext}" ), recursive=True )
        source_list     =   glob.glob( os.path.join( self.config["paths"]["source"], f"**/*.{source_ext}" ), recursive=True )
        tests_list      =   glob.glob( os.path.join( self.config["paths"]["tests"], f"**/*.{source_ext}" ), recursive=True )

        self.hpp_list = [hpp( p, self ) for p in include_list]
        self.cpp_list = [cpp( p, self ) for p in source_list + tests_list]
        self.hierarchy_items = self._get_hierarchy_items( )

        self._lock = threading.Lock( )
        self._stop_event = threading.Event( )

        self._stabilize_dependencies( )
        self.binary_list = [binary_builder( c ) for c in self.cpp_list if c.is_main]
        
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

    def _get_hierarchy_items( self ):
        hierarchy_items = {}
        for h in self.hpp_list:
            hierarchy_items.setdefault( h.hierarchy, { "cpp": None, "hpp": None } )[ "hpp" ] = h
        for c in self.cpp_list:
            hierarchy_items.setdefault( c.hierarchy, { "cpp": None, "hpp": None } )[ "cpp" ] = c
        return hierarchy_items

    def _update_pair_info( self ):
        for item in self.hierarchy_items.values( ):
            cpp_obj = item[ "cpp" ]
            hpp_obj = item[ "hpp" ]
            if cpp_obj:
                cpp_obj.hpp = hpp_obj
            if hpp_obj:
                hpp_obj.cpp = cpp_obj

    def _update_included_items( self ):
        self._update_pair_info( )
        include_pattern = re.compile( r'#include\s*[<\"]([^>\"]+)[>\"]' )
        for obj in self.hpp_list + self.cpp_list:
            obj.included_items = {}
            matches = include_pattern.findall( obj.content )
            for match in matches:
                included_hierarchy = os.path.splitext( match )[0]
                if included_hierarchy in self.hierarchy_items:
                    obj.included_items[included_hierarchy] = self.hierarchy_items[included_hierarchy]

    def _stabilize_dependencies( self ):
        self._update_included_items( )
        has_changes = True
        while has_changes:
            has_changes = False
            for obj in self.hpp_list + self.cpp_list:
                current_max = max( datetime.datetime.min, obj.dependencies_modified_at )
                old_max = current_max
                for dep_item in obj.included_items.values( ):
                    if dep_item[ "hpp" ]:
                        current_max = max( current_max, dep_item[ "hpp" ].dependencies_modified_at )
                if current_max > old_max:
                    has_changes = True
                    obj.dependencies_modified_at = current_max

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
