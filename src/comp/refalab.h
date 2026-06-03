// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-05-29
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file refalab.h  ----------
//          Compiler version
//--------------------------------------

#ifndef REFALAB_H
#define REFALAB_H

#include <stdint.h>

#define TAGO 0
#define TAGF 2
#define TAGN 4
#define TAGR 6
#define TAGLB 1
#define TAGRB 3
#define TAGK 5
#define TAGD 7

#define Wbin "wb"
#define Rbin "rb"

#define ZBLL 4 // sizeof(uint32_t)
#define LBLL sizeof(void *)
#define SMBL (ZBLL + LBLL)

#define MAX_NUMBER 0xFFFFFFFFLL

#define MAX_IDENTIFIER_LENGTH 40
#define MAX_IDENTIFIER_EXTERN_LENGTH 32

#endif

//----------  end of file refalab.h  ----------
