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
#   File:   docs/agent/analyses/opengl/opengl-tag/src/opengl_tag/__init__.py
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-08-24 14:27:21
#   


import argparse
from pathlib import Path

from .db import tag_database
from .pipeline import CONSTANTS_TAG, FUNCTIONS_TAG, TYPES_TAG, run

#   paths relative to this uv project (docs/agent/analyses/opengl/opengl-tag/)
_DEFAULT_HEADER = (
    Path( __file__ ).resolve( ).parents[ 7 ]
    / "include" / "sak" / "opengl" / "glad" / "snake_case.hpp"
)
_DEFAULT_GL_HEADER = (
    Path( __file__ ).resolve( ).parents[ 7 ]
    / "include" / "sak" / "opengl" / "glad" / "gl.h"
)
_DEFAULT_DB = Path( __file__ ).resolve( ).parents[ 2 ] / "opengl_tag.db"
_DEFAULT_GL_XML = Path( __file__ ).resolve( ).parents[ 2 ] / "gl.xml"


def main( ) -> None:
    parser = argparse.ArgumentParser(
        description = "register opengl objects into the tag database"
    )
    parser.add_argument(
        "--header",
        type = Path,
        default = _DEFAULT_HEADER,
        help = f"snake_case glad header (default: {_DEFAULT_HEADER})",
    )
    parser.add_argument(
        "--gl-header",
        type = Path,
        default = _DEFAULT_GL_HEADER,
        help = f"glad gl.h header (default: {_DEFAULT_GL_HEADER})",
    )
    parser.add_argument(
        "--db",
        type = Path,
        default = _DEFAULT_DB,
        help = f"sqlite database path (default: {_DEFAULT_DB})",
    )
    parser.add_argument(
        "--gl-xml",
        type = Path,
        default = _DEFAULT_GL_XML,
        help = f"khronos gl.xml path (default: {_DEFAULT_GL_XML})",
    )
    parser.add_argument(
        "--reset",
        action = "store_true",
        help = "recreate the database from scratch before registering",
    )
    parser.add_argument(
        "--count-by-tag",
        action = "store_true",
        help = "print the number of objects per tag and exit",
    )
    parser.add_argument(
        "--tag",
        type = str,
        help = "list all objects belonging to a tag and exit",
    )
    args = parser.parse_args( )

    if args.tag:
        with tag_database( args.db ) as db:
            for obj in db.objects_of( args.tag ):
                if obj.original_name:
                    print( f"{obj.name}\t({obj.original_name})" )
                else:
                    print( obj.name )
        return

    if args.count_by_tag:
        with tag_database( args.db ) as db:
            for item in db.count_by_tag( ):
                print( f"{item.name}\t{item.count}" )
        return

    count = run( args.header, args.gl_header, args.db, args.gl_xml, reset = args.reset )
    print(
        f"registered {count} objects "
        f"({FUNCTIONS_TAG}/{CONSTANTS_TAG}/{TYPES_TAG})"
    )


