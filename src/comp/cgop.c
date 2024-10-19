// Copyright 2024 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2024-10-19
// https://github.com/Aleksandr3Bocharov/RefalAB

//------------------file----CGOP.C--------------------
//    generation of the assembly language operators
//----------------------------------------------------

#include <stddef.h>
#include <stdint.h>
#include "refal.def"
#include "cgop.h"
#include "cj.h"

typedef struct _TAG
{
    char b1;
    char b2;
} T_TAG;

void gopn(char k, char n)
{
    jbyte(k);
    jbyte(n);
    return;
}

void gopnm(char k, char n, char m)
{
    jbyte(k);
    jbyte(n);
    jbyte(m);
    return;
}

void gopl(char k, const uint8_t *l)
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
    const char *r = (char *)&(code->info.codef);
    if (code->tag == TAGO)
    {
        jbyte(*r);
        jbyte(*(r + 1));
        for (size_t i = 2; i < LBLL; i++)
            jbyte('\0');
    }
    else
    {
        size_t i = 0;
        for (; i < sizeof(uint32_t); i++)
            jbyte(*(r + i));
        for (; i < LBLL; i++)
            jbyte('\0');
    }
    return;
}

void gops(char k, const T_LINKTI *code)
{
    jbyte(k);
    gsymbol(code);
    return;
}

/*
void ghw(uint16_t h)
{
    const T_TAG *po = (T_TAG *)&h; //  eg
    jbyte(po->b1);
    jbyte(po->b2);
    return;
}
*/

//----------  end of file CGOP.C  -------------------
