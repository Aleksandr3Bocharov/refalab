// Copyright 2025 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2025-03-18
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
        char infoc[2];
        uint32_t coden;
        uint8_t *codef;
    } info;
} T_LINKTI;

extern void gopl(char k, const uint8_t *l);
extern void gopn(char k, char n);
extern void gopnm(char k, char n, char m);
extern void gops(char k, const T_LINKTI *code);
extern void gsymbol(const T_LINKTI *code);

#endif
