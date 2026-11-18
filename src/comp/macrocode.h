// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-11-17
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file macrocode.h  ----------
//             generator of macrocode text
//----------------------------------------

#ifndef MACROCODE_H
#define MACROCODE_H

#include <stddef.h>
#include <stdint.h>
#include "avl_identifiers.h"

// Writers of the macrocode text: data bytes and addresses, label definitions,
// the module prologue/epilogue (start/end) and the entry/extrn/equ declarations.
// The module image is accumulated in memory (bytes and label references) and is written
// as an LLVM IR source when the module ends (macrocode_end).
// macrocode_start/macrocode_end: begin and finish the image
// macrocode_byte/macrocode_address: append a data byte or an address of a label
// macrocode_label: define a label at the current address (macrocode_where)
// macrocode_equ: make a label an alias of another one
// macrocode_entry/macrocode_extrn: export and import a label under an external name
extern void macrocode_address(T_LABEL *label);
extern void macrocode_byte(uint8_t byte);
extern void macrocode_end(void);
extern void macrocode_entry(T_LABEL *label, const char *identifier_extern, uint8_t identifier_extern_length, size_t identifier_extern_cursor_number);
extern void macrocode_equ(T_LABEL *equ_label, T_LABEL *label);
extern void macrocode_extrn(T_LABEL *label, const char *identifier_extern, uint8_t identifier_extern_length, size_t identifier_extern_cursor_number);
extern void macrocode_label(T_LABEL *label);
extern void macrocode_start(void);
extern size_t macrocode_where(void);

#endif
