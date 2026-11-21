#!/usr/bin/python3

#   
#   Copyright (C) 2026 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#   
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#   
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#   
#   
#   File:   docs/agent/analyses/opengl/opengl-tag/src/opengl_tag/extract.py
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-08-23 23:55:36
#   


import re
import xml.etree.ElementTree as ET
from pathlib import Path


_FUNCTION = re.compile(
    r"^#define\s+(gl_[A-Za-z0-9_]+)\s+glad_(gl[A-Za-z0-9_]+)", re.MULTILINE
)
_CONSTANT = re.compile( r"^#define\s+(GL_[A-Za-z0-9_]+)", re.MULTILINE )
_TYPEDEF = re.compile( r"^typedef\s+(.+?);$", re.MULTILINE )

#   declared-name of a typedef: either `... *NAME` (function/callback pointer)
#   or a trailing `NAME` before the semicolon
_PTR_DECLARED = re.compile( r"\*\s*([A-Za-z_][A-Za-z0-9_]*)\s*\)" )
_TRAIL_DECLARED = re.compile( r"([A-Za-z_][A-Za-z0-9_]*)\s*$" )

#   declared type names that are NOT OpenGL API objects: the glad loader
#   internals, the per-function PFNGL pointers (already covered by the
#   `functions` tag) and the EGL-interop aliases
_EXCLUDED_TYPES = (
    "GLADapiproc",
    "GLADloadfunc",
    "GLADuserptrloadfunc",
    "GLADprecallback",
    "GLADpostcallback",
    "GLADglprocaddrfunc",
    "GLVULKANPROCNV",
    "GLeglClientBufferEXT",
    "GLeglImageOES",
)


def _declared_name( body: str ) -> str | None:
    """extract the declared name of a single-line typedef body"""
    if "*" in body:
        match = _PTR_DECLARED.search( body )
        if match:
            return  match.group( 1 )
    match = _TRAIL_DECLARED.search( body )
    return  match.group( 1 ) if match else None


def _read( path: str | Path ) -> str:
    return  Path( path ).read_text( encoding = "utf-8" )


def function_names( header: str | Path ) -> list[ str ]:
    """distinct snake_case gl_ function names from the glad header"""
    text = _read( header )
    return  sorted( set( snake for snake, _ in _FUNCTION.findall( text ) ) )


def function_originals( header: str | Path ) -> dict[ str, str ]:
    """snake_case gl_ name -> original camelCase name from the glad header"""
    text = _read( header )
    return  dict( _FUNCTION.findall( text ) )


def constant_names( header: str | Path ) -> list[ str ]:
    """distinct GL_ constant names from the glad header"""
    text = _read( header )
    return  sorted( set( _CONSTANT.findall( text ) ) )


def _kebab( name: str ) -> str:
    """camelCase -> lower-kebab-case (e.g. TextureMagFilter -> texture-mag-filter)"""
    return  re.sub(
        r"(?<=[a-z0-9])(?=[A-Z])|(?<=[A-Z])(?=[A-Z][a-z])", "-", name
    ).lower( )


def _split_groups( value: str | None ) -> set[ str ]:
    """comma-separated gl.xml group attribute -> set of prefixed lower-kebab tags"""
    if not value:
        return  set( )
    return  { f"group-{_kebab( group.strip( ) )}" for group in value.split( "," ) if group.strip( ) }


def _split_classes( value: str | None ) -> set[ str ]:
    """gl.xml class attribute -> set of lower-kebab tags
    a class value is a single multi-word token (e.g. "program pipeline")"""
    if not value:
        return  set( )
    return  { re.sub( r"\s+", "-", value.strip( ) ).lower( ) }


def _read_xml( gl_xml: str | Path ) -> ET.Element:
    return  ET.parse( str( gl_xml ) ).getroot( )


