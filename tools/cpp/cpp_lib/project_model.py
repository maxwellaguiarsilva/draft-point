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
#   File:   tools/cpp/cpp_lib/project_model.py
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-20 17:23:04
#


from lib.fso import text_file, file, get_file_list
from lib.project_file import project_file
from lib.common import to_dict, to_json
import re
import os


def parse_hierarchy( hierarchy ):
    return  re.split( r"[/:\\.]+", hierarchy )


class cpp_project_file( project_file ):
    include_regex   =   re.compile( r'#include\s*(?P<full>(?P<open>[<"])(?P<path>[^>"]+)(?P<close>[>"]))' )

    def __init__( self, file_path, project ):
        self.project    =   project
        super( ).__init__( file_path )
    
    @property
    def shebang( self ):
        return  ""
    
    @property
    def comment_string( self ):
        return  self.project.config[ "language" ][ "comment_string" ]
    
    def refresh( self ):
        super( ).refresh( )
        self.includes   =   [ match.group( "path" ) for match in self.include_regex.finditer( self.content ) ] if self.content else [ ]

    @property
    def hierarchy( self ):
        path    =   self.path
        for prefix in [ self.project.include_dir, self.project.source_dir, self.project.tests_dir ]:
            if path.startswith( prefix ):
                path    =   os.path.relpath( path, prefix )
                break
        return  os.path.splitext( path )[ 0 ]
    
    @property
    def dependencies( self ):
        visited =   { self }
        stack   =   [ self ]
        while stack:
            current =   stack.pop( )
            for include in current.includes:
                header  =   self.project.get_file( include, is_header = True )
                if header and header not in visited:
                    visited.add( header )
                    stack.append( header )
        return  { dep for dep in visited if isinstance( dep, hpp ) and dep is not self }
    
    @property
    def dependencies_modified_at( self ):
        return  max( [ self.modified_at ] + [ dep.modified_at for dep in self.dependencies ], default = self.modified_at )

    @property
    def to_dict( self ):
        return  super( ).to_dict | to_dict( self, [ "hierarchy", "includes", "dependencies_modified_at" ] )



class hpp( cpp_project_file ):
    def __init__( self, *args, **kwargs ):
        super( ).__init__( *args, **kwargs )


class cpp( cpp_project_file ):
    main_regex  =   r"\b(int|auto)\s+main\s*\("

    def __init__( self, *args, **kwargs ):
        super( ).__init__( *args, **kwargs )
        self.is_main = bool( re.search( self.main_regex, self.content ) ) if self.content else False
        self.is_test = self.path.startswith( self.project.tests_dir )
        self.object  = file( os.path.join( self.project.build_dir, os.path.splitext( self.path )[ 0 ] + ".o" ) )
        self.binary  = file( os.path.join( self.project.output_dir, self.name ) )

    @property
    def compiled_at( self ):
        return  self.object.modified_at

    @property
    def is_compilation_needed( self ):
        return  ( 
            self.compiled_at is None 
            or self.compiled_at < self.modified_at 
            or self.compiled_at < self.dependencies_modified_at 
        )

    @property
    def is_linkage_needed( self ):
        if not self.is_main:
            return  False
        
        link_max_compiled_at = max( [ self.compiled_at ] + [ dep.compiled_at for dep in self.link_list if dep.compiled_at ], default = None )
        
        return  (
            self.binary.modified_at is None
            or ( link_max_compiled_at is not None and self.binary.modified_at < link_max_compiled_at )
        )

    @property
    def link_list( self ):
        if not self.is_main:
            return  [ ]
        return  [
            implementation
            for dep in self.dependencies
            if ( implementation := self.project.get_file( dep.hierarchy, is_header = False ) )
        ]

    @property
    def to_dict( self ):
        return  super( ).to_dict | to_dict( self, [ "is_main", "is_test", "compiled_at", "is_compilation_needed", "is_linkage_needed", "link_list" ] )


class project_model:

    def __init__( self, config ):
        self.config =   config

        paths   =   self.config[ "paths" ]
        self.include_dir    =   paths[ "include" ]
        self.source_dir     =   paths[ "source" ]
        self.tests_dir      =   paths[ "tests" ]
        self.adhoc_dir      =   paths[ "adhoc" ]
        self.build_dir      =   paths[ "build" ]
        self.output_dir     =   paths[ "output" ]

        language    =   self.config[ "language" ]
        self.source_ext  =   language[ "source_extension" ]
        self.header_ext  =   language[ "header_extension" ]
        
        self.files  =   self.scan_dir( [ self.include_dir, self.source_dir, self.tests_dir ] )

    def scan_dir( self, dir_path ):
        if isinstance( dir_path, list ):
            files   =   { }
            for path in dir_path:
                files.update( self.scan_dir( path ) )
            return  files
        
        return  {
            file_path: cpp( file_path, self ) if file_path.endswith( f".{self.source_ext}" ) else hpp( file_path, self )
            for file_path in get_file_list( dir_path, extensions = [ self.header_ext, self.source_ext ] )
        }

    def get_file( self, file_path, is_header = True ):
        extension   =   self.header_ext if is_header else self.source_ext
        directory   =   self.include_dir if is_header else self.source_dir

        candidates  =   [
            file_path
            ,f"{file_path}.{extension}"
            ,os.path.join( directory, file_path )
            ,os.path.join( directory, f"{file_path}.{extension}" )
        ]

        for candidate in candidates:
            if candidate in self.files:
                return  self.files[ candidate ]

        return  None

    def get_path_for_hierarchy( self, hierarchy, file_type, flg_adhoc = False ):
        hierarchy_list = parse_hierarchy( hierarchy )
        rel_path = "/".join( hierarchy_list )

        if file_type == "header":
            return  f"{self.include_dir}/{rel_path}.{self.header_ext}"
        
        if file_type == "source":
            return  f"{self.source_dir}/{rel_path}.{self.source_ext}"
        
        if file_type == "test":
            if flg_adhoc:
                prefix = self._get_next_adhoc_prefix( )
                return  f"{self.adhoc_dir}/{prefix}_{hierarchy}/{prefix}_{hierarchy}.{self.source_ext}"
            
            test_file_name = f"test_{'_'.join( hierarchy_list )}.{self.source_ext}"
            return  f"{self.tests_dir}/{ '/'.join( hierarchy_list[ :-1 ] + [ test_file_name ] ) }"

        raise ValueError( f"invalid file_type: {file_type}" )

    def _get_next_adhoc_prefix( self ):
        from pathlib import Path
        path = Path( self.adhoc_dir )
        path.mkdir( exist_ok = True, parents = True )
        
        ids = {
            int( p.name.split( "_" )[ 0 ] )
            for p in path.iterdir( )
            if p.is_dir( ) and p.name.split( "_" )[ 0 ].isdigit( )
        }
        
        next_counter = 1
        while next_counter in ids:
            next_counter += 1
        
        return  f"{next_counter:04d}"

    @property
    def to_dict( self ):
        return  to_dict( self, [ "files" ] )

    def __repr__( self ):
        return  to_json( self.to_dict )
                                      

if __name__ == "__main__":
    import sys
    if len( sys.argv ) > 1 and sys.argv[ 1 ] == "--run-test":
        from cpp_lib.cpp_project_config import cpp_project_config
        print( project_model( cpp_project_config ).files[ "source/main.cpp" ] )


