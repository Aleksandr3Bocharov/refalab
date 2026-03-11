// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-03-11
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

extern void gopl(uint8_t k, const uint8_t *l);
extern void gopn(uint8_t k, uint8_t n);
extern void gopnm(uint8_t k, uint8_t n, uint8_t m);
extern void gops(uint8_t k, const T_LINKTI *code);
extern void gsymbol(const T_LINKTI *code);

#endif
