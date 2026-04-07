// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-04-07
// https://github.com/Aleksandr3Bocharov/refalab

//-----------------  file  --  cj.h  -------------------
//             generator of assembler text
//------------------------------------------------------

#ifndef CJ_H
#define CJ_H

#include <stddef.h>
#include "clu.h"

extern void jit_address(T_LABEL *label);
extern void jit_byte(uint8_t byte);
extern void jit_end(void);
extern void jit_entry(T_LABEL *label, const char *idendifier_extern, uint8_t idendifier_extern_length);
extern void jit_equ(T_LABEL *equ_label, T_LABEL *label);
extern void jit_extrn(T_LABEL *label, const char *idendifier_extern, uint8_t idendifier_extern_length);
extern void jit_label(T_LABEL *label);
extern void jit_start(void);
extern size_t jit_where(void);

#endif
