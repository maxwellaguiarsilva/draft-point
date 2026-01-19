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
#   File:   project_tree
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-15 10:00:00


import os
import re
import glob
import datetime
from lib.common import ensure, read_file


def parse_hierarchy( hierarchy ):
    return re.split( r"[/:\\.]+", hierarchy )


class include_reference:
    def __init__( self, raw_text, display_text, target_node ):
        self.raw_text = raw_text
        self.display_text = display_text
        self.target_node = target_node
        self.is_redundant = False


class tree_node:
    def __init__( self, path, hierarchy, is_external = False ):
        self.path = path
        self.hierarchy = hierarchy
        self.is_external = is_external
        self.content = None
        self.direct_includes = [ ]
        self.closure = set( )
        self.modified_at = datetime.datetime.min

        if self.path and os.path.exists( self.path ):
            self.modified_at = datetime.datetime.fromtimestamp( os.path.getmtime( self.path ) )
            self.content = read_file( self.path )


class project_tree:
    def __init__( self, config ):
        self.config = config
        self.nodes = { }
        self.hierarchy_map = { }
        self.relative_path_map = { }
        self.include_pattern = re.compile( self.config[ "language" ][ "patterns" ][ "include_directive" ] )
        
        self._scan_project( )
        self._resolve_all_includes( )
        self._calculate_all_closures( )
        self._identify_redundancies( )

    def _scan_project( self ):
        include_dir = self.config[ "paths" ][ "include" ]
        source_dir = self.config[ "paths" ][ "source" ]
        tests_dir = self.config[ "paths" ][ "tests" ]

        self._scan_dir( include_dir, include_dir )
        self._scan_dir( source_dir, source_dir )
        self._scan_dir( tests_dir, tests_dir )

    def _scan_dir( self, root_path, base_path ):
        exts = ( 
             f".{self.config[ 'language' ][ 'header_extension' ]}"
            ,f".{self.config[ 'language' ][ 'source_extension' ]}" 
        )
        pattern = os.path.join( root_path, "**", "*" )
        for path in glob.glob( pattern, recursive=True ):
            if os.path.isfile( path ) and path.endswith( exts ):
                relative_path_from_base = os.path.relpath( path, base_path )
                hierarchy = os.path.splitext( relative_path_from_base )[ 0 ]
                
                node = tree_node( path, hierarchy )
                self.nodes[ path ] = node
                self.relative_path_map[ relative_path_from_base ] = node
                
                #   hierarchy_map can still be used, but it's ambiguous if both .hpp and .cpp exist
                #   we'll prefer .hpp for hierarchy_map if it exists
                if hierarchy not in self.hierarchy_map or path.endswith( f".{self.config[ 'language' ][ 'header_extension' ]}" ):
                    self.hierarchy_map[ hierarchy ] = node

    def _resolve_include( self, include_path ):
        #   first, check if it's a precise relative path
        if include_path in self.relative_path_map:
            return self.relative_path_map[ include_path ]

        #   then, check as a hierarchy (without extension)
        hierarchy = os.path.splitext( include_path )[ 0 ]
        if hierarchy in self.hierarchy_map:
            return self.hierarchy_map[ hierarchy ]
        
        #   otherwise, it's external/system
        if include_path not in self.nodes:
            node = tree_node( None, include_path, is_external = True )
            self.nodes[ include_path ] = node
        return self.nodes[ include_path ]

    def _resolve_all_includes( self ):
        for node in list( self.nodes.values( ) ):
            if not node.content:
                continue
            
            for match in self.include_pattern.finditer( node.content ):
                raw_text = match.group( "full" )
                include_path = match.group( "path" )
                target_node = self._resolve_include( include_path )
                node.direct_includes.append( include_reference( raw_text, raw_text, target_node ) )

    def _calculate_closure( self, node, visited ):
        if node in visited:
            return set( )
        
        if node.closure:
            return node.closure
            
        new_visited = visited | { node }
        closure = set( )
        for ref in node.direct_includes:
            target = ref.target_node
            closure.add( target )
            closure |= self._calculate_closure( target, new_visited )
            
        node.closure = closure
        return closure

    def _calculate_all_closures( self ):
        for node in self.nodes.values( ):
            self._calculate_closure( node, set( ) )

    def _identify_redundancies( self ):
        for node in self.nodes.values( ):
            for i, ref in enumerate( node.direct_includes ):
                target = ref.target_node
                for j, other_ref in enumerate( node.direct_includes ):
                    if i == j:
                        continue
                    if target in other_ref.target_node.closure:
                        ref.is_redundant = True
                        break


