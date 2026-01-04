#!/usr/bin/python3

import sys
import os
import subprocess
import datetime
import time
import re
import json

def get_git_config_value( configuration_name ):
    try:
        command = [ "git", "config", "--global", configuration_name ]
        process = subprocess.run( command, capture_output=True, text=True, check=True )
        return  process.stdout.strip( )
    except subprocess.CalledProcessError:
        return  "value not found"

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
    return  f"created file: {file_path}\n"

def create_class( class_hierarchy, include_list=[], using_list=[], create_header_only=False ):
    message = ""
    hierarchy_list = re.split( r"[/:\\.]+", class_hierarchy )

    data = {
         "num_year": datetime.datetime.now( ).strftime( "%Y" )
        ,"des_full_name": get_git_config_value( "user.name" )
        ,"des_email": get_git_config_value( "user.email" )
        ,"des_formatted_datetime": datetime.datetime.now( ).strftime( "%Y-%m-%d %H:%M" )
        ,"des_file_path": "/".join( hierarchy_list ) + ".hpp"
        ,"class_name": hierarchy_list[-1]
        ,"namespace_list": hierarchy_list[:-1]
        ,"include_list": include_list
        ,"using_list": using_list
        ,"header_guard": f"header_guard_{ str( time.time_ns( ) )[-9:] }"
    }

    content_hpp = render_template( "class-hpp", data )
    file_path_hpp = f"include/{data['des_file_path']}"
    message += write_file( file_path_hpp, content_hpp )

    if not create_header_only:
        data.update( {
             "include_list": [ data[ "des_file_path" ] ]
            ,"des_file_path": "/".join( hierarchy_list ) + ".cpp"
        } )
        content_cpp = render_template( "class-cpp", data )
        file_path_cpp = f"source/{data['des_file_path']}"
        message += write_file( file_path_cpp, content_cpp )

    return message

def create_test( hierarchy, flg_adhoc=False, include_list=[] ):
    message = ""
    
    if flg_adhoc:
        adhoc_dir = "tests/adhoc"
        os.makedirs( adhoc_dir, exist_ok=True )
        
        existing_adhocs = [ d for d in os.listdir( adhoc_dir ) if os.path.isdir( os.path.join( adhoc_dir, d ) ) ]
        max_counter = 0
        for d in existing_adhocs:
            match = re.match( r"(\d+)_", d )
            if match:
                counter = int( match.group( 1 ) )
                if counter > max_counter:
                    max_counter = counter
        
        next_counter = max_counter + 1
        prefix = f"{next_counter:04d}"
        test_folder = f"{prefix}_{hierarchy}"
        file_path = f"{adhoc_dir}/{test_folder}/{prefix}_{hierarchy}.cpp"
        display_hierarchy = hierarchy
    else:
        hierarchy_list = re.split( r"[/:\\.]+", hierarchy )
        path = "/".join( hierarchy_list[:-1] )
        filename = "test_" + "_".join( hierarchy_list ) + ".cpp"
        
        if path:
            file_path = f"tests/{path}/{filename}"
        else:
            file_path = f"tests/{filename}"
        display_hierarchy = hierarchy

    data = {
         "num_year": datetime.datetime.now( ).strftime( "%Y" )
        ,"des_full_name": get_git_config_value( "user.name" )
        ,"des_email": get_git_config_value( "user.email" )
        ,"des_formatted_datetime": datetime.datetime.now( ).strftime( "%Y-%m-%d %H:%M" )
        ,"des_file_path": file_path
        ,"hierarchy": display_hierarchy
        ,"include_list": include_list
    }

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
