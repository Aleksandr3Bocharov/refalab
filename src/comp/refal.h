// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-03-25
// https://github.com/Aleksandr3Bocharov/refalab

//-----------  file  --  REFAL.H -------------
//      The main file of RefalAB compiler
//--------------------------------------------

#ifndef REFAL_H
#define REFAL_H

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "cgop.h"

#define MAX_IDENTIFIER_LENGTH 40
#define MAX_EXTERN_IDENTIFIER_LENGTH 32

typedef struct options_
{ // compiling   options
    bool source_listing;
    bool full_name;
    bool names;
    bool assembler_source_only;
    char assembler_options[8192];
} T_OPTIONS;

extern T_OPTIONS options;

typedef struct scanner
{ // the table for corresponding with scanner
    uint32_t carriage_number;
    char module_name[MAX_IDENTIFIER_LENGTH]; // module name
    uint8_t module_name_length;              // module name length
    uint32_t module_number;
} T_SCANNER;

extern T_SCANNER scanner;

typedef struct scanning_element
{                                           // current statement element
    uint8_t type;                           //    element type
    char identifier[MAX_IDENTIFIER_LENGTH]; //    variable idenfifier
    uint8_t identifier_length;              // variable identifier length
    bool v_variable;
    T_LINKTI code;
    T_LINKTI specifier;
} T_SCANNING_ELEMENT;

extern T_SCANNING_ELEMENT scanning_element;

extern FILE *sysprint, *terminal;
extern FILE *assembler_source; // for assem

extern void oshibka(void);
extern void scan(void);

#endif
