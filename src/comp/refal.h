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

typedef struct scn_
{ // the table for corresponding with scanner
    uint32_t nomkar;
    char modname_var[MAX_IDENTIFIER_LENGTH]; // module name
    size_t modnmlen;              // module name length
} T_SCN;

extern T_SCN scn_;

typedef struct scn_e_
{                        // current statement element
    uint32_t t;          //    element type
    char si[MAX_IDENTIFIER_LENGTH]; //    variable idenfifier
    size_t si_leng;      // variable identifier length
    bool v;
    T_LINKTI code;
    T_LINKTI spec;
} T_SCN_E;

extern T_SCN_E scn_e;

extern FILE *sysprint, *systerm;
extern FILE *syslin; // for assem
extern FILE *systxt; // for module names

extern uint32_t nommod;

extern void oshibka(void);
extern void scan(void);

#endif
