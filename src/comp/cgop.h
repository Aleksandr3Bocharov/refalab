// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-03-25
// https://github.com/Aleksandr3Bocharov/refalab

//------------------file----CGOP.H--------------------
//    generation of the assembly language operators
//----------------------------------------------------

#ifndef CGOP_H
#define CGOP_H

#include <stdint.h>

typedef struct linkti
{
    uint16_t tag;
    union
    {
        char infoc;
        uint32_t coden;
        uint8_t *codef;
    } info;
} T_LINKTI;

extern void generate_operator_l(uint8_t operator, const uint8_t *l);
extern void generate_operator_n(uint8_t operator, uint8_t n);
extern void generate_operator_n_m(uint8_t operator, uint8_t n, uint8_t m);
extern void generate_operator_s(uint8_t operator, const T_LINKTI *code);
extern void generate_symbol(const T_LINKTI *code);

#endif
