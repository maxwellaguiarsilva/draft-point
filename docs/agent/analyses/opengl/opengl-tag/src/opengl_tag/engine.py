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
#   File:   docs/agent/analyses/opengl/opengl-tag/src/opengl_tag/engine.py
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-08-24 14:27:01
#   


class TagEngine:
    """Evaluates tag memberships and set algebra with memoization and cycle detection.

    A tag with no rules is physical: its members are the direct object links.
    A tag with rules is derived: its members are an ordered left fold over its rules,
    seeded with the identity of the first operation (union -> empty set;
    intersection and difference -> every object).
    """

    def __init__(
        self,
        direct_tag_members: dict[ int, set[ int ] ],
        rules_by_tag: dict[ int, list[ tuple[ str, int ] ] ],
        all_object_ids: set[ int ],
    ):
        self._direct_members = direct_tag_members
        self._rules = rules_by_tag
        self._all_object_ids = all_object_ids
        self._cache: dict[ int, set[ int ] ] = { }
        self._evaluating: set[ int ] = set( )

    def members_of( self, tag_id: int ) -> set[ int ]:
        if tag_id in self._cache:
            return  self._cache[ tag_id ]

        if tag_id in self._evaluating:
            raise   ValueError( f"Circular dependency detected for tag id: {tag_id}" )

        self._evaluating.add( tag_id )
        try:
            rules = self._rules.get( tag_id, [ ] )
            if not rules:
                result = set( self._direct_members.get( tag_id, set( ) ) )
            else:
                first_op, _ = rules[ 0 ]
                accumulator = (
                    set( ) if first_op == "union" else set( self._all_object_ids )
                )
                for op, target_id in rules:
                    target_members = self.members_of( target_id )
                    if op == "union":
                        accumulator |= target_members
                    elif op == "intersection":
                        accumulator &= target_members
                    elif op == "difference":
                        accumulator -= target_members
                    else:
                        raise   ValueError( f"Unknown operation: {op}" )
                result = accumulator

            self._cache[ tag_id ] = result
            return  result
        finally:
            self._evaluating.remove( tag_id )


