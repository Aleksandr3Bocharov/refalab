// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-05-11
// https://github.com/Aleksandr3Bocharov/refalab

//------------ file -- XCOMP.C ---------------
//   MO: nrel, lrel
//--------------------------------------------

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "refalab.h"
#include "rfintf.h"

extern uint8_t refalab_true, refalab_false;

static void nrel_(void)
{
    const T_LINKCB *x_current = refal.previous_argument->next;
    const T_LINKCB *y_current = x_current->info.codep;
    T_BIG_NUMBER X, Y;
    if (x_current->tag != TAGLB || !read_big_numbers_expression(&X, &Y, x_current, y_current, refal.next_argument))
    {
        refal.upshot = 2;
        return;
    }
    char compare_result = '=';
    const uint8_t compare = compare_big_numbers(&X, &Y);
    if (compare == 1)
        compare_result = '<';
    else if (compare == 0)
        compare_result = '>';
    refal.previous_argument->tag = TAGO;
    refal.previous_argument->info.code = NULL;
    refal.previous_argument->info.infoc = compare_result;
    transplantation(refal.previous_result, refal.previous_argument->previous, refal.next_argument);
    return;
}
char nrel_0[] = {Z4 'N', 'R', 'E', 'L', (char)4};
G_L_B uint8_t refalab_nrel = '\122';
void (*nrel_1)(void) = nrel_;

static void ltn_(void)
{
    const T_LINKCB *x_current = refal.previous_argument->next;
    const T_LINKCB *y_current = x_current->info.codep;
    T_BIG_NUMBER X, Y;
    if (x_current->tag != TAGLB || !read_big_numbers_expression(&X, &Y, x_current, y_current, refal.next_argument))
    {
        refal.upshot = 2;
        return;
    }
    char compare_result = '=';
    const uint8_t compare = compare_big_numbers(&X, &Y);
    if (compare == 1)
        compare_result = '<';
    else if (compare == 0)
        compare_result = '>';
    refal.previous_argument->tag = TAGO;
    refal.previous_argument->info.code = NULL;
    refal.previous_argument->info.infoc = compare_result;
    transplantation(refal.previous_result, refal.previous_argument->previous, refal.next_argument);
    return;
}
char ltn_0[] = {Z3 'L', 'T', 'N', (char)3};
G_L_B uint8_t refalab_ltn = '\122';
void (*ltn_1)(void) = ltn_;

static uint8_t compare_expressions_lexicographic(const T_LINKCB *before, const T_LINKCB *middle, const T_LINKCB *after)
{
    const T_LINKCB *first_current = before->next;
    const T_LINKCB *second_current = middle->next;
    for (; first_current != middle && second_current != after; first_current = first_current->next, second_current = second_current->next)
        if ((first_current->tag == TAGLB && second_current->tag == TAGLB) || (first_current->tag == TAGRB && second_current->tag == TAGRB))
            continue;
        else if (first_current->tag == TAGLB || second_current->tag == TAGRB)
            return 0; //X>Y
        else if (first_current->tag == TAGRB || second_current->tag == TAGLB)
            return 1; //X<Y
        else if (first_current->tag > second_current->tag)
            return 0; //X>Y
        else if (first_current->tag < second_current->tag)
            return 1; //X<Y
        else if ((size_t)first_current->info.code > (size_t)second_current->info.code)
            return 0; //X>Y
        else if ((size_t)first_current->info.code < (size_t)second_current->info.code)
            return 1; //X<Y
    if (first_current == middle && second_current != after)
        return 1; //X<Y
    if (second_current == after && first_current != middle)
        return 0; //X>Y
    return 2; //X=Y
}

static void lrel_(void)
{
    const T_LINKCB *first_begin = refal.previous_argument->next;
    if (first_begin->tag != TAGLB)
    {
        refal.upshot = 2;
        return;
    }
    const T_LINKCB *second_begin = first_begin->info.codep;
    char compare_result = '=';
    const uint8_t compare = compare_expressions_lexicographic(first_begin, second_begin, refal.next_argument);
    if (compare == 0)
        compare_result = '>';
    else if (compare == 1)
        compare_result = '<';
    refal.previous_argument->tag = TAGO;
    refal.previous_argument->info.code = NULL;
    refal.previous_argument->info.infoc = compare_result;
    transplantation(refal.previous_result, refal.previous_argument->previous, refal.previous_argument->next);
    return;
}
char lrel_0[] = {Z4 'L', 'R', 'E', 'L', (char)4};
G_L_B uint8_t refalab_lrel = '\122';
void (*lrel_1)(void) = lrel_;

//------------------ end of file  XCOMP.C ----------------
