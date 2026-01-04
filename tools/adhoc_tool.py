#!/usr/bin/python3

import sys
import json

def run_adhoc( params ):
    """
    Esta função contém a lógica experimental que está sendo testada.
    O dicionário 'params' contém os dados enviados via MCP.
    """
    name = params.get( "name", "Mundo" )
    a = params.get( "a", 0 )
    b = params.get( "b", 0 )
    text_to_reverse = params.get( "text", "" )
    
    result = {
        "status": "success",
        "greeting": f"Olá, {name}!",
        "sum": a + b,
        "reversed_text": text_to_reverse[ ::-1 ],
        "received_params": params
    }
    return json.dumps( result, indent=4 )

if __name__ == "__main__":
    if len( sys.argv ) < 2:
        print( "Error: No parameters provided", file=sys.stderr )
        sys.exit( 1 )
    
    try:
        # Carrega o dicionário de entrada a partir da string JSON
        params = json.loads( sys.argv[ 1 ] )
        print( run_adhoc( params ) )
    except json.JSONDecodeError as e:
        print( f"Error decoding JSON: {e}", file=sys.stderr )
        sys.exit( 1 )
    except Exception as e:
        print( f"Error: {e}", file=sys.stderr )
        sys.exit( 1 )
