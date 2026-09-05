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
#   File:   docs/agent/analyses/opengl/opengl-tag/src/opengl_tag/db.py
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-08-23 10:29:05
#   


import sqlite3
from collections.abc import Iterable
from pathlib import Path
from types import TracebackType

from .engine import TagEngine
from .models import ObjectRecord, Rule, TagCount

#   the operation catalog: fixed set of binary set operations a rule applies
OPERATION_NAMES = ( "union", "intersection", "difference" )


class tag_database:
    """sqlite database that records the tagging of opengl objects

    an object may hold n tags
    a tag marks objects either directly (object_tag) or through an ordered
    fold of rules applied to other tags
    reading an object's tags returns every tag whose membership contains it"""

    schema = """
    CREATE TABLE IF NOT EXISTS object (
        id            INTEGER PRIMARY KEY AUTOINCREMENT,
        name          TEXT NOT NULL UNIQUE,
        original_name TEXT
    );

    CREATE TABLE IF NOT EXISTS tag (
        id   INTEGER PRIMARY KEY AUTOINCREMENT,
        name TEXT NOT NULL UNIQUE
    );

    CREATE TABLE IF NOT EXISTS object_tag (
        object_id INTEGER NOT NULL REFERENCES object( id ) ON DELETE CASCADE,
        tag_id    INTEGER NOT NULL REFERENCES tag( id ) ON DELETE CASCADE,
        PRIMARY KEY ( object_id, tag_id )
    );

    CREATE TABLE IF NOT EXISTS operation (
        id_operation INTEGER PRIMARY KEY AUTOINCREMENT,
        name         TEXT NOT NULL UNIQUE
    );

    CREATE TABLE IF NOT EXISTS rule (
        position     INTEGER NOT NULL,
        id_tag       INTEGER NOT NULL REFERENCES tag( id ) ON DELETE CASCADE,
        id_operation INTEGER NOT NULL REFERENCES operation( id_operation ),
        id_target    INTEGER NOT NULL REFERENCES tag( id ) ON DELETE CASCADE,
        PRIMARY KEY ( position, id_tag )
    );
    """

    def __init__( self, db_path: str | Path ):
        self._connection = sqlite3.connect( str( db_path ) )
        self._connection.row_factory = sqlite3.Row
        self._connection.execute( "PRAGMA foreign_keys = ON" )
        self._connection.executescript( self.schema )
        self._seed_operations( )
        self._connection.commit( )

    def __enter__( self ) -> "tag_database":
        return  self

    def __exit__(
        self,
        exc_type: type[ BaseException ] | None,
        exc_val: BaseException | None,
        exc_tb: TracebackType | None,
    ) -> None:
        if exc_type is None:
            self.commit( )
        self.close( )

    def commit( self ) -> None:
        self._connection.commit( )

    def close( self ) -> None:
        self._connection.close( )

    def add_objects(
        self,
        objects: Iterable[ tuple[ str, str | None ] | ObjectRecord | str ],
    ) -> None:
        """batch insert objects (name, original_name)"""
        params: list[ tuple[ str, str | None ] ] = [ ]
        for item in objects:
            if isinstance( item, str ):
                params.append( ( item, None ) )
            elif isinstance( item, ObjectRecord ):
                params.append( ( item.name, item.original_name ) )
            elif isinstance( item, ( tuple, list ) ):
                params.append( ( item[ 0 ], item[ 1 ] if len( item ) > 1 else None ) )
        self._connection.executemany(
            "INSERT OR IGNORE INTO object ( name, original_name ) VALUES ( ?, ? )",
            params,
        )

    def add_object( self, name: str, original_name: str | None = None ) -> int | None:
        self.add_objects( [ ( name, original_name ) ] )
        self.commit( )
        return  self._id_of( "object", name )

    def add_tags( self, names: Iterable[ str ] ) -> None:
        """batch insert tags by name"""
        self._connection.executemany(
            "INSERT OR IGNORE INTO tag ( name ) VALUES ( ? )",
            [ ( name, ) for name in names ],
        )

    def add_tag( self, name: str ) -> int | None:
        self.add_tags( [ name ] )
        self.commit( )
        return  self._id_of( "tag", name )

    def link_many( self, links: Iterable[ tuple[ str, str ] ] ) -> None:
        """batch link (object_name, tag_name) pairs"""
        self._connection.executemany(
            """
            INSERT OR IGNORE INTO object_tag ( object_id, tag_id )
            SELECT object.id, tag.id
            FROM object, tag
            WHERE object.name = ? AND tag.name = ?
            """,
            list( links ),
        )

    def link( self, object_name: str, tag_name: str ) -> None:
        self.link_many( [ ( object_name, tag_name ) ] )
        self.commit( )

    def add_rule(
        self,
        tag_name: str,
        operation_name: str,
        target_name: str,
        position: int | None = None,
    ) -> None:
        """register one operation of a tag over a target tag

        `position` defaults to the next free slot (max + 1); an explicit value
        inserts the rule at a chosen fold order
        re-registering the same operation over the same target is a no-op"""
        tag_id = self._id_of( "tag", tag_name )
        operation_id = self._id_of( "operation", operation_name, "id_operation" )
        target_id = self._id_of( "tag", target_name )
        if tag_id is None:
            raise   ValueError( f"tag not found: {tag_name}" )
        if operation_id is None:
            raise   ValueError( f"operation not found: {operation_name}" )
        if target_id is None:
            raise   ValueError( f"target tag not found: {target_name}" )
        if self._rule_exists( tag_id, operation_id, target_id ):
            return
        if position is None:
            position = self._next_position( tag_id )
        self._connection.execute(
            """
            INSERT INTO rule ( position, id_tag, id_operation, id_target )
            VALUES ( ?, ?, ?, ? )
            """,
            ( position, tag_id, operation_id, target_id ),
        )

    def add_rules(
        self,
        rules: Iterable[ tuple[ str, str, str ] ],
    ) -> None:
        """batch register rules: iterable of (tag_name, operation_name, target_name)"""
        for tag_name, operation_name, target_name in rules:
            self.add_rule( tag_name, operation_name, target_name )

    def rules_of( self, tag_name: str ) -> list[ Rule ]:
        """rules of a tag in fold order: list of Rule(operation, target)"""
        tag_id = self._id_of( "tag", tag_name )
        if tag_id is None:
            return  [ ]
        rows = self._connection.execute(
            """
            SELECT operation.name AS operation, target.name AS target
            FROM rule
            JOIN operation ON operation.id_operation = rule.id_operation
            JOIN tag AS target ON target.id = rule.id_target
            WHERE rule.id_tag = :tag_id
            ORDER BY rule.position
            """,
            { "tag_id": tag_id },
        )
        return  [
            Rule( operation = row[ "operation" ], target = row[ "target" ] )
            for row in rows
        ]

    def create_engine( self ) -> TagEngine:
        """load graph data into a TagEngine for set algebra resolution"""
        direct_members: dict[ int, set[ int ] ] = { }
        for row in self._connection.execute( "SELECT tag_id, object_id FROM object_tag" ):
            direct_members.setdefault( row[ "tag_id" ], set( ) ).add( row[ "object_id" ] )

        rules_by_tag: dict[ int, list[ tuple[ str, int ] ] ] = { }
        rows = self._connection.execute(
            """
            SELECT rule.id_tag, operation.name AS operation, rule.id_target
            FROM rule
            JOIN operation ON operation.id_operation = rule.id_operation
            ORDER BY rule.position
            """
        )
        for row in rows:
            rules_by_tag.setdefault( row[ "id_tag" ], [ ] ).append(
                ( row[ "operation" ], row[ "id_target" ] )
            )

        all_object_ids = {
            row[ "id" ] for row in self._connection.execute( "SELECT id FROM object" )
        }
        return  TagEngine( direct_members, rules_by_tag, all_object_ids )

    def tags_of( self, object_name: str ) -> list[ str ]:
        object_id = self._id_of( "object", object_name )
        if object_id is None:
            return  [ ]
        engine = self.create_engine( )
        rows = self._connection.execute( "SELECT id, name FROM tag" )
        return  sorted(
            row[ "name" ]
            for row in rows
            if object_id in engine.members_of( row[ "id" ] )
        )

    def objects_of( self, tag_name: str ) -> list[ ObjectRecord ]:
        """list of objects belonging to a tag, sorted by name"""
        tag_id = self._id_of( "tag", tag_name )
        if tag_id is None:
            return  [ ]
        engine = self.create_engine( )
        member_ids = engine.members_of( tag_id )
        if not member_ids:
            return  [ ]
        placeholders = ",".join( "?" * len( member_ids ) )
        rows = self._connection.execute(
            f"SELECT name, original_name FROM object WHERE id IN ({placeholders}) ORDER BY name",
            list( member_ids ),
        )
        return  [
            ObjectRecord( name = row[ "name" ], original_name = row[ "original_name" ] )
            for row in rows
        ]

    def count_by_tag( self ) -> list[ TagCount ]:
        """number of objects per tag, most populated first"""
        engine = self.create_engine( )
        rows = self._connection.execute( "SELECT id, name FROM tag" )
        counts = [
            TagCount(
                name = row[ "name" ],
                count = len( engine.members_of( row[ "id" ] ) ),
            )
            for row in rows
        ]
        return  sorted( counts, key = lambda item: item.count, reverse = True )

    def _rule_exists(
        self, tag_id: int, operation_id: int, target_id: int
    ) -> bool:
        row = self._connection.execute(
            """
            SELECT 1 FROM rule
            WHERE id_tag = ? AND id_operation = ? AND id_target = ?
            """,
            ( tag_id, operation_id, target_id ),
        ).fetchone( )
        return  row is not None

    def _next_position( self, tag_id: int ) -> int:
        row = self._connection.execute(
            """
            SELECT COALESCE( MAX( position ), -1 ) + 1 AS next
            FROM rule
            WHERE id_tag = ?
            """,
            ( tag_id, ),
        ).fetchone( )
        return  row[ "next" ]

    def _seed_operations( self ) -> None:
        self._connection.executemany(
            "INSERT OR IGNORE INTO operation ( name ) VALUES ( ? )",
            [ ( name, ) for name in OPERATION_NAMES ],
        )

    def _id_of(
        self, table: str, name: str, id_column: str = "id"
    ) -> int | None:
        row = self._connection.execute(
            f"SELECT {id_column} FROM {table} WHERE name = ?", ( name, )
        ).fetchone( )
        return  row[ id_column ] if row else None


