// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-06-05
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file generate_operators.h  ----------
//    generation of the assembly language operators
//-------------------------------------------------

#ifndef GENERATE_OPERATORS_H
#define GENERATE_OPERATORS_H

#include <stdint.h>
#include "clu.h"

typedef struct linkti
{
    uint32_t tag;
    union
    {
        char infoc;
        uint32_t coden;
        T_LABEL *codef;
    } info;
} T_LINKTI;

extern void generate_operator_l(uint8_t operator, T_LABEL *l);
extern void generate_operator_n(uint8_t operator, uint8_t n);
extern void generate_operator_n_m(uint8_t operator, uint8_t n, uint8_t m);
extern void generate_operator_s(uint8_t operator, const T_LINKTI *code);
extern void generate_symbol(const T_LINKTI *code);

#endif
