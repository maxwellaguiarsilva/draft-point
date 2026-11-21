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
#   File:   docs/agent/analyses/opengl/opengl-tag/src/opengl_tag/pipeline.py
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-08-23 23:56:12
#   


from pathlib import Path

from sak.common import create_process

from .db import tag_database
from .extract import (
    bitmask_group_names,
    constant_groups,
    constant_names,
    function_class_owners,
    function_param_signatures,
    function_signature_classes,
    function_groups,
    function_names,
    function_originals,
    type_names,
)

#   tag guaranteed to apply to every object extracted from the snake_case
#   wrapper: they are all OpenGL functions
FUNCTIONS_TAG = "functions"
CONSTANTS_TAG = "constants"
TYPES_TAG = "types"

#   derived root tag: union of every gl.xml `group` tag
GROUP_TAG = "group"

#   derived root tag: union of every gl.xml `class` tag
CLASS_TAG = "class"

#   bitmask sets: `bitmask-groups` = union of the bitmask group tags;
#   `bitmask` = constants that belong to a bitmask group
BITMASK_GROUPS_TAG = "bitmask-groups"
BITMASK_TAG = "bitmask"

#   bind-to-edit: the closed set of functions that Khronos discourages in
#   favor of a more modern approach (DSA named functions, `create_*`, or the
#   GLSL `layout(location=...)` qualifier). A physical tag derived entirely
#   from name prefixes and the `gl.xml` signature, never enumerated:
#   `gen_*` / `framebuffer_texture*` / `framebuffer_renderbuffer` /
#   `tex_buffer*` are clean prefixes; the ambiguous `bind_*` family is split
#   by `is_modern_bind` (unit/index/multi binds stay, target/context/locator
#   binds are excluded).
BIND_TO_EDIT_TAG = "bind-to-edit"

#   derived tag: the modern object surface = the class-owner scope minus the
#   bind-to-edit set
DIRECT_STATE_ACCESS_TAG = "direct-state-access"

#   dsa-<class> derived tags: intersection of `direct-state-access` with each
#   `class-<class>` owner scope; the DSA surface scoped to one class
DSA_PREFIX = "dsa-"
CLASS_PREFIX = "class-"

#   draw/capture pipeline bind classes whose current bind has no modern
#   substitute: their target binds (`bind_framebuffer`, `bind_transform_feedback`)
#   are mandatory to render into / capture with, so they stay in the surface
#   (like the single-handle `bind_vertex_array` / `bind_program_pipeline`).
#   keyed by the `class-<class>` owner tag from `function_class_owners`.
BIND_PIPELINE_CLASSES = { "class-framebuffer", "class-transform-feedback" }

#   RAII tiers: unions over the `dsa-*` sub-tags
DSA_SINGLE_RAII_TAG = "dsa-single-raii"
DSA_LIST_RAII_TAG = "dsa-list-raii"
DSA_SINGLE_RAII_CLASSES = { "shader", "program" }
DSA_LIST_RAII_CLASSES = {
    "buffer",
    "texture",
    "framebuffer",
    "vertex-array",
    "renderbuffer",
    "sampler",
    "transform-feedback",
    "query",
}


def _is_modern_bind( snake: str, params: list[ tuple[ str, str ] ] ) -> bool:
    """whether a `gl_bind_*` function binds to a unit/index slot (modern) rather
    than to the class target / context / a name-location (bind-to-edit)

    `snake` is the snake_case name; `params` is the ordered (base type, name)
    signature from gl.xml. rule-only, no enumeration:
    - flg_location   : "location" token in the name (glsl layout qualifier replaces it)
    - flg_index      : an `index` / `first` parameter (indexed or multi bind)
    - flg_target_bind: a `GLenum target` first parameter without an index slot
    a single-handle bind (e.g. `bind_vertex_array`, `bind_program_pipeline`) is
    a draw-pipeline bind with no modern substitute, so it stays in the surface"""
    flg_location = "location" in snake
    param_names = { name for _, name in params }
    flg_index = "index" in param_names or "first" in param_names
    flg_enum_target = bool( params ) and params[ 0 ][ 0 ] == "GLenum"
    flg_target_bind = flg_enum_target and not flg_index
    return  not ( flg_location or flg_target_bind )


