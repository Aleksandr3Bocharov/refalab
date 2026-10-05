// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-10-05
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file compile_output.c  ----------
//    output abstraction for macrocode/buffers
//---------------------------------------------

#include <stddef.h>
#include <stdint.h>
#include "compile_output.h"
#include "avl_identifiers.h"
#include "macrocode.h"
#include "function_pool.h"

static T_OUTPUT_MODE current_mode = OUTPUT_MACROCODE;
static T_STORED_SENTENCE *current_sentence = NULL;

void compile_output_init(void)
{
    current_mode = OUTPUT_MACROCODE;
    current_sentence = NULL;
    return;
}

void compile_output_set_mode(T_OUTPUT_MODE mode, T_STORED_SENTENCE *sentence)
{
    current_mode = mode;
    current_sentence = sentence;
    return;
}

void compile_output_byte(uint8_t byte)
{
    switch (current_mode)
    {
    case OUTPUT_MACROCODE:
        macrocode_byte(byte);
        break;
    case OUTPUT_LEFT_PART:
        sentence_append_byte_left(current_sentence, byte);
        break;
    case OUTPUT_RIGHT_PART:
        sentence_append_byte_right(current_sentence, byte);
    }
    return;
}

void compile_output_address(T_LABEL *label)
{
    switch (current_mode)
    {
    case OUTPUT_MACROCODE:
        macrocode_address(label);
        break;
    case OUTPUT_LEFT_PART:
        sentence_append_address_left(current_sentence, label);
        break;
    case OUTPUT_RIGHT_PART:
        sentence_append_address_right(current_sentence, label);
    }
    return;
}

void compile_output_label(T_LABEL *label)
{
    if (current_mode == OUTPUT_MACROCODE)
        macrocode_label(label);
    return;
}

//----------  end of file compile_output.c  ----------
