// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-06-11
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file macrocode.h  ----------
//             generator of macrocode text
//----------------------------------------

#ifndef MACROCODE_H
#define MACROCODE_H

#include <stddef.h>
#include <stdint.h>
#include "avl_identifiers.h"

extern void macrocode_address(T_LABEL *label);
extern void macrocode_byte(uint8_t byte);
extern void macrocode_end(void);
extern void macrocode_entry(T_LABEL *label, const char *idendifier_extern, uint8_t idendifier_extern_length);
extern void macrocode_equ(T_LABEL *equ_label, T_LABEL *label);
extern void macrocode_extrn(T_LABEL *label, const char *idendifier_extern, uint8_t idendifier_extern_length);
extern void macrocode_label(T_LABEL *label);
extern void macrocode_start(void);
extern size_t macrocode_where(void);

#endif
