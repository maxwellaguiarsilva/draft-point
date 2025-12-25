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
#   File:   tree-builder
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2025-12-25 16:09:39


import copy
import datetime
import glob
import os
import re


DEFAULT_CONFIG = {
    # Compiler information
    "compiler": {
        "executable": "clang++",       # Options: "g++", "clang++" or full path
        "standard": "c++23",           # Examples: "c++17", "c++20", "c++23"
        "use_64_bits": True,           # Abstraction for -m64 (64-bit)
    },

    # Folder organization
    "paths": {
        "source": "source",            # Project source directory containing .cpp files
        "include": "include",          # Project include directory containing .hpp files
        "tests": "tests",              # Test source directory containing .cpp files
        "build": "build",              # Build directory for object files (.o) and dependency files (.d)
        "output": "dist",              # Output directory for the final binaries
    },

    # Build rules (How to compile)
    "build_behavior": {
        # Options: "none" (-O0), "balanced" (-O2), "aggressive" (-O3), "debug" (-Og)
        "optimization": "balanced",    
        "debug_symbols": False,        # Generates symbols for GDB (-g)
        "generate_dependencies": True, # Generates .d files (intelligent recompilation)
        "experimental_library": True,  # Enables -fexperimental-library
    },

    # Quality Control (Warning and analysis flags)
    "quality_control": {
        # Options: "minimal", "high" (-Wall -Wextra), "pedantic"
        "warning_level": "high",       
        "treat_warnings_as_errors": True, # -Werror
        "stop_on_first_error": True,      # -Wfatal-errors
        "static_analysis": {
            "enabled": True,
            "tool": "cppcheck",
            "strictness": "exhaustive"    # Options: "normal", "exhaustive"
        }
    },

    # External dependencies (No -l or -I prefixes)
    "dependencies": {
        "libraries": [],               # Example: ["ncurses", "pthread"]
        "include_dirs": []             # Additional paths for header search
    },

    # File search patterns
    "patterns": {
        "source_extension": "cpp",
        "header_extension": "hpp",
        "main_function": r"\b(int|auto)\s+main\s*\("
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

        with open( path, 'r' ) as f:
            self.content = f.read( )

    def _repr_included_items( self ):
        included_keys = [f'"{k}"' for k in self.included_items.keys()] if hasattr(self, 'included_items') else []
        return "[" + ", ".join(included_keys) + "]"


class cpp( project_file ):
    def __init__( self, path, project ):
        source_folder = project.config["paths"]["source"]
        tests_folder  = project.config["paths"]["tests"]
        build_folder  = project.config["paths"]["build"]
        main_regexp   = project.config["patterns"]["main_function"]

        self.is_test = path.startswith( tests_folder )
        base_folder  = tests_folder if self.is_test else source_folder

        super().__init__( path, project, base_folder )
        
        # Caminhos no diretório de build
        build_base = os.path.join( build_folder, base_folder )
        self.object_path = os.path.join( build_base, self.hierarchy + ".o" )
        self.dependency_path = self.object_path + ".d"

        self.is_main = bool( re.search( main_regexp, self.content ) )

        if os.path.exists( self.object_path ) and os.path.exists( self.dependency_path ):
            self.compiled_at = datetime.datetime.fromtimestamp( max( 
                os.path.getmtime( self.object_path ), 
                os.path.getmtime( self.dependency_path ) 
            ) )
        else:
            self.compiled_at = None
    
    #   llm-prompt-target 
    def build( self ):
        if self.compiled_at and self.dependencies_modified_at <= self.compiled_at:
            return

        cppcheck_params = self.project._get_cppcheck_params
        compile_params = self.project._get_compile_params
        
        cppcheck_command = f"cppcheck {cppcheck_params} {self.path}"
        compiler_command = f"{self.project.config['compiler']['executable']} {compile_params} -c {self.path} -o {self.object_path}"

        print( f"cppcheck: {cppcheck_command}" )
        if os.system( cppcheck_command ) != 0:
            raise Exception( f"cppcheck falhou para {self.path}" )
        
        print( f"clang++: {compiler_command}" )
        if os.system( compiler_command ) != 0:
            raise Exception( f"clang++ falhou para {self.path}" )


    def __repr__( self ):
        compiled_at_str = f"\"{self.compiled_at.isoformat()}\"" if self.compiled_at else "null"
        return ( f"{{"
            f"\"hierarchy\": \"{self.hierarchy}\""
            f", \"is_main\": {str(self.is_main).lower()}"
            f", \"is_test\": {str(self.is_test).lower()}"
            f", \"cpp_path\": \"{self.path}\""
            f", \"object_path\": \"{self.object_path}\""
            f", \"dependency_path\": \"{self.dependency_path}\""
            f", \"modified_at\": \"{self.modified_at.isoformat()}\""
            f", \"dependencies_modified_at\": \"{self.dependencies_modified_at.isoformat()}\""
            f", \"compiled_at\": {compiled_at_str}"
            f", \"included_items\": {self._repr_included_items()}"
            f"}}"
        )


class binary_builder:
    def __init__( self, cpp ):
        if not cpp.is_main:
            raise ValueError( f"O arquivo {cpp.path} não contém uma função main." )
        
        self.cpp = cpp
        dist_folder  = self.cpp.project.config["paths"]["output"]
        
        # O binário terá apenas o nome do arquivo, sem a extensão e sem a hierarquia
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
            
            # Combine includes from both cpp and hpp (if they exist)
            all_includes = set()
            if cpp_obj:
                all_includes.update( cpp_obj.included_items.keys() )
            if hpp_obj:
                all_includes.update( hpp_obj.included_items.keys() )
                
            for incl_hierarchy in all_includes:
                walk( incl_hierarchy )

        walk( self.cpp.hierarchy )

    def __repr__( self ):
        modified_at_str = f"\"{self.modified_at.isoformat()}\"" if self.modified_at else "null"
        dependencies_json = [ f"\"{d.hierarchy}\"" for d in self.dependencies_list ]
        return ( f"{{"
            f"\"hierarchy\": \"{self.cpp.hierarchy}\""
            f", \"binary_path\": \"{self.binary_path}\""
            f", \"modified_at\": {modified_at_str}"
            f", \"dependencies_list\": [{', '.join(dependencies_json)}]"
            f"}}"
        )


class hpp( project_file ):
    def __init__( self, path, project ):
        base_folder = project.config["paths"]["include"]
        super().__init__( path, project, base_folder )

    def __repr__( self ):
        return ( f"{{"
            f"\"hierarchy\": \"{self.hierarchy}\""
            f", \"hpp_path\": \"{self.path}\""
            f", \"modified_at\": \"{self.modified_at.isoformat()}\""
            f", \"included_items\": {self._repr_included_items()}"
            f"}}"
        )

class project:
    def __init__( self, config ):
        self.config = deep_update( copy.deepcopy( DEFAULT_CONFIG ), config )
        
        include_ext = self.config["patterns"]["header_extension"]
        source_ext  = self.config["patterns"]["source_extension"]

        include_list    =   glob.glob( os.path.join( self.config["paths"]["include"], f"**/*.{include_ext}" ), recursive=True )
        source_list     =   glob.glob( os.path.join( self.config["paths"]["source"], f"**/*.{source_ext}" ), recursive=True )
        tests_list      =   glob.glob( os.path.join( self.config["paths"]["tests"], f"**/*.{source_ext}" ), recursive=True )

        self.hpp_list = [hpp( p, self ) for p in include_list]
        self.cpp_list = [cpp( p, self ) for p in source_list + tests_list]
        self.hierarchy_items = self._get_hierarchy_items( )

        self._update_pair_info( )
        self._update_included_items( )
        self._stabilize_dependencies( )
        self.binary_list = [binary_builder( c ) for c in self.cpp_list if c.is_main]

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
            if cpp_obj and hpp_obj:
                cpp_obj.dependencies_modified_at = max( cpp_obj.dependencies_modified_at, hpp_obj.modified_at )
                hpp_obj.dependencies_modified_at = max( cpp_obj.dependencies_modified_at, hpp_obj.modified_at )

    def _update_included_items( self ):
        include_pattern = re.compile( r'#include\s*[<"]([^>"]+)[>"]' )
        for obj in self.hpp_list + self.cpp_list:
            obj.included_items = {}
            matches = include_pattern.findall( obj.content )
            for match in matches:
                included_hierarchy = os.path.splitext( match )[0]
                if included_hierarchy in self.hierarchy_items:
                    obj.included_items[included_hierarchy] = self.hierarchy_items[included_hierarchy]

    def _stabilize_dependencies( self ):
        has_changes = True
        while has_changes:
            has_changes = False
            for item in self.hierarchy_items.values( ):
                cpp_obj = item[ "cpp" ]
                hpp_obj = item[ "hpp" ]
                
                # Pega a data atual da hierarquia
                current_max = datetime.datetime.min
                if cpp_obj: current_max = max( current_max, cpp_obj.dependencies_modified_at )
                if hpp_obj: current_max = max( current_max, hpp_obj.dependencies_modified_at )
                old_max = current_max
                
                # Verifica dependências de ambos
                for obj in [ o for o in [cpp_obj, hpp_obj] if o ]:
                    for dep_item in obj.included_items.values( ):
                        if dep_item[ "hpp" ]:
                            current_max = max( current_max, dep_item[ "hpp" ].dependencies_modified_at )
                        if dep_item[ "cpp" ]:
                            current_max = max( current_max, dep_item[ "cpp" ].dependencies_modified_at )
                
                if current_max > old_max:
                    has_changes = True
                    if cpp_obj: cpp_obj.dependencies_modified_at = current_max
                    if hpp_obj: hpp_obj.dependencies_modified_at = current_max

    @property
    def _get_compile_params( self ):
        config = self.config
        params = []
        
        params.append( f"-std={config['compiler']['standard']}" )
        if config['compiler']['use_64_bits']:
            params.append( "-m64" )
            
        opt_map = { "none": "-O0", "balanced": "-O2", "aggressive": "-O3", "debug": "-Og" }
        params.append( opt_map.get( config['build_behavior']['optimization'], "-O2" ) )
        
        if config['build_behavior']['debug_symbols']:
            params.append( "-g" )
        if config['build_behavior']['generate_dependencies']:
            params.append( "-MMD -MP" )
        if config['build_behavior']['experimental_library']:
            params.append( "-fexperimental-library" )
            
        warn_map = { "minimal": ["-Wall"], "high": ["-Wall", "-Wextra"], "pedantic": ["-Wall", "-Wextra", "-Wpedantic"] }
        params.extend( warn_map.get( config['quality_control']['warning_level'], ["-Wall", "-Wextra"] ) )
        
        if config['quality_control']['treat_warnings_as_errors']:
            params.append( "-Werror" )
        if config['quality_control']['stop_on_first_error']:
            params.append( "-Wfatal-errors" )
            
        params.append( f"-I{config['paths']['include']}" )
        for d in config['dependencies']['include_dirs']:
            params.append( f"-I{d}" )
            
        return " ".join( params )

    @property
    def _get_link_params( self ):
        config = self.config
        params = []
        
        if config['compiler']['use_64_bits']:
            params.append( "-m64" )
            
        opt_map = { "none": "-O0", "balanced": "-O2", "aggressive": "-O3", "debug": "-Og" }
        params.append( opt_map.get( config['build_behavior']['optimization'], "-O2" ) )

        if config['build_behavior']['debug_symbols']:
            params.append( "-g" )

        for lib in config['dependencies']['libraries']:
            params.append( f"-l{lib}" )
            
        return " ".join( params )

    @property
    def _get_cppcheck_params( self ):
        config = self.config
        params = [
            "--quiet",
            "--enable=all",
            "--suppress=missingIncludeSystem",
            "--suppress=checkersReport",
            "--inline-suppr",
            f"--std={config['compiler']['standard']}",
            "--error-exitcode=1"
        ]
        
        if config['quality_control']['static_analysis']['strictness'] == "exhaustive":
            params.append( "--check-level=exhaustive" )
            
        params.append( f"-I{config['paths']['include']}" )
        for d in config['dependencies']['include_dirs']:
            params.append( f"-I{d}" )
            
        return " ".join( params )

    def __repr__( self ):
        items = [ ]
        for key, value in self.hierarchy_items.items( ):
            cpp_json = str( value[ "cpp" ] ) if value[ "cpp" ] else "null"
            hpp_json = str( value[ "hpp" ] ) if value[ "hpp" ] else "null"
            items.append( f"\"{key}\": {{ \"cpp\": {cpp_json}, \"hpp\": {hpp_json} }}" )
        return "{\n " + "\n ,".join( items ) + "\n}"

    def build( self ):
        for c in self.cpp_list:
            os.makedirs( os.path.dirname( c.object_path ), exist_ok=True )
            c.build( )

        for b in self.binary_list:
            os.makedirs( os.path.dirname( b.binary_path ), exist_ok=True )

        #   print( self.binary_list )


current_project = project( { } )
#   print( current_project )
current_project.build( )



