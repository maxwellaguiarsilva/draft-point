import re
import sys
import json
import file_generator

class formatter:
    def __init__( self, content: str, file_path: str = None, flg_fix: bool = True ):
        self.content = content
        self.file_path = file_path
        self.messages = []
        self.flg_fix = flg_fix

    def run( self ):
        self._validate_license( )
        self._consecutive_newlines( )
        self._return_spacing( )
        self._trailing_newlines( )
        self._include_spacing( )
        self._bracket_spacing( )
        return self.content

    def verify( self ):
        self.flg_fix = False
        self.run( )
        return self.messages

    def _apply( self, pattern: str, replacement: str, message: str, flags: int = 0 ):
        if self.flg_fix:
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
                    self.messages.append( ( line_no, message ) )

    def _validate_license( self ):
        if not self.file_path:
            return

        try:
            #   1. Gera o cabeçalho ideal baseado nos metadados canônicos
            data = file_generator.get_canonical_metadata( self.file_path )
            ideal_header = file_generator.render_template( "file-header", data ).strip( )
            
            #   2. Extrai o cabeçalho atual (até o primeiro double newline)
            parts = self.content.split( '\n\n', 1 )
            actual_header = parts[ 0 ].strip( )
            
            if actual_header != ideal_header:
                if self.flg_fix:
                    #   Restaura o cabeçalho mantendo o corpo do arquivo
                    body = parts[ 1 ] if len( parts ) > 1 else ""
                    self.content = ideal_header + "\n\n" + body
                    self.messages.append( f"restored canonical license header for {self.file_path}" )
                else:
                    self.messages.append( ( 1, f"license header mismatch in {self.file_path}" ) )
        except Exception as e:
            self.messages.append( ( 0, f"license validation skipped: {e}" ) )

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
            if self.flg_fix:
                self.content = new_content
                self.messages.append( "file must end with exactly 2 empty lines and no trailing whitespace" )
            else:
                line_no = self.content.count( '\n' ) + 1
                self.messages.append( ( line_no, "file must end with exactly 2 empty lines and no trailing whitespace" ) )

    def _include_spacing( self ):
        self._apply( 
            r'(^#include\s+.*)\n(?:[ \t]*\n)+(?=#include\s+.*)', 
            r'\1\n', 
            "include directives must not be separated by empty lines" ,
            flags = re.MULTILINE 
        )

    def _bracket_spacing( self ):
        #   We apply spacing rules only to the body (after the license header)
        split_index = self.content.find( '\n\n' )
        if split_index == -1:
            return

        header = self.content[ :split_index + 2 ]
        body = self.content[ split_index + 2 : ]
        
        #   Regex patterns for ( space ) and [ space ]
        #   1. (not_space -> ( space
        #   2. not_space) -> space )
        patterns = [
             ( r'\((?![ \t\n\)])', r'( ', "missing space after '('" )
            ,( r'(?<![ \t\n\(])\)', r' )', "missing space before ')'" )
            ,( r'\[(?![ \t\n\]])', r'[ ', "missing space after '['" )
            ,( r'(?<![ \t\n\[])\]', r' ]', "missing space before ']'" )
        ]

        original_body = body
        for pattern, replacement, message in patterns:
            if self.flg_fix:
                body = re.sub( pattern, replacement, body )
            else:
                for match in re.finditer( pattern, body ):
                    line_no = header.count( '\n' ) + original_body.count( '\n', 0, match.start( ) ) + 1
                    self.messages.append( ( line_no, message ) )

        if self.flg_fix and body != original_body:
            self.content = header + body
            self.messages.append( "fixed bracket spacing ( ( space ) and [ space ] rules )" )


if __name__ == "__main__":
    if len( sys.argv ) > 2:
        command = sys.argv[ 1 ]
        file_path = sys.argv[ 2 ]
        try:
            with open( file_path, 'r' ) as f:
                content = f.read( )
            
            if command == "--formatting":
                fmt = formatter( content, file_path = file_path )
                violations = fmt.verify( )
                print( json.dumps( violations ) )
            elif command == "--fix":
                fmt = formatter( content, file_path = file_path )
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