def _bind_to_edit_names(
    original_to_snake: dict[ str, str ],
    param_signatures: dict[ str, list[ tuple[ str, str ] ] ],
    owner_classes: dict[ str, set[ str ] ] | None = None,
) -> set[ str ]:
    """snake_case names of the bind-to-edit set, from clean prefixes and the
    bind-to-edit rule for the ambiguous `bind_*` family"""
    result: set[ str ] = set( )
    for original, snake in original_to_snake.items( ):
        body = snake[ len( "gl_" ): ]
        if body.startswith( "gen_" ):
            result.add( snake )
        elif body.startswith( "framebuffer_texture" ) or body == "framebuffer_renderbuffer":
            result.add( snake )
        elif body.startswith( "tex_buffer" ):
            result.add( snake )
        elif body.startswith( "bind_" ) and not _is_modern_bind(
            snake, param_signatures.get( original, [ ] )
        ):
            owners = owner_classes.get( original, set( ) ) if owner_classes else set( )
            if owners.isdisjoint( BIND_PIPELINE_CLASSES ):
                result.add( snake )
    return  result

#   canonical khronos gl.xml shipped by glad2, fetched on demand when missing
GL_XML_URL = (
    "https://raw.githubusercontent.com/Dav1dde/glad/"
    "refs/heads/glad2/glad/files/gl.xml"
)


def _register(
    db: tag_database,
    names: list[ str ],
    tag: str,
    count: int,
    originals: dict[ str, str ] | None = None,
) -> int:
    objects = [
        ( name, originals.get( name ) if originals else None ) for name in names
    ]
    db.add_objects( objects )
    db.link_many( [ ( name, tag ) for name in names ] )
    return  count + len( names )


def _link_groups(
    db: tag_database,
    groups: dict[ str, set[ str ] ],
    to_db_name: dict[ str, str ] | None = None,
) -> set[ str ]:
    """create the group tags and link every object to its semantic groups

    `groups` is keyed by the gl.xml name; `to_db_name` translates it to the
    database `name` when the two differ (functions: camelCase -> snake_case)
    returns the group tag names created so the `group` root can be derived"""
    group_names: set[ str ] = set( )
    links: list[ tuple[ str, str ] ] = [ ]
    for name, group_names_of_object in groups.items( ):
        db_name = to_db_name[ name ] if to_db_name else name
        for group in sorted( group_names_of_object ):
            group_names.add( group )
            links.append( ( db_name, group ) )
    db.add_tags( sorted( group_names ) )
    db.link_many( links )
    return  group_names


def _ensure_gl_xml( gl_xml: str | Path ) -> None:
    """download gl.xml via curl if it is not present locally yet"""
    path = Path( gl_xml )
    if path.exists( ):
        return
    create_process(
        [ "curl", "--location", "--fail", "--output", str( path ), GL_XML_URL ]
    )


