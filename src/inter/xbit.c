// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-08-07
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file xbit.c  ----------
//                Bitwise MO:
//      Bit, Bitand, Bitor, Bitxor,
//      Bitnot, Bitshl, Bitshr,
//      Bitrol, Bitror
//-----------------------------------

#include <stddef.h>
#include <stdint.h>
#include "refalab.h"
#include "interface.h"

static void bit_(void)
{
    do
    {
        T_LINKCB *current_argument = refal.previous_argument->next;
        if (current_argument->tag != TAGN)
            break;
        uint64_t bit = gcoden(current_argument);
        current_argument = current_argument->next;
        if (current_argument->tag == TAGN)
        {
            bit = (bit << 32) | gcoden(current_argument);
            current_argument = current_argument->next;
        }
        if (current_argument->tag != TAGLB)
            break;
        T_LINKCB *right_bracket = current_argument->info.codep;
        if (right_bracket->next != refal.next_argument)
            break;
        if (bit < 32 || (bit & 31) != 0)
            break;
        const uint64_t need_numbers = bit >> 5;
        T_LINKCB *begin_numbers = current_argument->next;
        uint64_t have_numbers = 0;
        T_LINKCB *current_number;
        for (current_number = begin_numbers; current_number != right_bracket; current_number = current_number->next)
        {
            if (current_number->tag != TAGN)
                break;
            have_numbers++;
        }
        if (current_number != right_bracket)
            break;
        if (have_numbers == need_numbers)
        {
            transplantation(refal.previous_result, current_argument, right_bracket);
            return;
        }
        if (have_numbers < need_numbers)
        {
            uint64_t add_numbers = need_numbers - have_numbers;
            if (!extended_insert_from_free_memory(begin_numbers->previous, (size_t)add_numbers))
                return;
            begin_numbers = current_argument->next;
            for (current_number = begin_numbers; add_numbers != 0; add_numbers--, current_number = current_number->next)
            {
                current_number->tag = TAGN;
                current_number->info.code = NULL;
            }
            transplantation(refal.previous_result, current_argument, right_bracket);
            return;
        }
        uint64_t skip_numbers = have_numbers - need_numbers;
        begin_numbers = current_argument->next;
        while (skip_numbers-- != 0)
            begin_numbers = begin_numbers->next;
        transplantation(refal.previous_result, begin_numbers->previous, right_bracket);
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char bit_0[] = {Z3 'B', 'I', 'T', (char)3};
G_L_B uint8_t refalab_bit = '\122';
void (*bit_1)(void) = bit_;

//----------  end of file xbit.c  ----------
