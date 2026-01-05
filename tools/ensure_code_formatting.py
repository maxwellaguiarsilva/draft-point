import re
import sys
import json

class formatter:
    def __init__( self, content: str, mode: str = "fix" ):
        self.content = content
        self.messages = []
        self.mode = mode

    def run( self ):
        self._consecutive_newlines( )
        self._return_spacing( )
        self._trailing_newlines( )
        self._include_spacing( )
        return self.content

    def verify( self ):
        self.mode = "verify"
        self.run( )
        return self.messages

    def _apply( self, pattern: str, replacement: str, message: str, flags: int = 0 ):
        if self.mode == "fix":
            new_content = re.sub( pattern, replacement, self.content, flags = flags )
            if new_content != self.content:
                self.content = new_content
                self.messages.append( message )
        else:
            for match in re.finditer( pattern, self.content, flags = flags ):
                old_text = match.group( 0 )
                new_text = re.sub( pattern, replacement, old_text, flags = flags )
                if old_text != new_text:
                    line_no = self.content.count( '\n', 0, match.start( ) ) + 1
                    self.messages.append( f"Line {line_no}: Violation: {message}" )

    def _consecutive_newlines( self ):
        self._apply( 
            r'\n{4,}', 
            '\n\n\n', 
            "too many consecutive empty lines (maximum 2 allowed)" 
        )

    def _return_spacing( self ):
        self._apply( 
            r'([ \t])return\b(?![ \t]*;)[ \t]*', 
            r'\1return\1', 
            "return must be followed by exactly one space or tab (matching the preceding indentation character)" 
        )

    def _trailing_newlines( self ):
        new_content = self.content.rstrip( ) + "\n\n\n"
        if new_content != self.content:
            if self.mode == "fix":
                self.content = new_content
            self.messages.append( "file must end with exactly 2 empty lines and no trailing whitespace" )

    def _include_spacing( self ):
        self._apply( 
            r'(^#include\s+.*)\n(?:[ \t]*\n)+(?=#include\s+.*)', 
            r'\1\n', 
            "include directives must not be separated by empty lines" ,
            flags = re.MULTILINE 
        )


def verify_spacing( content: str ) -> list[ tuple[ int, str ] ]:
    violations = [ ]
    lines = content.splitlines( )
    
    p1 = re.compile( r'\((?![ \t\n\)])' )
    p2 = re.compile( r'(?<![ \t\n\(])\)' )
    p3 = re.compile( r'\[(?![ \t\n\]])' )
    p4 = re.compile( r'(?<![ \t\n\[])\]' )
    
    for i, line in enumerate( lines, 1 ):
        if p1.search( line ) or p2.search( line ) or p3.search( line ) or p4.search( line ):
            violations.append( ( i, line.strip( ) ) )
    
    return violations


if __name__ == "__main__":
    if len( sys.argv ) > 2:
        command = sys.argv[ 1 ]
        file_path = sys.argv[ 2 ]
        try:
            with open( file_path, 'r' ) as f:
                content = f.read( )
            
            if command == "--verify":
                violations = verify_spacing( content )
                print( json.dumps( violations ) )
            elif command == "--verify-rules":
                fmt = formatter( content )
                violations = fmt.verify( )
                print( json.dumps( violations ) )
            elif command == "--fix":
                fmt = formatter( content )
                new_content = fmt.run( )
                result = {
                    "content": new_content,
                    "messages": fmt.messages,
                    "changed": content != new_content
                }
                print( json.dumps( result ) )
        except Exception as e:
            print( f"Error: {e}", file=sys.stderr )
            sys.exit( 1 )