def function_groups( gl_xml: str | Path ) -> dict[ str, set[ str ] ]:
    """command original name -> set of lower-kebab group tags

    a command's groups come from the return type (`proto`) and its parameters"""
    root = _read_xml( gl_xml )
    result: dict[ str, set[ str ] ] = { }
    for command in root.findall( ".//command" ):
        name_node = command.find( "proto/name" )
        if name_node is None:
            continue
        groups: set[ str ] = set( )
        proto = command.find( "proto" )
        if proto is not None:
            groups |= _split_groups( proto.get( "group" ) )
        for param in command.findall( "param" ):
            groups |= _split_groups( param.get( "group" ) )
        if groups:
            result.setdefault( name_node.text, set( ) ).update( groups )
    return  result


def function_signature_classes( gl_xml: str | Path ) -> dict[ str, set[ str ] ]:
    """command original name -> set of prefixed object-class tags from its signature

    every `class` of a function's signature becomes a tag prefixed by where it
    appears: `param-<class>` from a parameter, `proto-<class>` from the return
    type. the prefix keeps object-class tags in a namespace separate from the
    `group` semantic tags (e.g. `param-buffer` vs the `buffer` group)"""
    root = _read_xml( gl_xml )
    result: dict[ str, set[ str ] ] = { }
    for command in root.findall( ".//command" ):
        name_node = command.find( "proto/name" )
        if name_node is None:
            continue
        classes: set[ str ] = set( )
        proto = command.find( "proto" )
        if proto is not None:
            classes |= { f"proto-{cls}" for cls in _split_classes( proto.get( "class" ) ) }
        for param in command.findall( "param" ):
            classes |= { f"param-{cls}" for cls in _split_classes( param.get( "class" ) ) }
        if classes:
            result.setdefault( name_node.text, set( ) ).update( classes )
    return  result


def function_class_owners( gl_xml: str | Path ) -> dict[ str, set[ str ] ]:
    """command original name -> single class owner tag

    a function's primary class owner is determined by receiver semantics:
    - `proto` class marks the owner for constructor/creation functions;
    - the first parameter carrying a `class` marks the receiver/owner for operations."""
    root = _read_xml( gl_xml )
    result: dict[ str, set[ str ] ] = { }
    for command in root.findall( ".//command" ):
        name_node = command.find( "proto/name" )
        if name_node is None:
            continue
        proto = command.find( "proto" )
        owner: str | None = None
        if proto is not None and proto.get( "class" ):
            owner_classes = _split_classes( proto.get( "class" ) )
            if owner_classes:
                owner = next( iter( owner_classes ) )
        else:
            for param in command.findall( "param" ):
                if param.get( "class" ):
                    owner_classes = _split_classes( param.get( "class" ) )
                    if owner_classes:
                        owner = next( iter( owner_classes ) )
                    break
        if owner:
            result[ name_node.text ] = { f"class-{owner}" }
    return  result


def function_param_signatures( gl_xml: str | Path ) -> dict[ str, list[ tuple[ str, str ] ] ]:
    """command original name -> ordered list of (base type, param name)

    `base type` is the last `<ptype>` of the parameter (drops qualifiers and
    pointer/array decorations), used to detect a `GLenum target` first
    parameter; `param name` is the `<name>`. supports the bind-to-edit
    `is_modern_bind` rule without a name-token enumeration"""
    root = _read_xml( gl_xml )
    result: dict[ str, list[ tuple[ str, str ] ] ] = { }
    for command in root.findall( ".//command" ):
        name_node = command.find( "proto/name" )
        if name_node is None:
            continue
        params: list[ tuple[ str, str ] ] = [ ]
        for param in command.findall( "param" ):
            types = param.findall( "ptype" )
            base_type = types[ -1 ].text.strip( ) if types else ""
            name_node_of_param = param.find( "name" )
            params.append( ( base_type, name_node_of_param.text.strip( ) if name_node_of_param is not None else "" ) )
        result[ name_node.text ] = params
    return  result


