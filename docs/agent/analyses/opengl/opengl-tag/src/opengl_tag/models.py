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
#   File:   docs/agent/analyses/opengl/opengl-tag/src/opengl_tag/models.py
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-08-24 14:26:53
#   


from typing import NamedTuple


class Rule( NamedTuple ):
    """rule item applying an operation over a target tag (order in array defines fold position)"""
    operation: str
    target: str


class TagCount( NamedTuple ):
    """tag name and the number of objects belonging to it"""
    name: str
    count: int


class ObjectRecord( NamedTuple ):
    """object name and optional original source name"""
    name: str
    original_name: str | None = None


