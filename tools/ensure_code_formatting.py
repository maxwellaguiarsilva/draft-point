import re


def apply_rule( content: str, rule_func, message: str, messages: list ) -> str:
    new_content = rule_func( content )
    if new_content != content:
        messages.append( message )
    return new_content

def rule_consecutive_newlines( content: str ) -> str:
    return re.sub( r'\n{4,}', '\n\n\n', content )

def rule_return_spacing( content: str ) -> str:
    return re.sub( r'([ \t])return\b(?![ \t]*;)[ \t]*', r'\1return\1', content )

def rule_trailing_newlines( content: str ) -> str:
    return content.rstrip( ) + "\n\n\n"

def rule_include_spacing( content: str ) -> str:
    return re.sub( r'(^#include\s+.*)\n(?:[ \t]*\n)+(?=#include\s+.*)', r'\1\n', content, flags = re.MULTILINE )

def ensure_code_formatting( content: str ) -> tuple:
    messages = []
    content = apply_rule( content, rule_consecutive_newlines, "adjusted consecutive newlines", messages )
    content = apply_rule( content, rule_return_spacing, "adjusted return spacing", messages )
    content = apply_rule( content, rule_trailing_newlines, "adjusted trailing whitespace and newlines", messages )
    content = apply_rule( content, rule_include_spacing, "adjusted include directives spacing", messages )
    
    return content, messages