// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-11-17
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file refalab.h  ----------
//          Compiler version
//--------------------------------------

#ifndef REFALAB_H
#define REFALAB_H

#include <stdint.h>

// Tag values of a list cell, as in the interpreter header.
// TAGO: a symbol, TAGF: a function label, TAGN: a macrodigit, TAGR: a reference to a box
#define TAGO 0
#define TAGF 2
#define TAGN 4
#define TAGR 6
// TAGLB/TAGRB: structure brackets, TAGK/TAGD: call brackets '<' and '>'.
#define TAGLB 1
#define TAGRB 3
#define TAGK 5
#define TAGD 7

// File open modes for binary output and input.
#define Wbin "wb"
#define Rbin "rb"

// Sizes used when laying out a function label:
// ZBLL - a 32-bit number, LBLL - a pointer, SMBL - a number followed by a pointer.
#define ZBLL sizeof(uint32_t)
#define LBLL sizeof(void *)
#define SMBL (ZBLL + LBLL)

// Largest macrodigit.
#define MAX_NUMBER UINT32_MAX

// Longest identifier and longest external (linker) name: the length is kept in one byte.
#define MAX_IDENTIFIER_LENGTH UINT8_MAX
#define MAX_IDENTIFIER_EXTERN_LENGTH UINT8_MAX

#endif

//----------  end of file refalab.h  ----------
