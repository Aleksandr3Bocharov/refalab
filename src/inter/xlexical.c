// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-07-25
// https://github.com/Aleksandr3Bocharov/refalab

//-----------  file xlexical.c  ----------
//                 Lexical MO:
//     Numb, Symb, First, Last,
//     Left, Right, Del_left, Del_right,
//     Length, Lengthw, Multe,
//     Chr, Ord, Upper, Lower,
//     Empty, Unbrackets, Reverse
//----------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include "refalab.h"
#include "interface.h"

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
    sprintf(number_string, "%" PRIu32, number);
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
    do
    {
        T_LINKCB *first_argument = refal.previous_argument->next;
        if (first_argument == refal.next_argument || first_argument->tag != TAGN)
            break;
        const uint32_t number = gcoden(first_argument);
        T_LINKCB *current_argument = first_argument;
        bool impossible = false;
        for (uint32_t k = 0; k < number; k++)
        {
            current_argument = current_argument->next;
            if (current_argument == refal.next_argument)
            {
                impossible = true;
                break;
            }
            if (current_argument->tag == TAGLB)
                current_argument = current_argument->info.codep;
        }
        if (impossible)
            break;
        current_argument = current_argument->next;
        refal.previous_argument->tag = TAGLB;
        refal.previous_argument->info.codep = first_argument;
        first_argument->tag = TAGRB;
        first_argument->info.codep = refal.previous_argument;
        transplantation(refal.previous_argument, first_argument, current_argument);
        transplantation(refal.previous_result, refal.next_result, refal.next_argument);
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char first_0[] = {Z5 'F', 'I', 'R', 'S', 'T', (char)5};
G_L_B uint8_t refalab_first = '\122';
void (*first_1)(void) = first_;

static void last_(void)
{
    do
    {
        T_LINKCB *first_argument = refal.previous_argument->next;
        if (first_argument == refal.next_argument || first_argument->tag != TAGN)
            break;
        const uint32_t number = gcoden(first_argument);
        T_LINKCB *current_argument = refal.next_argument;
        bool impossible = false;
        for (uint32_t k = 0; k < number; k++)
        {
            current_argument = current_argument->previous;
            if (current_argument == first_argument)
            {
                impossible = true;
                break;
            }
            if (current_argument->tag == TAGRB)
                current_argument = current_argument->info.codep;
        }
        if (impossible)
            break;
        current_argument = current_argument->previous;
        refal.previous_argument->tag = TAGLB;
        refal.previous_argument->info.codep = first_argument;
        first_argument->tag = TAGRB;
        first_argument->info.codep = refal.previous_argument;
        transplantation(refal.previous_argument, current_argument, refal.next_argument);
        transplantation(refal.previous_result, first_argument, refal.next_argument);
        transplantation(refal.next_result->previous, refal.next_result, refal.next_argument);
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char last_0[] = {Z4 'L', 'A', 'S', 'T', (char)4};
G_L_B uint8_t refalab_last = '\122';
void (*last_1)(void) = last_;

static void left_(void)
{
    do
    {
        T_LINKCB *first_argument = refal.previous_argument->next;
        if (first_argument == refal.next_argument || first_argument->tag != TAGN)
            break;
        const uint32_t number = gcoden(first_argument);
        if (number == 0)
            break;
        T_LINKCB *current_argument = first_argument;
        bool impossible = false;
        for (uint32_t k = 0; k < number; k++)
        {
            current_argument = current_argument->next;
            if (current_argument == refal.next_argument)
            {
                impossible = true;
                break;
            }
            if (current_argument->tag == TAGLB)
                current_argument = current_argument->info.codep;
        }
        if (impossible)
            break;
        T_LINKCB *current_argument_begin = current_argument;
        if (current_argument->tag == TAGRB)
            current_argument_begin = current_argument->info.codep;
        transplantation(refal.previous_result, current_argument_begin->previous, current_argument->next);
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char left_0[] = {Z4 'L', 'E', 'F', 'T', (char)4};
G_L_B uint8_t refalab_left = '\122';
void (*left_1)(void) = left_;

static void right_(void)
{
    do
    {
        T_LINKCB *first_argument = refal.previous_argument->next;
        if (first_argument == refal.next_argument || first_argument->tag != TAGN)
            break;
        const uint32_t number = gcoden(first_argument);
        if (number == 0)
            break;
        T_LINKCB *current_argument = refal.next_argument;
        bool impossible = false;
        for (uint32_t k = 0; k < number; k++)
        {
            current_argument = current_argument->previous;
            if (current_argument == first_argument)
            {
                impossible = true;
                break;
            }
            if (current_argument->tag == TAGRB)
                current_argument = current_argument->info.codep;
        }
        if (impossible)
            break;
        T_LINKCB *current_argument_end = current_argument;
        if (current_argument->tag == TAGLB)
            current_argument_end = current_argument->info.codep;
        transplantation(refal.previous_result, current_argument->previous, current_argument_end->next);
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char right_0[] = {Z5 'R', 'I', 'G', 'H', 'T', (char)5};
G_L_B uint8_t refalab_right = '\122';
void (*right_1)(void) = right_;

static void del_left_(void)
{
    do
    {
        T_LINKCB *first_argument = refal.previous_argument->next;
        if (first_argument == refal.next_argument || first_argument->tag != TAGN)
            break;
        const uint32_t number = gcoden(first_argument);
        if (number == 0)
            break;
        T_LINKCB *current_argument = first_argument;
        bool impossible = false;
        for (uint32_t k = 0; k < number; k++)
        {
            current_argument = current_argument->next;
            if (current_argument == refal.next_argument)
            {
                impossible = true;
                break;
            }
            if (current_argument->tag == TAGLB)
                current_argument = current_argument->info.codep;
        }
        if (impossible)
            break;
        T_LINKCB *current_argument_begin = current_argument;
        if (current_argument->tag == TAGRB)
            current_argument_begin = current_argument->info.codep;
        insert_to_free_memory(current_argument_begin->previous, current_argument->next);
        transplantation(refal.previous_result, first_argument, refal.next_argument);
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char del_left_0[] = {Z0 'D', 'E', 'L', '_', 'L', 'E', 'F', 'T', (char)8};
G_L_B uint8_t refalab_del_left = '\122';
void (*del_left_1)(void) = del_left_;

static void del_right_(void)
{
    do
    {
        T_LINKCB *first_argument = refal.previous_argument->next;
        if (first_argument == refal.next_argument || first_argument->tag != TAGN)
            break;
        const uint32_t number = gcoden(first_argument);
        if (number == 0)
            break;
        T_LINKCB *current_argument = refal.next_argument;
        bool impossible = false;
        for (uint32_t k = 0; k < number; k++)
        {
            current_argument = current_argument->previous;
            if (current_argument == first_argument)
            {
                impossible = true;
                break;
            }
            if (current_argument->tag == TAGRB)
                current_argument = current_argument->info.codep;
        }
        if (impossible)
            break;
        T_LINKCB *current_argument_end = current_argument;
        if (current_argument->tag == TAGLB)
            current_argument_end = current_argument->info.codep;
        insert_to_free_memory(current_argument->previous, current_argument_end->next);
        transplantation(refal.previous_result, first_argument, refal.next_argument);
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char del_right_0[] = {Z1 'D', 'E', 'L', '_', 'R', 'I', 'G', 'H', 'T', (char)9};
G_L_B uint8_t refalab_del_right = '\122';
void (*del_right_1)(void) = del_right_;

static void length_(void)
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
char length_0[] = {Z6 'L', 'E', 'N', 'G', 'T', 'H', (char)6};
G_L_B uint8_t refalab_length = '\122';
void (*length_1)(void) = length_;

static void lengthw_(void)
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
char lengthw_0[] = {Z7 'L', 'E', 'N', 'G', 'T', 'H', 'W', (char)7};
G_L_B uint8_t refalab_lengthw = '\122';
void (*lengthw_1)(void) = lengthw_;

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
            current_argument->info.infoc = (char)toupper((unsigned char)current_argument->info.infoc);
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
            current_argument->info.infoc = (char)tolower((unsigned char)current_argument->info.infoc);
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

static void reverse_(void)
{
    if (refal.previous_argument->next == refal.next_argument)
        return;
    T_LINKCB *current_argument = refal.previous_argument->next;
    while (current_argument != refal.next_argument)
    {
        T_LINKCB *next_argument = current_argument->next;
        current_argument->next = current_argument->previous;
        current_argument->previous = next_argument;
        if (current_argument->tag == TAGLB)
            current_argument->tag = TAGRB;
        else if (current_argument->tag == TAGRB)
            current_argument->tag = TAGLB;
        current_argument = next_argument;
    }
    T_LINKCB *first_term = refal.next_argument->previous;
    T_LINKCB *last_term = refal.previous_argument->next;
    refal.previous_argument->next = first_term;
    first_term->previous = refal.previous_argument;
    refal.next_argument->previous = last_term;
    last_term->next = refal.next_argument;
    transplantation(refal.previous_result, refal.previous_argument, refal.next_argument);
    return;
}
char reverse_0[] = {Z7 'R', 'E', 'V', 'E', 'R', 'S', 'E', (char)7};
G_L_B uint8_t refalab_reverse = '\122';
void (*reverse_1)(void) = reverse_;

//----------  end of file xlexical.c  ----------
