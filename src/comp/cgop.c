// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-03-11
// https://github.com/Aleksandr3Bocharov/refalab

//------------------file----CGOP.C--------------------
//    generation of the assembly language operators
//----------------------------------------------------

#include <stddef.h>
#include <stdint.h>
#include "refalab.h"
#include "cgop.h"
#include "cj.h"

typedef struct _TAG
{
    uint8_t b1;
    uint8_t b2;
} T_TAG;

void gopn(uint8_t k, uint8_t n)
{
    jbyte(k);
    jbyte(n);
    return;
}

void gopnm(uint8_t k, uint8_t n, uint8_t m)
{
    jbyte(k);
    jbyte(n);
    jbyte(m);
    return;
}

void gopl(uint8_t k, const uint8_t *l)
{
    jbyte(k);
    j3addr((T_U *)(void *)l);
    return;
}

void gsymbol(const T_LINKTI *code)
{
    const T_TAG *q = (T_TAG *)&(code->tag);
    jbyte(q->b1);
    jbyte(q->b2);
    if (code->tag == TAGF)
    {
        j3addr((T_U *)(void *)code->info.codef);
        return;
    };
    const uint8_t *r = (uint8_t *)&(code->info.codef);
    if (code->tag == TAGO)
    {
        jbyte(*r);
        for (size_t i = 1; i < LBLL; i++)
            jbyte(0);
    }
    else
    {
        size_t i = 0;
        for (; i < 4 /* sizeof(uint32_t) */; i++)
            jbyte(*(r + i));
        for (; i < LBLL; i++)
            jbyte(0);
    }
    return;
}

void gops(uint8_t k, const T_LINKTI *code)
{
    jbyte(k);
    gsymbol(code);
    return;
}

//----------  end of file CGOP.C  -------------------
