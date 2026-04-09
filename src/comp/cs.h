// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-04-09
// https://github.com/Aleksandr3Bocharov/refalab

//-------------  file  --  CS.H  ---------------
//         Work with identifier table
//----------------------------------------------

#ifndef CH_H
#define CH_H

#include <stddef.h>
#include "clu.h"

typedef struct info_label
{
    void *info;
    uint8_t mode;
} T_INFO_LABEL;

extern T_INFO_LABEL *generate_info_label(void);

extern void function_definition(const char *identifier, uint8_t identifier_length);
extern void specifier_definition(const char *identifier, uint8_t identifier_length);
extern T_LABEL *function_reference(const char *identifier, uint8_t identifier_length);
extern T_LABEL *specifier_reference(const char *identifier, uint8_t identifier_length, char tail);

extern void set_empty(const char *identifier, uint8_t identifier_length);
extern void set_entry(const char *identifier, uint8_t identifier_length, const char *identifier_extern, uint8_t identifier_extern_length);
extern void set_equ(const char *identifier1, uint8_t identifier1_length, const char *identifier2, uint8_t identifier2_length);
extern void set_extrn(const char *identifier, uint8_t identifier_length, const char *identifier_extern, uint8_t identifier_extern_length);
extern void set_swap(const char *identifier, uint8_t identifier_length);

extern void module_init(void);
extern void module_terminate(void);
extern void module_end(void);

#endif
