// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-06-20
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file identifiers.h  ----------
//         Work with identifier table
//------------------------------------------

#ifndef IDENTIFIERS_H
#define IDENTIFIERS_H

#include <stddef.h>
#include <stdint.h>
#include "avl_identifiers.h"

typedef struct info_label
{
    void *info;
    uint8_t mode;
} T_INFO_LABEL;

extern T_INFO_LABEL *generate_info_label(void);

extern void function_definition(const char *identifier, uint8_t identifier_length, size_t identifier_cursor_number);
extern void specifier_definition(const char *identifier, uint8_t identifier_length, size_t identifier_cursor_number);
extern T_LABEL *function_reference(const char *identifier, uint8_t identifier_length, size_t identifier_cursor_number);
extern T_LABEL *specifier_reference(const char *identifier, uint8_t identifier_length, size_t identifier_cursor_number, char tail);

extern void set_empty(const char *identifier, uint8_t identifier_length, size_t identifier_cursor_number);
extern void set_entry(const char *identifier, uint8_t identifier_length, size_t identifier_cursor_number, const char *identifier_extern, uint8_t identifier_extern_length, size_t identifier_extern_cursor_number);
extern void set_equ(const char *identifier1, uint8_t identifier1_length, size_t identifier1_cursor_number, const char *identifier2, uint8_t identifier2_length, size_t identifier2_cursor_number);
extern void set_extrn(const char *identifier, uint8_t identifier_length, size_t identifier_cursor_number, const char *identifier_extern, uint8_t identifier_extern_length, size_t identifier_extern_cursor_number);
extern void set_swap(const char *identifier, uint8_t identifier_length, size_t identifier_cursor_number);

extern void module_init(void);
extern void module_terminate(void);
extern void module_end(void);

#endif
