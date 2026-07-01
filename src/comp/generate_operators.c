// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-07-01
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file generate_operators.c  ----------
//    generation of the assembly language operators
//-------------------------------------------------

#include <stddef.h>
#include <stdint.h>
#include "refalab.h"
#include "generate_operators.h"
#include "macrocode.h"

void generate_operator_n(uint8_t operator, uint8_t n)
{
    macrocode_byte(operator);
    macrocode_byte(n);
    return;
}

void generate_operator_n_m(uint8_t operator, uint8_t n, uint8_t m)
{
    macrocode_byte(operator);
    macrocode_byte(n);
    macrocode_byte(m);
    return;
}

void generate_operator_l(uint8_t operator, T_LABEL *l)
{
    macrocode_byte(operator);
    macrocode_address(l);
    return;
}

void generate_symbol(const T_LINKTI *code)
{
    const uint32_t code_tag = code->tag;
    macrocode_byte((uint8_t)((code_tag >> 24) & 0xFF));
    macrocode_byte((uint8_t)((code_tag >> 16) & 0xFF));
    macrocode_byte((uint8_t)((code_tag >> 8) & 0xFF));
    macrocode_byte((uint8_t)(code_tag & 0xFF));
    if (code->tag == TAGF)
    {
        macrocode_address(code->info.codef);
        return;
    };
    const uint8_t *code_info = (uint8_t *)&(code->info.codef);
    if (code->tag == TAGO)
    {
        macrocode_byte(*code_info);
        for (uint8_t i = 1; i < LBLL; i++)
            macrocode_byte(0);
    }
    else
    {
        uint8_t i = 0;
        for (; i < ZBLL; i++)
            macrocode_byte(*(code_info + i));
        for (; i < LBLL; i++)
            macrocode_byte(0);
    }
    return;
}

void generate_operator_s(uint8_t operator, const T_LINKTI *code)
{
    macrocode_byte(operator);
    generate_symbol(code);
    return;
}

//----------  end of file generate_operators.c  ----------
