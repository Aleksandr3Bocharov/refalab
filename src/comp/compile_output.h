// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-10-05
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file compile_output.h  ----------
//    output abstraction for macrocode/buffers
//---------------------------------------------

#ifndef COMPILE_OUTPUT_H
#define COMPILE_OUTPUT_H

#include <stdint.h>
#include "avl_identifiers.h"
#include "function_pool.h"

typedef enum
{
    OUTPUT_MACROCODE,
    OUTPUT_LEFT_PART,
    OUTPUT_RIGHT_PART
} T_OUTPUT_MODE;

extern void compile_output_init(void);
extern void compile_output_set_mode(T_OUTPUT_MODE mode, T_STORED_SENTENCE *sentence);
extern void compile_output_set_current_sentence(T_STORED_SENTENCE *sentence);
extern void compile_output_switch_to_right_part(void);
extern void compile_output_switch_to_macrocode(void);
extern void compile_output_byte(uint8_t byte);
extern void compile_output_address(T_LABEL *label);
extern void compile_output_label(T_LABEL *label);

#endif

//----------  end of file compile_output.h  ----------
