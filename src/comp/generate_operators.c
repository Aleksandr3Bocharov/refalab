// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-06-05
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file generate_operators.c  ----------
//    generation of the assembly language operators
//-------------------------------------------------

#include <stddef.h>
#include <stdint.h>
#include "refalab.h"
#include "generate_operators.h"
#include "cj.h"

typedef struct _TAG
{
    uint8_t byte1;
    uint8_t byte2;
    uint8_t byte3;
    uint8_t byte4;
} T_TAG;

void generate_operator_n(uint8_t operator, uint8_t n)
{
    jit_byte(operator);
    jit_byte(n);
    return;
}

void generate_operator_n_m(uint8_t operator, uint8_t n, uint8_t m)
{
    jit_byte(operator);
    jit_byte(n);
    jit_byte(m);
    return;
}

void generate_operator_l(uint8_t operator, T_LABEL *l)
{
    jit_byte(operator);
    jit_address(l);
    return;
}

void generate_symbol(const T_LINKTI *code)
{
    const T_TAG *code_tag = (T_TAG *)&(code->tag);
    jit_byte(code_tag->byte1);
    jit_byte(code_tag->byte2);
    jit_byte(code_tag->byte3);
    jit_byte(code_tag->byte4);
    if (code->tag == TAGF)
    {
        jit_address(code->info.codef);
        return;
    };
    const uint8_t *code_info = (uint8_t *)&(code->info.codef);
    if (code->tag == TAGO)
    {
        jit_byte(*code_info);
        for (uint8_t i = 1; i < LBLL; i++)
            jit_byte(0);
    }
    else
    {
        uint8_t i = 0;
        for (; i < 4 /* sizeof(uint32_t) */; i++)
            jit_byte(*(code_info + i));
        for (; i < LBLL; i++)
            jit_byte(0);
    }
    return;
}

void generate_operator_s(uint8_t operator, const T_LINKTI *code)
{
    jit_byte(operator);
    generate_symbol(code);
    return;
}

//----------  end of file generate_operators.c  ----------
