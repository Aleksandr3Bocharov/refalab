// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-06-09
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
    size_t carriage_number;
    char module_name[MAX_IDENTIFIER_LENGTH]; // module name
    uint8_t module_name_length;              // module name length
    uint32_t module_number;
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
} T_SENTENCE_ELEMENT;

extern T_SENTENCE_ELEMENT current_sentence_element;

extern FILE *refalab_source_listing, *terminal;
extern FILE *llvm_source; // for llvm

extern void processing_error(void);
extern void scan_sentence_element(void);

#endif
