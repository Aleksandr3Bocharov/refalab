// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-05-18
// https://github.com/Aleksandr3Bocharov/refalab

//-------------- file -- XMO.C -------------
//                 Lexical MO:
//     numb, symb, first, last,
//     lengr, lengw, multe,
//     chr, ord, upper, lower,
//     empty, del_first2, del_lastw,
//     unbrackets
//------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include "refalab.h"
#include "rfintf.h"

static void numb_(void)
{
    T_LINKCB *current_argument = refal.previous_argument->next;
    const char sign = current_argument->info.infoc;
    const T_LINKCB *sign_argument = current_argument;
    if (current_argument->tag == TAGO && (sign == '-' || sign == '+'))
    {
        current_argument = current_argument->next;
        if (sign == '+')
            sign_argument = current_argument;
    }
    T_LINKCB *number_argument = current_argument;
    while (current_argument->tag == TAGO && current_argument->info.infoc == '0')
        current_argument = current_argument->next;
    char number_string[11];
    uint8_t i;
    bool impossible = false;
    for (i = 0; current_argument != refal.next_argument; i++)
    {
        if (current_argument->tag != TAGO || i == 10)
        {
            impossible = true;
            break;
        }
        number_string[i] = current_argument->info.infoc;
        if (number_string[i] < '0' || number_string[i] > '9')
        {
            impossible = true;
            break;
        }
        if (i == 9 && strncmp(number_string, "4294967295", i + 1) > 0)
        {
            impossible = true;
            break;
        }
        current_argument = current_argument->next;
    }
    if (impossible)
    {
        refal.upshot = 2;
        return;
    }
    number_string[i] = '\0';
    if (strlen(number_string) == 0)
    {
        number_argument = refal.previous_argument;
        sign_argument = number_argument;
    }
    number_argument->tag = TAGN;
    number_argument->info.code = NULL;
    pcoden(number_argument, (uint32_t)atoll(number_string));
    transplantation(refal.previous_result, sign_argument->previous, number_argument->next);
    return;
}
char numb_0[] = {Z4 'N', 'U', 'M', 'B', (char)4};
G_L_B uint8_t refalab_numb = '\122';
void (*numb_1)(void) = numb_;

static void symb_(void)
{
    T_LINKCB *current_argument = refal.previous_argument->next;
    size_t argument_length = 1;
    char sign = current_argument->info.infoc;
    T_LINKCB *sign_argument = current_argument;
    if (current_argument->tag == TAGO && (sign == '-' || sign == '+'))
    {
        argument_length++;
        current_argument = current_argument->next;
        if (sign == '+')
            sign_argument = current_argument;
    }
    else
        sign = '+';
    T_LINKCB *begin_number_argument = current_argument;
    while (current_argument->tag == TAGN && gcoden(current_argument) == 0)
    {
        argument_length++;
        current_argument = current_argument->next;
    }
    uint8_t i;
    for (i = 0; current_argument != refal.next_argument; i++, argument_length++, current_argument = current_argument->next)
        if (current_argument->tag != TAGN || i == 1)
        {
            refal.upshot = 2;
            return;
        }
    if (i == 0)
    {
        refal.previous_argument->tag = TAGO;
        refal.previous_argument->info.code = NULL;
        refal.previous_argument->info.infoc = '0';
        transplantation(refal.previous_result, refal.previous_argument->previous, refal.previous_argument->next);
        return;
    }
    current_argument = current_argument->previous;
    uint32_t number = gcoden(current_argument);
    char number_string[11];
    sprintf(number_string, "%u", number);
    const size_t number_length = strlen(number_string);
    const size_t result_need = number_length + (sign == '-' ? 1 : 0);
    if (result_need > argument_length)
        if (!extended_insert_from_free_memory(refal.next_result, result_need - argument_length))
            return;
    transplantation(refal.next_argument->previous, refal.next_result, sign_argument);
    for (i = 0, current_argument = begin_number_argument; i < number_length; i++, current_argument = current_argument->next)
    {
        current_argument->tag = TAGO;
        current_argument->info.code = NULL;
        current_argument->info.infoc = number_string[i];
    }
    transplantation(refal.previous_result, sign_argument->previous, current_argument);
    return;
}
char symb_0[] = {Z4 'S', 'Y', 'M', 'B', (char)4};
G_L_B uint8_t refalab_symb = '\122';
void (*symb_1)(void) = symb_;

