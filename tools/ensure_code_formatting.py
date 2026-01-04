import re

class formatter:
    def __init__( self, content: str ):
        self.content = content
        self.messages = []

    def run( self ):
        self._consecutive_newlines( )
        self._return_spacing( )
        self._trailing_newlines( )
        self._include_spacing( )
        return self.content

    def _apply( self, pattern: str, replacement: str, message: str, flags: int = 0 ):
        new_content = re.sub( pattern, replacement, self.content, flags = flags )
        if new_content != self.content:
            self.content = new_content
            self.messages.append( message )

    def _consecutive_newlines( self ):
        self._apply( r'\n{4,}', '\n\n\n', "adjusted consecutive newlines" )

    def _return_spacing( self ):
        self._apply( r'([ \t])return\b(?![ \t]*;)[ \t]*', r'\1return\1', "adjusted return spacing" )

    def _trailing_newlines( self ):
        new_content = self.content.rstrip( ) + "\n\n\n"
        if new_content != self.content:
            self.content = new_content
            self.messages.append( "adjusted trailing whitespace and newlines" )

    def _include_spacing( self ):
        self._apply( 
            r'(^#include\s+.*)\n(?:[ \t]*\n)+(?=#include\s+.*)', 
            r'\1\n', 
            "adjusted include directives spacing", 
            flags = re.MULTILINE 
        )

