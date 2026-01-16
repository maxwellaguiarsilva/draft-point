#!/usr/bin/python3


#   Copyright (C) 2025 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
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
#   You should have received a copy of the GNU General License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#   
#   
#   File:   config
#   Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
#   
#   Created on 2026-01-16 11:00:00


import os
import copy
from lib.common import get_cpu_count, deep_update
from lib.config import DEFAULT_CONFIG


DEFAULT_CPP_CONFIG = {
    #   compiler information
    "compiler": {
        "executable": "clang++"        #   options: "g++", "clang++" or full path
        ,"standard": "c++23"           #   examples: "c++17", "c++20", "c++23"
        ,"use_64_bits": True           #   abstraction for -m64 (64-bit)
        ,"extra_compile_flags": [
            "-ffunction-sections"      #   place each function into its own section
            ,"-fdata-sections"         #   place each data item into its own section
            ,"-flto"                   #   enable link time optimization
        ]
        ,"extra_link_flags": [
            "-flto"                    #   enable link time optimization during linking
        ]
        ,"linker_direct_options": [
            "--as-needed"              #   only link libraries that satisfy undefined symbols
            ,"--gc-sections"           #   remove unused sections (dead code elimination)
        ]
    }

    #   build rules (how to compile)
    ,"build_behavior": {
        #   options: "none" (-O0), "balanced" (-O2), "aggressive" (-O3), "debug" (-Og)
        "optimization": "balanced"
        ,"optimization_levels": {
            "none": "-O0"
            ,"balanced": "-O2"
            ,"aggressive": "-O3"
            ,"debug": "-Og"
        }
        ,"debug_symbols": False         #   generates symbols for gdb (-g)
        ,"generate_dependencies": False #   generates .d files (intelligent recompilation)
        ,"experimental_library": True   #   enables -fexperimental-library
        ,"max_threads": get_cpu_count( )
    }

    #   quality control (warning and analysis flags)
    ,"quality_control": {
        #   options: "minimal", "high" (-Wall -Wextra), "pedantic"
        "warning_level": "high"
        ,"warning_levels": {
            "minimal": ["-Wall"]
            ,"high": ["-Wall", "-Wextra"]
            ,"pedantic": ["-Wall", "-Wextra", "-Wpedantic"]
        }
        ,"treat_warnings_as_errors": True #   -Werror
        ,"stop_on_first_error": True      #   -Wfatal-errors
        ,"static_analysis": {
            "enabled": True
            ,"tool": "cppcheck"
            ,"strictness": "exhaustive"    #   options: "normal", "exhaustive"
            ,"suppressions": [
                "missingIncludeSystem"
                ,"checkersReport"
            ]
        }
    }

    #   external dependencies (no -l or -I prefixes)
    ,"dependencies": {
        "libraries": []                #   example: ["ncurses", "pthread"]
        ,"include_dirs": []            #   additional paths for header search
        ,"library_dirs": []             #   additional paths for library search
    }

    #   file search patterns
    ,"patterns": {
        "source_extension": "cpp"
        ,"header_extension": "hpp"
        ,"main_function": r"\b(int|auto)\s+main\s*\("
    }
}

#   The cpp config performs a deep_update on the generic config
DEFAULT_CPP_CONFIG = deep_update( copy.deepcopy( DEFAULT_CONFIG ), DEFAULT_CPP_CONFIG )
