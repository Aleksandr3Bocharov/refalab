// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-10-05
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file generate_operators.c  ----------
//    generation of the assembly language operators
//-------------------------------------------------

#include <stddef.h>
#include <stdint.h>
#include "refalab.h"
#include "generate_operators.h"
#include "compile_output.h"

void generate_operator_n(uint8_t operator, uint8_t n)
{
    compile_output_byte(operator);
    compile_output_byte(n);
    return;
}

void generate_operator_n_m(uint8_t operator, uint8_t n, uint8_t m)
{
    compile_output_byte(operator);
    compile_output_byte(n);
    compile_output_byte(m);
    return;
}

void generate_operator_l(uint8_t operator, T_LABEL *l)
{
    compile_output_byte(operator);
    compile_output_address(l);
    return;
}

void generate_symbol(const T_LINKTI *code)
{
    const uint8_t *tag_bytes = (const uint8_t *)&(code->tag);
    for (uint8_t i = 0; i < ZBLL; i++)
        compile_output_byte(tag_bytes[i]);
    if (code->tag == TAGF)
    {
        compile_output_address(code->info.codef);
        return;
    };
    const uint8_t *code_info = (const uint8_t *)&(code->info.codef);
    if (code->tag == TAGO)
    {
        compile_output_byte(*code_info);
        for (uint8_t i = 1; i < LBLL; i++)
            compile_output_byte(0);
    }
    else
    {
        uint8_t i = 0;
        for (; i < ZBLL; i++)
            compile_output_byte(*(code_info + i));
        for (; i < LBLL; i++)
            compile_output_byte(0);
    }
    return;
}

void generate_operator_s(uint8_t operator, const T_LINKTI *code)
{
    compile_output_byte(operator);
    generate_symbol(code);
    return;
}

//----------  end of file generate_operators.c  ----------
