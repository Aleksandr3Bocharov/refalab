// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-04-28
// https://github.com/Aleksandr3Bocharov/refalab

//------------ file -- XCOMP.C ---------------
//   MO: nrel, lrel
//--------------------------------------------

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "refalab.h"
#include "rfintf.h"

static T_LINKCB *x_current, *y_current, *X_begin, *X_end, *begin, *end, *Y_begin, *Y_end;
static size_t length, X_length, Y_length;
static char sign, X_sign, Y_sign;

static bool read_number(void)
{
    sign = '+';
    end = y_current->previous;
    if (x_current == end)
    { // pustoe chislo
        length = 0;
        return true;
    }
    x_current = x_current->next;
    if (x_current->tag == TAGO &&
        (x_current->info.infoc == '+' || x_current->info.infoc == '-'))
    {
        sign = x_current->info.infoc;
        x_current = x_current->next;
        if (x_current == y_current)
            return false; //  w chisle - lish znak
    }
    for (; x_current->tag == TAGN && gcoden(x_current) == 0; x_current = x_current->next)
        ;
    if (x_current == y_current)
        length = 0; //  wse cifry - nuli
    else
    {
        for (length = 0, begin = x_current; x_current->tag == TAGN; x_current = x_current->next, length++)
            ;
        if (x_current != y_current)
            return false; // ne makrocifra
    }
    return true;
}

static bool read_numbers(void)
{
    x_current = refal.previous_argument->next;
    if (x_current->tag != TAGLB)
        return false;
    y_current = x_current->info.codep;
    if (read_number())
    {
        X_begin = begin;
        X_end = end;
        X_sign = sign;
        X_length = length;
    }
    else
        return false;
    x_current = y_current;
    y_current = refal.next_argument;
    if (read_number())
    {
        Y_begin = begin;
        Y_end = end;
        Y_sign = sign;
        Y_length = length;
        return true;
    }
    else
        return false;
}

static uint32_t compare_numbers(void)
{ //  if X < Y then true  ( po modulju)
    if (X_length < Y_length)
        return 1;
    if (X_length > Y_length)
        return 0;
    for (x_current = X_begin, y_current = Y_begin; x_current != X_end->next; x_current = x_current->next, y_current = y_current->next)
    {
        if (gcoden(x_current) < gcoden(y_current))
            return 1;
        if (gcoden(x_current) > gcoden(y_current))
            return 0;
    }
    return 2; // X=Y
}

static void nrel_(void)
{
    if (!read_numbers())
    {
        refal.upshot = 2;
        return;
    }
    char compare_result;
    if (X_length == 0 && Y_length == 0)
        compare_result = '=';
    else
    {
        const uint32_t compare = compare_numbers();
        if (X_sign == Y_sign && compare == 2)
            compare_result = '=';
        else
        {
            if ((X_sign == '-' && Y_sign == '+') ||
                (X_sign == '-' && Y_sign == '-' && compare == 0) ||
                (X_sign == '+' && Y_sign == '+' && compare == 1))
                compare_result = '<';
            else
                compare_result = '>';
        }
    }
    refal.previous_argument->tag = TAGO;
    refal.previous_argument->info.code = NULL;
    refal.previous_argument->info.infoc = compare_result;
    transplantation(refal.previous_result, refal.previous_argument->previous, refal.next_argument);
    return;
}
char nrel_0[] = {Z4 'N', 'R', 'E', 'L', (char)4};
G_L_B uint8_t refalab_nrel = '\122';
void (*nrel_1)(void) = nrel_;

static void lrel_(void)
{
    const T_LINKCB *first_begin = refal.previous_argument->next;
    if (first_begin->tag != TAGLB)
    {
        refal.upshot = 2;
        return;
    }
    const T_LINKCB *second_begin = first_begin->info.codep;
    const T_LINKCB *first_current = first_begin->next;
    const T_LINKCB *second_current = second_begin->next;
    char compare_result = '=';
    for (; compare_result == '=' && first_current != second_begin && second_current != refal.next_argument; first_current = first_current->next, second_current = second_current->next)
        if ((first_current->tag == TAGLB && second_current->tag == TAGLB) ||
            (first_current->tag == TAGRB && second_current->tag == TAGRB))
            continue;
        else if (first_current->tag == TAGLB || second_current->tag == TAGRB)
            compare_result = '>';
        else if (first_current->tag == TAGRB || second_current->tag == TAGLB)
            compare_result = '<';
        else if (first_current->tag > second_current->tag)
            compare_result = '>';
        else if (first_current->tag < second_current->tag)
            compare_result = '<';
        else if ((size_t)first_current->info.code > (size_t)second_current->info.code)
            compare_result = '>';
        else if ((size_t)first_current->info.code < (size_t)second_current->info.code)
            compare_result = '<';
    if (compare_result == '=')
    {
        if (first_current == second_begin && second_current != refal.next_argument)
            compare_result = '<';
        else if (second_current == refal.next_argument && first_current != second_begin)
            compare_result = '>';
    }
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
