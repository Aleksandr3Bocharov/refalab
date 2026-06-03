// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-06-05
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file macrocode.h  ----------
//             generator of macrocode text
//----------------------------------------

#ifndef MACROCODE_H
#define MACROCODE_H

#include <stddef.h>
#include "avl_identifiers.h"

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
