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
#   File:   file_generator
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-04 19:36:49


import sys
import os
import subprocess
import datetime
import time
import re
import json

def strip_project_prefix( path ):
    """Remove prefixos de diretório para exibição limpa no cabeçalho."""
    prefixes = [ "include/", "source/", "tests/" ]
    for p in prefixes:
        if path.startswith( p ):
            return path[ len( p ) : ]
    return path

def fetch_git_first_commit( file_path ):
    """Recupera data, nome e email do primeiro commit de um arquivo via Git."""
    if not os.path.exists( file_path ):
        return None
    try:
        #   Formato: YYYY-MM-DD HH:MM|Nome|Email
        cmd = [
            "git", "log", "--follow", "--reverse", 
            "--date=format:%Y-%m-%d %H:%M", 
            "--format=%ad|%an|%ae", "--", file_path
        ]
        result = subprocess.run( cmd, capture_output=True, text=True, check=True )
        if not result.stdout.strip( ):
            return None
        first_line = result.stdout.splitlines( )[ 0 ]
        dt, name, email = first_line.split( '|' )
        return {
            "year": dt.split( '-' )[ 0 ],
            "datetime": dt,
            "name": name,
            "email": email
        }
    except Exception:
        return None

def get_canonical_metadata( full_relative_path ):
    """Gera o conjunto de metadados oficial para um arquivo (existente ou novo)."""
    canonical_path = strip_project_prefix( full_relative_path )
    git_info = fetch_git_first_commit( full_relative_path )
    
    if git_info:
        return {
             "num_year": git_info[ "year" ]
            ,"des_full_name": git_info[ "name" ]
            ,"des_email": git_info[ "email" ]
            ,"des_formatted_datetime": git_info[ "datetime" ]
            ,"des_file_path": canonical_path
        }
    
    #   Fallback para arquivos novos (ainda não commitados)
    return {
         "num_year": datetime.datetime.now( ).strftime( "%Y" )
        ,"des_full_name": get_git_config_value( "user.name" )
        ,"des_email": get_git_config_value( "user.email" )
        ,"des_formatted_datetime": datetime.datetime.now( ).strftime( "%Y-%m-%d %H:%M" )
        ,"des_file_path": canonical_path
    }

def get_git_config_value( configuration_name ):
    try:
        command = [ "git", "config", "--global", configuration_name ]
        process = subprocess.run( command, capture_output=True, text=True, check=True )
        return process.stdout.strip( )
    except subprocess.CalledProcessError:
        return "value not found"

def render_template( template_name, data ):
    data_json = json.dumps( data )
    process = subprocess.run( 
        [ "python3", "tools/template.py", template_name, data_json ], 
        capture_output=True, 
        text=True, 
        check=True 
    )
    return process.stdout

def write_file( file_path, content ):
    os.makedirs( os.path.dirname( file_path ), exist_ok=True )
    with open( file_path, "w" ) as f:
        f.write( content )
    return f"created file: {file_path}\n"

def create_class( class_hierarchy, include_list=[], using_list=[], create_header_only=False ):
    message = ""
    hierarchy_list = re.split( r"[/:\\.]+", class_hierarchy )

    file_path_hpp = f"include/{ '/'.join( hierarchy_list ) }.hpp"
    data = get_canonical_metadata( file_path_hpp )
    data.update( {
         "class_name": hierarchy_list[ -1 ]
        ,"namespace_list": hierarchy_list[ :-1 ]
        ,"include_list": include_list
        ,"using_list": using_list
        ,"header_guard": f"header_guard_{ str( time.time_ns( ) )[ -9: ] }"
    } )

    content_hpp = render_template( "class-hpp", data )
    message += write_file( file_path_hpp, content_hpp )

    if not create_header_only:
        file_path_cpp = f"source/{ '/'.join( hierarchy_list ) }.cpp"
        data = get_canonical_metadata( file_path_cpp )
        data.update( {
             "include_list": [ strip_project_prefix( file_path_hpp ) ]
            ,"class_name": hierarchy_list[ -1 ]
            ,"namespace_list": hierarchy_list[ :-1 ]
            ,"using_list": using_list
        } )
        content_cpp = render_template( "class-cpp", data )
        message += write_file( file_path_cpp, content_cpp )

    return message

def create_test( hierarchy, flg_adhoc=False, include_list=[] ):
    message = ""
    
    if flg_adhoc:
        adhoc_dir = "tests/adhoc"
        os.makedirs( adhoc_dir, exist_ok=True )
        
        existing_adhocs = [ d for d in os.listdir( adhoc_dir ) if os.path.isdir( os.path.join( adhoc_dir, d ) ) ]
        existing_counters = set( )
        for d in existing_adhocs:
            match = re.match( r"(\d+)_", d )
            if match:
                existing_counters.add( int( match.group( 1 ) ) )
        
        next_counter = 1
        while next_counter in existing_counters:
            next_counter += 1
        
        prefix = f"{next_counter:04d}"
        test_folder = f"{prefix}_{hierarchy}"
        file_path = f"{adhoc_dir}/{test_folder}/{prefix}_{hierarchy}.cpp"
        display_hierarchy = hierarchy
    else:
        hierarchy_list = re.split( r"[/:\\.]+", hierarchy )
        path = "/".join( hierarchy_list[ :-1 ] )
        filename = "test_" + "_".join( hierarchy_list ) + ".cpp"
        
        if path:
            file_path = f"tests/{path}/{filename}"
        else:
            file_path = f"tests/{filename}"
        display_hierarchy = hierarchy

    data = get_canonical_metadata( file_path )
    data.update( {
         "hierarchy": display_hierarchy
        ,"include_list": include_list
    } )

    content_test = render_template( "test-cpp", data )
    message += write_file( file_path, content_test )
    return message

if __name__ == "__main__":
    if len( sys.argv ) < 2:
        sys.exit( 1 )
    
    command = sys.argv[ 1 ]
    args = json.loads( sys.argv[ 2 ] ) if len( sys.argv ) > 2 else { }

    if command == "create_class":
        print( create_class( **args ) )
    elif command == "create_test":
        print( create_test( **args ) )