def canonical_function_names(
    header: str | Path, owner_names: set[ str ]
) -> set[ str ]:
    """canonical modernity-tier function names (DSA / immutable-storage)

    applies the canonical token heuristic only within the subset anchored by
    the existing `class-*` owner rules (`owner_names` = camelCase originals
    that carry a class owner); `named` and `sync` match at any position, the
    remaining markers lead the name after the `gl_` prefix, matched as a name
    prefix (so multi-word markers like `get_texture` reach their DSA families).
    everything not matched is left out (deferred as `uncertain`), never
    force-classified"""
    original_to_snake = function_originals( header )
    result: set[ str ] = set( )
    for snake, original in original_to_snake.items( ):
        if original not in owner_names:
            continue
        body = snake[ len( "gl_" ): ]
        tokens = body.split( "_" )
        if any( token in _CANONICAL_ANY_POSITION for token in tokens ):
            result.add( snake )
        elif any(
            body == marker or body.startswith( marker + "_" )
            for marker in _CANONICAL_LEADING
        ):
            result.add( snake )
    return  result


def canonical_function_names(
    header: str | Path, owner_names: set[ str ]
) -> set[ str ]:
    """canonical modernity-tier function names (DSA / immutable-storage)

    applies the canonical token heuristic only within the subset anchored by
    the existing `class-*` owner rules (`owner_names` = camelCase originals
    that carry a class owner); `named` and `sync` match at any position, the
    remaining markers lead the name after the `gl_` prefix, matched as a name
    prefix (so multi-word markers like `get_texture` reach their DSA families).
    everything not matched is left out (deferred as `uncertain`), never
    force-classified"""
    original_to_snake = function_originals( header )
    result: set[ str ] = set( )
    for snake, original in original_to_snake.items( ):
        if original not in owner_names:
            continue
        body = snake[ len( "gl_" ): ]
        tokens = body.split( "_" )
        if any( token in _CANONICAL_ANY_POSITION for token in tokens ):
            result.add( snake )
        elif any(
            body == marker or body.startswith( marker + "_" )
            for marker in _CANONICAL_LEADING
        ):
            result.add( snake )
    return  result


def constant_groups( gl_xml: str | Path ) -> dict[ str, set[ str ] ]:
    """constant name -> set of lower-kebab group tags

    an `<enum>` inherits the enclosing `<enums>` group when it has none of its own"""
    root = _read_xml( gl_xml )
    result: dict[ str, set[ str ] ] = { }
    for enums in root.findall( ".//enums" ):
        default = _split_groups( enums.get( "group" ) )
        for enum in enums.findall( "enum" ):
            name = enum.get( "name" )
            if not name:
                continue
            groups = _split_groups( enum.get( "group" ) ) or default
            if groups:
                result.setdefault( name, set( ) ).update( groups )
    return  result


def bitmask_group_names( gl_xml: str | Path ) -> set[ str ]:
    """set of prefixed lower-kebab group names declared `type="bitmask"` in gl.xml

    only the enclosing `<enums>` blocks carry the bitmask type; individual
    `<enum>`/`<type>` elements never do"""
    root = _read_xml( gl_xml )
    return  {
        f"group-{_kebab( enums.get( 'group' ) )}"
        for enums in root.findall( ".//enums" )
        if enums.get( "type" ) == "bitmask" and enums.get( "group" )
    }


def type_names( header: str | Path ) -> list[ str ]:
    """distinct OpenGL scalar/pointer type names declared via typedef

    excludes the glad loader internals and the per-function PFNGL pointers"""
    text = _read( header )
    names: set[ str ] = set( )
    for line in _TYPEDEF.finditer( text ):
        body = line.group( 1 )
        if "PFNGL" in body:
            continue
        name = _declared_name( body )
        if name and name.startswith( "GL" ) and name not in _EXCLUDED_TYPES:
            names.add( name )
    return  sorted( names )


