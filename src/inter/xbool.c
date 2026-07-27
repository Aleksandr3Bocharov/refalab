// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-07-27
// https://github.com/Aleksandr3Bocharov/refalab

//-----------  file xbool.c  ----------
//                 Bool MO:
//             Existw, Existn
//-------------------------------------

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "refalab.h"
#include "interface.h"

extern uint8_t refalab_true, refalab_false;

static void existw_(void)
{
    if (refal.previous_argument->next == refal.next_argument)
    {
        refal.upshot = 2;
        return;
    }
    refal.previous_argument->info.codef = &refalab_false;
    const T_LINKCB *exist_term_begin = refal.previous_argument->next;
    const T_LINKCB *exist_term_end = exist_term_begin;
    if (exist_term_begin->tag == TAGLB)
        exist_term_end = exist_term_begin->info.codep;
    const T_LINKCB *current_argument = exist_term_end->next;
    while (current_argument != refal.next_argument)
    {
        if (find_duplicate(exist_term_begin->previous, exist_term_end->next, current_argument->previous) != NULL)
        {
            refal.previous_argument->info.codef = &refalab_true;
            break;
        }
        if (current_argument->tag == TAGLB)
            current_argument = current_argument->info.codep;
        current_argument = current_argument->next;
    }
    transplantation(refal.previous_result, refal.previous_argument->previous, refal.previous_argument->next);
    return;
}
char existw_0[] = {Z6 'E', 'X', 'I', 'S', 'T', 'W', (char)6};
G_L_B uint8_t refalab_existw = '\122';
void (*existw_1)(void) = existw_;

static void existn_(void)
{
    do
    {
        const T_LINKCB *exist_big_number_begin = refal.previous_argument->next;
        if (exist_big_number_begin->tag != TAGLB)
            break;
        const T_LINKCB *exist_big_number_end = exist_big_number_begin->info.codep;
        T_BIG_NUMBER Exist_Big_Number;
        if (!read_big_number_expression(&Exist_Big_Number, exist_big_number_begin, exist_big_number_end))
            break;
        const T_LINKCB *current_argument_begin = exist_big_number_end->next;
        bool impossible = false;
        bool exist_big_number = false;
        while (current_argument_begin != refal.next_argument)
        {
            if (current_argument_begin->tag != TAGLB)
            {
                impossible = true;
                break;
            }
            const T_LINKCB *current_argument_end = current_argument_begin->info.codep;
            T_BIG_NUMBER Current_Big_Number;
            if (!read_big_number_expression(&Current_Big_Number, current_argument_begin, current_argument_end))
            {
                impossible = true;
                break;
            }
            if (compare_big_numbers(&Exist_Big_Number, &Current_Big_Number) == 0)
            {
                exist_big_number = true;
                current_argument_begin = current_argument_end->next;
                break;
            }
            current_argument_begin = current_argument_end->next;
        }
        if (impossible)
            break;
        while (current_argument_begin != refal.next_argument)
        {
            if (current_argument_begin->tag != TAGLB)
            {
                impossible = true;
                break;
            }
            const T_LINKCB *current_argument_end = current_argument_begin->info.codep;
            T_BIG_NUMBER Current_Big_Number;
            if (!read_big_number_expression(&Current_Big_Number, current_argument_begin, current_argument_end))
            {
                impossible = true;
                break;
            }
            current_argument_begin = current_argument_end->next;
        }
        if (impossible)
            break;
        if (exist_big_number)
            refal.previous_argument->info.codef = &refalab_true;
        else
            refal.previous_argument->info.codef = &refalab_false;
        transplantation(refal.previous_result, refal.previous_argument->previous, refal.previous_argument->next);
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char existn_0[] = {Z6 'E', 'X', 'I', 'S', 'T', 'N', (char)6};
G_L_B uint8_t refalab_existn = '\122';
void (*existn_1)(void) = existn_;

//----------  end of file xbool.c  ----------