def run(
    header: str | Path,
    gl_header: str | Path,
    db_path: str | Path,
    gl_xml: str | Path,
    reset: bool = False,
) -> int:
    """register every function, constant and type into the tag database

    ensures gl.xml is present (downloading it via curl when missing), then
    registers; returns the number of objects registered; idempotent: re-running
    against the same headers and database adds nothing"""
    _ensure_gl_xml( gl_xml )

    if reset:
        Path( db_path ).unlink( missing_ok = True )

    with tag_database( db_path ) as db:
        db.add_tags( [ FUNCTIONS_TAG, CONSTANTS_TAG, TYPES_TAG ] )
        count = 0
        count = _register(
            db,
            function_names( header ),
            FUNCTIONS_TAG,
            count,
            function_originals( header ),
        )
        count = _register( db, constant_names( gl_header ), CONSTANTS_TAG, count )
        count = _register( db, type_names( gl_header ), TYPES_TAG, count )

        #   semantic labeling from gl.xml: `group` attribute only; skip gl.xml
        #   objects absent from the glad headers (GLX, non-core, non-API)
        original_to_snake = {
            original: snake for snake, original in function_originals( header ).items( )
        }
        function_groups_map = function_groups( gl_xml )
        function_group_names = _link_groups(
            db,
            { name: groups for name, groups in function_groups_map.items( ) if name in original_to_snake },
            original_to_snake,
        )
        constant_groups_map = constant_groups( gl_xml )
        existing_constants = set( constant_names( gl_header ) )
        constant_group_names = _link_groups(
            db,
            { name: groups for name, groups in constant_groups_map.items( ) if name in existing_constants },
        )

        #   `class` object tags: every `class` of a function's signature
        #   (proto + params), linked to the functions present in the headers
        signature_classes_map = function_signature_classes( gl_xml )
        signature_class_names = _link_groups(
            db,
            { name: classes for name, classes in signature_classes_map.items( ) if name in original_to_snake },
            original_to_snake,
        )

        #   class owner tags: primary receiver/constructor class per function
        function_class_owners_map = function_class_owners( gl_xml )
        owner_class_names = _link_groups(
            db,
            {
                name: owners
                for name, owners in function_class_owners_map.items( )
                if name in original_to_snake
            },
            original_to_snake,
        )

        #   the `class` root is a derived tag: union of the class-owner tags only
        db.add_tag( CLASS_TAG )
        db.add_rules(
            [
                ( CLASS_TAG, "union", cls )
                for cls in sorted( owner_class_names )
            ]
        )

        #   bind-to-edit: a physical tag derived entirely from name prefixes
        #   and the gl.xml signature (no enumeration)
        db.add_tag( BIND_TO_EDIT_TAG )
        db.link_many(
            [
                ( name, BIND_TO_EDIT_TAG )
                for name in sorted(
                    _bind_to_edit_names(
                        original_to_snake,
                        function_param_signatures( gl_xml ),
                        function_class_owners( gl_xml ),
                    )
                )
            ]
        )

        #   direct-state-access: the class-owner scope minus bind-to-edit.
        #   `intersection` seeds with every object, so the fold is
        #   (all ∩ class) - bind-to-edit = class - bind-to-edit.
        db.add_tag( DIRECT_STATE_ACCESS_TAG )
        db.add_rules(
            [
                ( DIRECT_STATE_ACCESS_TAG, "intersection", CLASS_TAG ),
                ( DIRECT_STATE_ACCESS_TAG, "difference", BIND_TO_EDIT_TAG ),
            ]
        )

        #   dsa-<class>: the DSA surface scoped to one class, derived by
        #   intersection over the `class-<class>` owner tags.
        for cls in sorted( owner_class_names ):
            if cls.startswith( CLASS_PREFIX ):
                dsa_tag = DSA_PREFIX + cls[ len( CLASS_PREFIX ): ]
                db.add_tag( dsa_tag )
                db.add_rules(
                    [
                        ( dsa_tag, "intersection", DIRECT_STATE_ACCESS_TAG ),
                        ( dsa_tag, "intersection", cls ),
                    ]
                )

        #   RAII tiers: unions over the `dsa-*` sub-tags of the lifecycle and
        #   inherently-direct classes.
        db.add_tag( DSA_SINGLE_RAII_TAG )
        db.add_rules(
            [
                ( DSA_SINGLE_RAII_TAG, "union", DSA_PREFIX + cls )
                for cls in sorted( DSA_SINGLE_RAII_CLASSES )
            ]
        )
        db.add_tag( DSA_LIST_RAII_TAG )
        db.add_rules(
            [
                ( DSA_LIST_RAII_TAG, "union", DSA_PREFIX + cls )
                for cls in sorted( DSA_LIST_RAII_CLASSES )
            ]
        )

        #   the `group` root is a derived tag: union of every group tag
        db.add_tag( GROUP_TAG )
        db.add_rules(
            [
                ( GROUP_TAG, "union", group )
                for group in sorted( function_group_names | constant_group_names )
            ]
        )

        #   bitmask sets derived from gl.xml `type="bitmask"` group tags;
        #   only groups that already exist as tags are folded (vendor-only
        #   bitmask groups have no registered object, so no tag)
        bitmask_group_tags = bitmask_group_names( gl_xml ) & (
            function_group_names | constant_group_names
        )
        db.add_tag( BITMASK_GROUPS_TAG )
        db.add_rules(
            [
                ( BITMASK_GROUPS_TAG, "union", group )
                for group in sorted( bitmask_group_tags )
            ]
        )
        db.add_tag( BITMASK_TAG )
        db.add_rules(
            [
                ( BITMASK_TAG, "intersection", CONSTANTS_TAG ),
                ( BITMASK_TAG, "intersection", BITMASK_GROUPS_TAG ),
            ]
        )
        return  count


