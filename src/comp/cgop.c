// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-03-25
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
    uint8_t byte1;
    uint8_t byte2;
} T_TAG;

void generate_operator_n(uint8_t operator, uint8_t n)
{
    jbyte(operator);
    jbyte(n);
    return;
}

void generate_operator_n_m(uint8_t operator, uint8_t n, uint8_t m)
{
    jbyte(operator);
    jbyte(n);
    jbyte(m);
    return;
}

void generate_operator_l(uint8_t operator, const uint8_t *l)
{
    jbyte(operator);
    j3addr((T_U *)(void *)l);
    return;
}

void generate_symbol(const T_LINKTI *code)
{
    const T_TAG *code_tag = (T_TAG *)&(code->tag);
    jbyte(code_tag->byte1);
    jbyte(code_tag->byte2);
    if (code->tag == TAGF)
    {
        j3addr((T_U *)(void *)code->info.codef);
        return;
    };
    const uint8_t *code_info = (uint8_t *)&(code->info.codef);
    if (code->tag == TAGO)
    {
        jbyte(*code_info);
        for (uint8_t i = 1; i < LBLL; i++)
            jbyte(0);
    }
    else
    {
        uint8_t i = 0;
        for (; i < 4 /* sizeof(uint32_t) */; i++)
            jbyte(*(code_info + i));
        for (; i < LBLL; i++)
            jbyte(0);
    }
    return;
}

void generate_operator_s(uint8_t operator, const T_LINKTI *code)
{
    jbyte(operator);
    generate_symbol(code);
    return;
}

//----------  end of file CGOP.C  -------------------
