import json
import sys
import subprocess

def verify_rules_batch( files ):
    results = [ ]
    for file_path in files:
        try:
            process = subprocess.run( 
                [ "python3", "tools/ensure_code_formatting.py", "--verify-rules", file_path ], 
                capture_output=True, text=True, check=True 
            )
            violations = json.loads( process.stdout )
            
            if violations:
                message = f"File: {file_path}\n"
                for violation in violations:
                    message += f"  - {violation}\n"
                results.append( message )
        except Exception as e:
            results.append( f"Error verifying {file_path}: {str(e)}" )
    
    if not results:
        return "No rule violations found in the provided files."
    
    return "\n".join( results ).strip( )

def run_adhoc( params ):
    files = params.get( "files", [ ] )
    return verify_rules_batch( files )

if __name__ == "__main__":
    if len( sys.argv ) > 1:
        params = json.loads( sys.argv[ 1 ] )
        print( run_adhoc( params ) )
