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
#   File:   project_map
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-15 10:00:00


import os
import re
import glob
import datetime
from lib.common import ensure, read_file, get_modification_time


def parse_hierarchy( hierarchy ):
    return re.split( r"[/:\\.]+", hierarchy )


class project_file:
    def __init__( self, path, hierarchy, is_external = False ):
        self.path = path
        self.hierarchy = hierarchy
        self.is_external = is_external
        self.content = None
        self.includes = set( )
        self.dependencies = set( )
        self.modified_at = datetime.datetime.min

        if self.path and not self.is_external:
            mtime = get_modification_time( self.path )
            if mtime:
                self.modified_at = mtime
                self.content = read_file( self.path )

    @property
    def dependencies_modified_at( self ):
        return max( [ self.modified_at ] + [ dep.modified_at for dep in self.dependencies ], default = self.modified_at )


class hpp( project_file ):
    def __init__( self, path, hierarchy, is_external = False ):
        super( ).__init__( path, hierarchy, is_external )


class cpp( project_file ):
    def __init__( self, path, hierarchy, config ):
        super( ).__init__( path, hierarchy )
        
        source_folder = config[ "paths" ][ "source" ]
        tests_folder  = config[ "paths" ][ "tests" ]
        build_folder  = config[ "paths" ][ "build" ]
        main_regexp   = config[ "language" ][ "patterns" ][ "main_function" ]

        self.is_test = self.path.startswith( tests_folder )
        base_folder  = tests_folder if self.is_test else source_folder
        
        build_base = os.path.join( build_folder, base_folder )
        self.object_path = os.path.join( build_base, self.hierarchy + ".o" )

        self.is_main = False
        if self.content:
            self.is_main = bool( re.search( main_regexp, self.content ) )
        
        self.update_compiled_at( )

    def update_compiled_at( self ):
        self.compiled_at = get_modification_time( self.object_path )
        return self.compiled_at


class project_map:
    def __init__( self, config ):
        self.config = config
        self.files = { }
        self.hierarchy_items = { }
        self.path_map = { }
        self.include_pattern = re.compile( self.config[ "language" ][ "patterns" ][ "include_directive" ] )
        
        self._scan_project( )
        self._resolve_all_includes( )
        self._calculate_all_dependencies( )

    def _scan_project( self ):
        include_dir = self.config[ "paths" ][ "include" ]
        source_dir = self.config[ "paths" ][ "source" ]
        tests_dir = self.config[ "paths" ][ "tests" ]

        self._scan_dir( include_dir, include_dir, hpp )
        self._scan_dir( source_dir, source_dir, cpp )
        self._scan_dir( tests_dir, tests_dir, cpp )

    def _scan_dir( self, root_path, base_path, file_class ):
        exts = ( 
             f".{self.config[ 'language' ][ 'header_extension' ]}"
            ,f".{self.config[ 'language' ][ 'source_extension' ]}" 
        )
        pattern = os.path.join( root_path, "**", "*" )
        for path in glob.glob( pattern, recursive = True ):
            if os.path.isfile( path ) and path.endswith( exts ):
                path_from_base = os.path.relpath( path, base_path )
                hierarchy = os.path.splitext( path_from_base )[ 0 ]
                
                if file_class == cpp:
                    file_obj = cpp( path, hierarchy, self.config )
                else:
                    file_obj = hpp( path, hierarchy )
                
                self.files[ path ] = file_obj
                self.path_map[ path_from_base ] = file_obj
                
                items = self.hierarchy_items.setdefault( hierarchy, { "cpp": None, "hpp": None } )
                if isinstance( file_obj, cpp ):
                    items[ "cpp" ] = file_obj
                else:
                    items[ "hpp" ] = file_obj

    def _resolve_include( self, include_path ):
        if include_path in self.path_map:
            return self.path_map[ include_path ]

        hierarchy = os.path.splitext( include_path )[ 0 ]
        if hierarchy in self.hierarchy_items:
            items = self.hierarchy_items[ hierarchy ]
            if items[ "hpp" ]:
                return items[ "hpp" ]
            if items[ "cpp" ]:
                return items[ "cpp" ]
        
        if include_path not in self.files:
            file_obj = hpp( None, include_path, is_external = True )
            self.files[ include_path ] = file_obj
        return self.files[ include_path ]

    def _resolve_all_includes( self ):
        for file_obj in list( self.files.values( ) ):
            if not file_obj.content:
                continue
            
            for match in self.include_pattern.finditer( file_obj.content ):
                include_path = match.group( "path" )
                target_file = self._resolve_include( include_path )
                file_obj.includes.add( target_file )

    def _calculate_dependencies( self, file_obj, visited ):
        if file_obj in visited:
            return set( )
        
        if file_obj.dependencies:
            return file_obj.dependencies
            
        new_visited = visited | { file_obj }
        dependencies = set( )
        for target in file_obj.includes:
            dependencies.add( target )
            dependencies |= self._calculate_dependencies( target, new_visited )
            
        file_obj.dependencies = dependencies
        return dependencies

    def _calculate_all_dependencies( self ):
        for file_obj in self.files.values( ):
            self._calculate_dependencies( file_obj, set( ) )