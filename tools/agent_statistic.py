import json
import os
import sys

STATISTIC_FILE = "/home/.gemini/statistic.json"

def format_table( data ):
    if not data:
        return "No statistics recorded yet."
    
    # Sort by count descending
    sorted_data = sorted( data, key=lambda x: x[ "count" ], reverse=True )
    
    # Calculate column widths
    name_width = max( len( item[ "name" ] ) for item in sorted_data )
    name_width = max( name_width, 4 ) # "name"
    
    desc_width = max( len( item[ "short-description" ] ) for item in sorted_data )
    desc_width = max( desc_width, 17 ) # "short-description"
    
    count_width = max( len( str( item[ "count" ] ) ) for item in sorted_data )
    count_width = max( count_width, 5 ) # "count"
    
    header = f"| {'name':<{name_width}} | {'short-description':<{desc_width}} | {'count':<{count_width}} |"
    separator = f"|{'-' * ( name_width + 2 )}|{'-' * ( desc_width + 2 )}|{'-' * ( count_width + 2 )}|"
    
    lines = [ header, separator ]
    for item in sorted_data:
        lines.append( f"| {item[ 'name' ]:<{name_width}} | {item[ 'short-description' ]:<{desc_width}} | {item[ 'count' ]:>{count_width}} |" )
    
    return "\n".join( lines )

def run_statistic( params ):
    # Validate allowed fields
    allowed_fields = { "name", "short-description" }
    for key in params:
        if key not in allowed_fields:
            return f"Error: field '{key}' is not allowed. Only 'name' and 'short-description' are accepted."

    # Ensure directory exists
    os.makedirs( os.path.dirname( STATISTIC_FILE ), exist_ok=True )
    
    if os.path.exists( STATISTIC_FILE ):
        try:
            with open( STATISTIC_FILE, "r" ) as f:
                data = json.load( f )
        except ( json.JSONDecodeError, IOError ):
            data = [ ]
    else:
        data = [ ]

    name = params.get( "name" )
    short_description = params.get( "short-description" )
    
    if name:
        # Normalize name: lower, trim, spaces to hyphens
        name = name.lower( ).strip( ).replace( " ", "-" )
        
        # Normalize description if provided: lower, trim, check for "."
        if short_description:
            short_description = short_description.lower( ).strip( )
            if "." in short_description:
                return "Error: short-description cannot contain periods ('.')."
        
        # Find entry
        entry = next( ( item for item in data if item[ "name" ] == name ), None )
        
        if entry:
            entry[ "count" ] += 1
            if short_description:
                entry[ "short-description" ] = short_description
        else:
            if not short_description:
                return f"Error: entry '{name}' does not exist. Please provide 'short-description' to create it."
            
            new_entry = {
                "name": name,
                "short-description": short_description,
                "count": 1
            }
            data.append( new_entry )
            
        # Save data
        with open( STATISTIC_FILE, "w" ) as f:
            json.dump( data, f, indent="\t" )
            
    return format_table( data )

if __name__ == "__main__":
    try:
        if len( sys.argv ) > 1:
            params = json.loads( sys.argv[ 1 ] )
            print( run_statistic( params ) )
        else:
            print( run_statistic( { } ) )
    except Exception as e:
        print( f"Error: {str( e )}" )
