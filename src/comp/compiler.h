// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-09-25
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file compiler.h  ----------
//      The main file of RefalAB compiler
//---------------------------------------

#ifndef COMPILER_H
#define COMPILER_H

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "refalab.h"
#include "compile_sentence.h"
#include "generate_operators.h"

typedef struct options_
{ // compiling   options
    bool source_listing;
    bool full_name;
    bool names;
    bool llvm_source_only;
    char llvm_options[8192];
} T_OPTIONS;

extern T_OPTIONS options;

typedef struct scanner
{ // the table for corresponding with scanner
    size_t last_error_cursor;
    char module_name[MAX_IDENTIFIER_LENGTH]; // module name
    uint8_t module_name_length;              // module name length
    uint32_t module_number;
    char label_name[MAX_IDENTIFIER_LENGTH]; // label (function or specifier) name
    uint8_t label_name_length;              // label (function or specifier) name length
    size_t label_cursor_number;             // label (function or specifier) cursor number
} T_SCANNER;

extern T_SCANNER scanner;

typedef struct sentence_element
{                                           // current sentence element
    T_SENTENCE_ELEMENT_TYPES type;          //    element type
    char identifier[MAX_IDENTIFIER_LENGTH]; //    variable idenfifier
    uint8_t identifier_length;              // variable identifier length
    bool v_variable;
    T_LINKTI code;
    T_LINKTI specifier;
    size_t cursor_number;
} T_SENTENCE_ELEMENT;

extern T_SENTENCE_ELEMENT current_sentence_element;

typedef struct save_scanner_state
{
    size_t cursor;                    // position in source code
    bool end_refalab_source;          // end of file flag
    bool scanner_station;             // literal chain mode
    bool scanner_station_k;           // mode after '<'
    bool scanner_big_number;          // big number mode
    bool left_part_sentence;          // left part flag
    bool suppress_listing;            // listing suppression flag
    T_SENTENCE_ELEMENT element;       // current element
    // Buffers for big numbers
    uint32_t *big_number_buffer_copy; // macrodigit buffer copy
    size_t big_number_count;          // macrodigit count
    size_t big_number_index;          // current index
} T_SAVE_SCANNER_STATE;

extern bool suppress_listing;

extern FILE *refalab_source_listing, *terminal;
extern FILE *llvm_source; // for llvm

extern void get_location(size_t *line, size_t *column, size_t cursor);
extern void processing_error(void);
extern void scan_sentence_element(void);

extern void free_scanner_state(T_SAVE_SCANNER_STATE *state);
extern void restore_scanner_state(const T_SAVE_SCANNER_STATE *state);
extern void save_scanner_state(T_SAVE_SCANNER_STATE *state);

#endif
