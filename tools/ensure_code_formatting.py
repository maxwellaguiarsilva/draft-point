import re

def ensure_code_formatting( content: str ) -> str:
    # Ensure no more than three consecutive newlines anywhere
    new_content = re.sub( r'\n{4,}', '\n\n\n', content )

    # Ensure space/tab after return matches space/tab before it, ignoring return;
    new_content = re.sub( r'([ \t])return\b(?![ \t]*;)[ \t]*', r'\1return\1', new_content )

    # Remove all trailing whitespace and newlines, then add exactly three
    # (one to end the content line, and two more for the empty lines)
    new_content = new_content.rstrip( ) + "\n\n\n"

    return new_content
