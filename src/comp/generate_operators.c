// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-11-17
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file generate_operators.c  ----------
//    generation of the assembly language operators
//-------------------------------------------------

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include "refalab.h"
#include "generate_operators.h"
#include "compile_output.h"
#include "compile_sentence.h"

#if defined mdebug
// names of the assembly language statement codes (see compile_sentence.h)
// Names of the assembly language statement codes (see compile_sentence.h),
// indexed by code; used only for the debug trace.
static const char *const operator_names[] = {
    [1] = "n_sjump",
    [2] = "n_fail",
    [3] = "n_sb1b2",
    [4] = "n_lsc",
    [5] = "n_rsc",
    [6] = "n_lsco",
    [7] = "n_rsco",
    [8] = "n_lsd",
    [9] = "n_rsd",
    [10] = "n_ltxt",
    [11] = "n_rtxt",
    [12] = "n_lb",
    [13] = "n_lby",
    [14] = "n_rb",
    [15] = "n_rby",
    [16] = "n_ls",
    [17] = "n_rs",
    [18] = "n_lw",
    [19] = "n_rw",
    [20] = "n_lbnil",
    [21] = "n_rbnil",
    [22] = "n_lbce",
    [23] = "n_rbce",
    [24] = "n_nil",
    [25] = "n_ce",
    [26] = "n_led",
    [27] = "n_red",
    [28] = "n_nnil",
    [29] = "n_ple",
    [30] = "n_plv",
    [31] = "n_le",
    [32] = "n_pre",
    [33] = "n_prv",
    [34] = "n_re",
    [35] = "n_plesc",
    [36] = "n_plvsc",
    [37] = "n_lesc",
    [38] = "n_presc",
    [39] = "n_prvsc",
    [40] = "n_resc",
    [41] = "n_lesd",
    [42] = "n_resd",
    [43] = "n_pleb",
    [44] = "n_plvb",
    [45] = "n_leb",
    [46] = "n_preb",
    [47] = "n_prvb",
    [48] = "n_reb",
    [49] = "n_eoe",
    [50] = "n_eoei",
    [51] = "n_lsrch",
    [52] = "n_rsrch",
    [53] = "n_wspc",
    [54] = "n_espc",
    [55] = "n_plespc",
    [56] = "n_lespc",
    [57] = "n_prespc",
    [58] = "n_respc",
    [59] = "n_lmax",
    [60] = "n_rmax",
    [61] = "n_eor",
    [62] = "n_ns",
    [63] = "n_nso",
    [64] = "n_text",
    [65] = "n_bl",
    [66] = "n_br",
    [67] = "n_blr",
    [68] = "n_bract",
    [70] = "n_muls",
    [71] = "n_mule",
    [72] = "n_tpl",
    [73] = "n_tplm",
    [74] = "n_tple",
    [75] = "n_tplv",
    [76] = "n_tpls",
    [77] = "n_eos",
    [78] = "n_swap",
    [79] = "n_blf",
};

// Returns the name of an operator code, or "n_?" for an unknown one.
const char *operator_name(uint8_t operator)
{
    if (operator < sizeof(operator_names) / sizeof(operator_names[0]) && operator_names[operator] != NULL)
        return operator_names[operator];
    return "n_?";
}

// prints a symbol in readable form (without the new line)
void debug_print_symbol(const T_LINKTI *code)
{
    switch (code->tag)
    {
    case TAGO:
    {
        const uint8_t ch = (uint8_t)code->info.infoc;
        if (ch >= 0x20 && ch < 0x7F)
            fprintf(stderr, "char '%c'", ch);
        else
            fprintf(stderr, "char 0x%02X", ch);
        break;
    }
    case TAGF:
        if (code->info.codef != NULL)
            fprintf(stderr, "function %.*s", code->info.codef->identifier_length, code->info.codef->identifier);
        else
            fprintf(stderr, "function (null)");
        break;
    case TAGN:
        fprintf(stderr, "macrodigit %u", code->info.coden);
        break;
    default:
        fprintf(stderr, "tag=%u info=0x%08X", code->tag, code->info.coden);
    }
    return;
}
#endif

// Generators of the assembly language statements.
// Every statement is the operator code followed by its operands:
// generate_operator - no operands
void generate_operator(uint8_t operator)
{
#if defined mdebug
    fprintf(stderr, "generate_operator: %s\n", operator_name(operator));
#endif
    compile_output_byte(operator);
    return;
}

// one numeric operand (a variable or element number)
void generate_operator_n(uint8_t operator, uint8_t n)
{
#if defined mdebug
    fprintf(stderr, "generate_operator: %s %u\n", operator_name(operator), n);
#endif
    compile_output_byte(operator);
    compile_output_byte(n);
    return;
}

// two numeric operands
void generate_operator_n_m(uint8_t operator, uint8_t n, uint8_t m)
{
#if defined mdebug
    fprintf(stderr, "generate_operator: %s %u %u\n", operator_name(operator), n, m);
#endif
    compile_output_byte(operator);
    compile_output_byte(n);
    compile_output_byte(m);
    return;
}

// one label operand (a jump target or a function reference)
void generate_operator_l(uint8_t operator, T_LABEL *l)
{
#if defined mdebug
    fprintf(stderr, "generate_operator: %s label=%p", operator_name(operator), (void *)l);
    if (l != NULL && l->identifier_length != 0)
        fprintf(stderr, " (%.*s)", l->identifier_length, l->identifier);
    fprintf(stderr, "\n");
#endif
    compile_output_byte(operator);
    compile_output_address(l);
    return;
}

// Outputs a symbol in the memory layout of a list cell: the tag, then the payload
// padded to the size of a pointer (a function label is output as an address).
void generate_symbol(const T_LINKTI *code)
{
#if defined mdebug
    fprintf(stderr, "generate_symbol: ");
    debug_print_symbol(code);
    fprintf(stderr, "\n");
#endif
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

// an operator followed by a symbol operand
void generate_operator_s(uint8_t operator, const T_LINKTI *code)
{
#if defined mdebug
    fprintf(stderr, "generate_operator: %s ", operator_name(operator));
    debug_print_symbol(code);
    fprintf(stderr, "\n");
#endif
    compile_output_byte(operator);
    generate_symbol(code);
    return;
}

//----------  end of file generate_operators.c  ----------
