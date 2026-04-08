// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-04-07
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
extern void spdef(const char *idp, size_t lid);
extern T_LABEL *fnref(const char *idp, size_t lid);
extern T_LABEL *spref(const char *idp, size_t lid, char d);

extern void set_empty(const char *identifier, uint8_t identifier_length);
extern void set_entry(const char *identifier, uint8_t identifier_length, const char *identifier_extern, uint8_t identifier_extern_length);
extern void sequ(const char *id1, size_t lid1, const char *id0, size_t lid0);
extern void set_extrn(const char *identifier, uint8_t identifier_length, const char *identifier_extern, uint8_t identifier_extern_length);
extern void set_swap(const char *identifier, uint8_t identifier_length);

extern void s_init(void);
extern void s_term(void);
extern void s_end(void);

#endif
