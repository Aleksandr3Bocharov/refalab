// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-04-03
// https://github.com/Aleksandr3Bocharov/refalab

//-----------------  file  --  cj.h  -------------------
//             generator of assembler text
//------------------------------------------------------

#ifndef CJ_H
#define CJ_H

#include <stddef.h>
#include "clu.h"

extern void jit_address(T_U *buffer_node);
extern void jit_byte(uint8_t buffer_byte);
extern void jit_end(void);
extern void jit_entry(T_U *entry_node, const char *idendifier_extern, uint8_t idendifier_extern_length);
extern void jit_equ(T_U *equ_node, T_U *node);
extern void jit_extrn(T_U *extrn_node, const char *idendifier_extern, uint8_t idendifier_extern_length);
extern void jit_label(T_U *node);
extern void jit_start(void);
extern size_t jit_where(void);

#endif