static void first_(void)
{
    T_LINKCB *first_argument = refal.previous_argument->next;
    if (first_argument == refal.next_argument || first_argument->tag != TAGN)
    {
        refal.upshot = 2;
        return;
    }; // FAIL
    const uint32_t number = gcoden(first_argument);
    T_LINKCB *current_argument = first_argument;
    for (uint32_t k = 1; k <= number; k++)
    {
        current_argument = current_argument->next;
        if (current_argument == refal.next_argument)
        {
            first_argument->info.code = NULL;
            first_argument->info.infoc = '*';
            first_argument->tag = TAGO;
            transplantation(refal.previous_result, refal.previous_argument, refal.next_argument);
            return;
        }
        if (current_argument->tag == TAGLB)
            current_argument = current_argument->info.codep;
    }
    current_argument = current_argument->next;
    refal.previous_argument->tag = TAGLB;
    refal.previous_argument->info.codep = first_argument;
    first_argument->tag = TAGRB;
    first_argument->info.codep = refal.previous_argument;
    transplantation(refal.previous_argument, first_argument, current_argument);
    transplantation(refal.previous_result, refal.next_result, refal.next_argument);
    return;
}
char first_0[] = {Z5 'F', 'I', 'R', 'S', 'T', (char)5};
G_L_B uint8_t refalab_first = '\122';
void (*first_1)(void) = first_;

static void last_(void)
{
    T_LINKCB *first_argument = refal.previous_argument->next;
    if (first_argument == refal.next_argument || first_argument->tag != TAGN)
    {
        refal.upshot = 2;
        return;
    }; // FAIL
    const uint32_t number = gcoden(first_argument);
    T_LINKCB *current_argument = refal.next_argument;
    for (uint32_t k = 1; k <= number; k++)
    {
        current_argument = current_argument->previous;
        if (current_argument == first_argument)
        {
            first_argument->tag = TAGO;
            first_argument->info.code = NULL;
            first_argument->info.infoc = '*';
            transplantation(refal.previous_result, first_argument, refal.next_argument);
            transplantation(refal.next_result->previous, first_argument->previous, first_argument->next);
            return;
        }
        if (current_argument->tag == TAGRB)
            current_argument = current_argument->info.codep;
    }
    current_argument = current_argument->previous;
    refal.previous_argument->tag = TAGLB;
    refal.previous_argument->info.codep = first_argument;
    first_argument->tag = TAGRB;
    first_argument->info.codep = refal.previous_argument;
    transplantation(refal.previous_argument, current_argument, refal.next_argument);
    transplantation(refal.previous_result, first_argument, refal.next_argument);
    transplantation(refal.next_result->previous, refal.next_result, refal.next_argument);
    return;
}
char last_0[] = {Z4 'L', 'A', 'S', 'T', (char)4};
G_L_B uint8_t refalab_last = '\122';
void (*last_1)(void) = last_;

static void lengr_(void)
{
    uint32_t length = 0;
    const T_LINKCB *current_argument = refal.previous_argument->next;
    while (current_argument != refal.next_argument)
    {
        length++;
        current_argument = current_argument->next;
    }
    refal.previous_argument->tag = TAGN;
    refal.previous_argument->info.code = NULL;
    pcoden(refal.previous_argument, length);
    transplantation(refal.previous_result, refal.next_result, refal.next_argument);
    return;
}
char lengr_0[] = {Z5 'L', 'E', 'N', 'G', 'R', (char)5};
G_L_B uint8_t refalab_lengr = '\122';
void (*lengr_1)(void) = lengr_;

static void lengw_(void)
{
    uint32_t length = 0;
    const T_LINKCB *current_argument = refal.previous_argument->next;
    while (current_argument != refal.next_argument)
    {
        length++;
        if (current_argument->tag == TAGLB)
            current_argument = current_argument->info.codep;
        current_argument = current_argument->next;
    }
    refal.previous_argument->tag = TAGN;
    refal.previous_argument->info.code = NULL;
    pcoden(refal.previous_argument, length);
    transplantation(refal.previous_result, refal.next_result, refal.next_argument);
    return;
}
char lengw_0[] = {Z5 'L', 'E', 'N', 'G', 'W', (char)5};
G_L_B uint8_t refalab_lengw = '\122';
void (*lengw_1)(void) = lengw_;

static void multe_(void)
{
    const T_LINKCB *symbol_number = refal.previous_argument->next;
    if (symbol_number == refal.next_argument || symbol_number->tag != TAGN)
    {
        refal.upshot = 2;
        return;
    }; // FAIL
    uint32_t number = gcoden(symbol_number);
    if (number == 0)
        return;
    const T_LINKCB *first_expression = symbol_number->next;
    if (first_expression == refal.next_argument)
        return;
    if (first_expression->next != refal.next_argument)
    {
        do
        {
            if (!copy_expression(refal.next_result->previous, first_expression->previous, refal.next_argument))
            {
                refal.upshot = 3;
                return;
            }; // LACK
            number--;
        } while (number >= 1);
    }
    else
    {
        if (number > 3)
            if (!extended_insert_from_free_memory(refal.next_result, number - 3))
                return; //  LACK
        const uint32_t expression_tag = first_expression->tag;
        void *expression_code = first_expression->info.code;
        T_LINKCB *current_argument = refal.next_result;
        for (uint32_t i = 0; i < number; i++)
        {
            current_argument = current_argument->next;
            current_argument->tag = expression_tag;
            current_argument->info.code = expression_code;
        }
        transplantation(refal.previous_result, refal.next_result, current_argument->next);
    }
    return;
}
char multe_0[] = {Z5 'M', 'U', 'L', 'T', 'E', (char)5};
G_L_B uint8_t refalab_multe = '\122';
void (*multe_1)(void) = multe_;

