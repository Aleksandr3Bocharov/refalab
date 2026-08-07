// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-08-07
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file xbitarray.c  ----------
//                Bitwise MO:
//      Bit, Bitand, Bitor, Bitxor,
//      Bitnot, Bitshl, Bitshr, Bitrot
//----------------------------------------

#include <stddef.h>
#include <stdint.h>
#include "refalab.h"
#include "interface.h"

#define Oand 1
#define Oor 2
#define Oxor 3
#define Onot 4

static void bit_operate(uint8_t operation);

static void bitand_(void) { bit_operate(Oand); }
char bitand_0[] = {Z6 'B', 'I', 'T', 'A', 'N', 'D', (char)6};
G_L_B uint8_t refalab_bitand = '\122';
void (*bitand_1)(void) = bitand_;

static void bitor_(void) { bit_operate(Oor); }
char bitor_0[] = {Z5 'B', 'I', 'T', 'O', 'R', (char)5};
G_L_B uint8_t refalab_bitor = '\122';
void (*bitor_1)(void) = bitor_;

static void bitxor_(void) { bit_operate(Oxor); }
char bitxor_0[] = {Z6 'B', 'I', 'T', 'X', 'O', 'R', (char)6};
G_L_B uint8_t refalab_bitxor = '\122';
void (*bitxor_1)(void) = bitxor_;

static void bitnot_(void) { bit_operate(Onot); }
char bitnot_0[] = {Z6 'B', 'I', 'T', 'N', 'O', 'T', (char)6};
G_L_B uint8_t refalab_bitnot = '\122';
void (*bitnot_1)(void) = bitnot_;

static void bit_operate(uint8_t operation)
{
    do
    {
        T_LINKCB *current_argument = refal.previous_argument->next;
        if (operation == Onot)
        {
            if (current_argument->tag != TAGN)
                break;
            T_LINKCB *begin_argument = current_argument;
            T_LINKCB *end_argument = current_argument;
            while (end_argument != refal.next_argument)
            {
                if (end_argument->tag != TAGN)
                    break;
                end_argument = end_argument->next;
            }
            if (end_argument != refal.next_argument)
                break;
            for (current_argument = begin_argument; current_argument != end_argument; current_argument = current_argument->next)
                pcoden(current_argument, ~gcoden(current_argument));
            transplantation(refal.previous_result, begin_argument->previous, end_argument);
            return;
        }
        if (current_argument->tag != TAGLB)
            break;
        T_LINKCB *right_bracket = current_argument->info.codep;
        T_LINKCB *x_begin = current_argument->next;
        T_LINKCB *y_begin = right_bracket->next;
        T_LINKCB *x = x_begin;
        T_LINKCB *y = y_begin;
        while (x != right_bracket && y != refal.next_argument)
        {
            if (x->tag != TAGN || y->tag != TAGN)
                break;
            x = x->next;
            y = y->next;
        }
        if (x != right_bracket || y != refal.next_argument)
            break;
        x = x_begin;
        y = y_begin;
        switch (operation)
        {
        case Oand:
            while (x != right_bracket)
            {
                pcoden(x, gcoden(x) & gcoden(y));
                x = x->next;
                y = y->next;
            }
            break;
        case Oor:
            while (x != right_bracket)
            {
                pcoden(x, gcoden(x) | gcoden(y));
                x = x->next;
                y = y->next;
            }
            break;
        case Oxor:
            while (x != right_bracket)
            {
                pcoden(x, gcoden(x) ^ gcoden(y));
                x = x->next;
                y = y->next;
            }
            break;
        }
        transplantation(refal.previous_result, current_argument, right_bracket);
        return;
    } while (false);
    refal.upshot = 2;
    return;
}

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

//----------  end of file xbitarray.c  ----------
