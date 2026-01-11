import json
import sys
import subprocess

def quick_upload( message ):
    try:
        #   1. Pull latest changes
        #   We use check=True to raise CalledProcessError on failure
        subprocess.run( [ "git", "pull" ], check=True, capture_output=True, text=True )
        
        #   2. Add all changes
        subprocess.run( [ "git", "add", "." ], check=True, capture_output=True, text=True )
        
        #   3. Commit with the provided message
        subprocess.run( [ "git", "commit", "-m", message ], check=True, capture_output=True, text=True )
        
        #   4. Push to remote
        subprocess.run( [ "git", "push" ], check=True, capture_output=True, text=True )
        
        return f"Quick upload successful: `{message}`"
    except subprocess.CalledProcessError as e:
        error_msg = e.stderr if e.stderr else e.stdout
        #   If commit fails because there's nothing to commit, we can handle it
        if "nothing to commit" in error_msg.lower( ):
             return "Quick upload aborted: Nothing to commit."
        return f"Quick upload failed at command: {' '.join(e.cmd)}\nError: {error_msg}"
    except Exception as e:
        return f"An unexpected error occurred during quick upload: {str(e)}"

if __name__ == "__main__":
    if len( sys.argv ) > 1:
        try:
            params = json.loads( sys.argv[ 1 ] )
            message = params.get( "message" )
            if message:
                print( quick_upload( message ) )
            else:
                print( "Error: 'message' parameter is required." )
        except json.JSONDecodeError:
            print( "Error: Invalid JSON parameters." )
    else:
        print( "Error: No parameters provided." )