static void chr_(void)
{
    T_LINKCB *current_argument = refal.previous_argument->next;
    while (current_argument != refal.next_argument)
    {
        if (current_argument->tag == TAGN)
        {
            current_argument->tag = TAGO;
            current_argument->info.code = NULL;
            current_argument->info.infoc = (char)gcoden(current_argument);
        }
        current_argument = current_argument->next;
    }
    transplantation(refal.previous_result, refal.previous_argument, refal.next_argument);
    return;
}
char chr_0[] = {Z3 'C', 'H', 'R', (char)3};
G_L_B uint8_t refalab_chr = '\122';
void (*chr_1)(void) = chr_;

static void ord_(void)
{
    T_LINKCB *current_argument = refal.previous_argument->next;
    while (current_argument != refal.next_argument)
    {
        if (current_argument->tag == TAGO)
            current_argument->tag = TAGN;
        current_argument = current_argument->next;
    }
    transplantation(refal.previous_result, refal.previous_argument, refal.next_argument);
    return;
}
char ord_0[] = {Z3 'O', 'R', 'D', (char)3};
G_L_B uint8_t refalab_ord = '\122';
void (*ord_1)(void) = ord_;

static void upper_(void)
{
    T_LINKCB *current_argument = refal.previous_argument->next;
    while (current_argument != refal.next_argument)
    {
        if (current_argument->tag == TAGO)
            current_argument->info.infoc = (char)toupper(current_argument->info.infoc);
        current_argument = current_argument->next;
    }
    transplantation(refal.previous_result, refal.previous_argument, refal.next_argument);
    return;
}
char upper_0[] = {Z5 'U', 'P', 'P', 'E', 'R', (char)5};
G_L_B uint8_t refalab_upper = '\122';
void (*upper_1)(void) = upper_;

static void lower_(void)
{
    T_LINKCB *current_argument = refal.previous_argument->next;
    while (current_argument != refal.next_argument)
    {
        if (current_argument->tag == TAGO)
            current_argument->info.infoc = (char)tolower(current_argument->info.infoc);
        current_argument = current_argument->next;
    }
    transplantation(refal.previous_result, refal.previous_argument, refal.next_argument);
    return;
}
char lower_0[] = {Z5 'L', 'O', 'W', 'E', 'R', (char)5};
G_L_B uint8_t refalab_lower = '\122';
void (*lower_1)(void) = lower_;

static void empty_(void)
{
    return;
}
char empty_0[] = {Z5 'E', 'M', 'P', 'T', 'Y', (char)5};
G_L_B uint8_t refalab_empty = '\122';
void (*empty_1)(void) = empty_;

static void del_firstw_(void)
{
    T_LINKCB *first_term = refal.previous_argument->next;
    if (first_term == refal.next_argument)
    {
        refal.upshot = 2;
        return;
    }
    if (first_term->tag == TAGLB)
        first_term = first_term->info.codep;
    transplantation(refal.previous_result, first_term, refal.next_argument);
    return;
}
char del_firstw_0[] = {Z2 'D', 'E', 'L', '_', 'F', 'I', 'R', 'S', 'T', 'W', (char)10};
G_L_B uint8_t refalab_del_firstw = '\122';
void (*del_firstw_1)(void) = del_firstw_;

static void del_lastw_(void)
{
    T_LINKCB *last_term = refal.next_argument->previous;
    if (refal.previous_argument == last_term)
    {
        refal.upshot = 2;
        return;
    }
    if (last_term->tag == TAGRB)
        last_term = last_term->info.codep;
    transplantation(refal.previous_result, refal.previous_argument, last_term);
    return;
}
char del_lastw_0[] = {Z1 'D', 'E', 'L', '_', 'L', 'A', 'S', 'T', 'W', (char)9};
G_L_B uint8_t refalab_del_lastw = '\122';
void (*del_lastw_1)(void) = del_lastw_;

static void unbrackets_(void)
{
    T_LINKCB *left_bracket = refal.previous_argument->next;
    T_LINKCB *right_bracket = left_bracket->info.codep;
    if (left_bracket->tag != TAGLB || right_bracket->next != refal.next_argument)
    {
        refal.upshot = 2;
        return;
    }
    transplantation(refal.previous_result, left_bracket, right_bracket);
    return;
}
char unbrackets_0[] = {Z2 'U', 'N', 'B', 'R', 'A', 'C', 'K', 'E', 'T', 'S', (char)10};
G_L_B uint8_t refalab_unbrackets = '\122';
void (*unbrackets_1)(void) = unbrackets_;

//-------------------- end of file  XMO.C ----------------
