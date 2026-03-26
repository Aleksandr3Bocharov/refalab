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
    size_t module_name_length;               // module name length
} T_SCANNER;

extern T_SCANNER scanner;

typedef struct scanning_element
{                                   // current statement element
    uint32_t t;                     //    element type
    char si[MAX_IDENTIFIER_LENGTH]; //    variable idenfifier
    size_t si_leng;                 // variable identifier length
    bool v;
    T_LINKTI code;
    T_LINKTI spec;
} T_SCANNING_ELEMENT;

extern T_SCANNING_ELEMENT scanning_element;

extern FILE *sysprint, *systerm;
extern FILE *syslin; // for assem
extern FILE *systxt; // for module names

extern uint32_t nommod;

extern void oshibka(void);
extern void scan(void);

#endif
