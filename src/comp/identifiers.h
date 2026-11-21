// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-11-21
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file identifiers.h  ----------
//         Work with identifier table
//------------------------------------------

#ifndef IDENTIFIERS_H
#define IDENTIFIERS_H

#include <stdint.h>
#include "avl_identifiers.h"

// Label of a sentence (the jump target for the next or the FAIL sentence);
// mode is kept for the label bookkeeping.
typedef struct info_label
{
    void *info;
    uint8_t mode;
} T_INFO_LABEL;

// Allocates a new sentence label.
extern T_INFO_LABEL *generate_info_label(void);

// Definition and reference of functions and specifiers.
// function_definition: begins a function or its next sentence; function_end: ends a function.
extern void function_definition(void);
extern void function_end(void);
extern T_LABEL *specifier_definition(void);
extern T_LABEL *function_reference(const char *identifier, uint8_t identifier_length, size_t identifier_cursor_number);
extern T_LABEL *specifier_reference(const char *identifier, uint8_t identifier_length, size_t identifier_cursor_number, char tail);

// Module directives: $EMPTY (a function that always fails), $ENTRY (export under an
// external name), $EQU (two names for one label), $EXTRN (import under an external
// name) and $SWAP (a box, the head of which is aligned to 8 bytes).
extern void set_empty(const char *identifier, uint8_t identifier_length, size_t identifier_cursor_number);
extern void set_entry(const char *identifier, uint8_t identifier_length, size_t identifier_cursor_number, const char *identifier_extern, uint8_t identifier_extern_length, size_t identifier_extern_cursor_number);
extern void set_equ(const char *identifier1, uint8_t identifier1_length, size_t identifier1_cursor_number, const char *identifier2, uint8_t identifier2_length, size_t identifier2_cursor_number);
extern void set_extrn(const char *identifier, uint8_t identifier_length, size_t identifier_cursor_number, const char *identifier_extern, uint8_t identifier_extern_length, size_t identifier_extern_cursor_number);
extern void set_swap(const char *identifier, uint8_t identifier_length, size_t identifier_cursor_number);

// Start of a module, check of all labels at its end and release of the tables.
extern void module_init(void);
extern void module_terminate(void);
extern void module_end(void);

#endif
