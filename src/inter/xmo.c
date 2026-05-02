// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-05-01
// https://github.com/Aleksandr3Bocharov/refalab

//-------------- file -- XMO.C -------------
//                 Lexical MO:
//     numb, symb, first, last,
//     lengr, lengw, multe,
//     chr, ord, upper, lower
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
    const char sign = current_argument->info.infoc;
    T_LINKCB *sign_argument = current_argument;
    if (current_argument->tag == TAGO && (sign == '-' || sign == '+'))
    {
        current_argument = current_argument->next;
        if (sign == '+')
            sign_argument = current_argument;
    }
    T_LINKCB *begin_number_argument = current_argument;
    while (current_argument->tag == TAGN && gcoden(current_argument) == 0)
        current_argument = current_argument->next;
    uint8_t i;
    for (i = 0; current_argument != refal.next_argument; i++, current_argument = current_argument->next)
        if (current_argument->tag != TAGN || i == 1)
        {
            refal.upshot = 2;
            return;
        }
    current_argument = current_argument->previous;
    uint32_t number = gcoden(current_argument);
    if (i == 0 || number == 0)
    {
        sign_argument = begin_number_argument;
        number = 0;
    }
    char number_string[11];
    sprintf(number_string, "%u", number);
    const uint8_t number_length = (uint8_t)strlen(number_string);
    if (!check_count_free_memory(number_length))
        if (!more_free_memory())
        {
            refal.upshot = 3;
            return;
        }
    if (sign_argument != refal.next_argument)
        insert_from_free_memory(begin_number_argument, number_length);
    else
    {
        sign_argument = sign_argument->previous;
        insert_from_free_memory(sign_argument, number_length);
        sign_argument = sign_argument->next;
        begin_number_argument = sign_argument;
    }
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
    T_LINKCB *pn = refal.previous_argument->next;
    if (pn == refal.next_argument || pn->tag != TAGN)
    {
        refal.upshot = 2;
        return;
    }; // FAIL
    const uint32_t n = gcoden(pn);
    T_LINKCB *p = pn;
    for (size_t k = 1; k <= n; k++)
    {
        p = p->next;
        if (p == refal.next_argument)
        {
            pn->info.code = NULL;
            pn->info.infoc = '*';
            pn->tag = TAGO;
            transplantation(refal.previous_result, refal.previous_argument, refal.next_argument);
            return;
        }
        if (p->tag == TAGLB)
            p = p->info.codep;
    }
    p = p->next;
    refal.previous_argument->tag = TAGLB;
    refal.previous_argument->info.codep = pn;
    pn->tag = TAGRB;
    pn->info.codep = refal.previous_argument;
    transplantation(refal.previous_argument, pn, p);
    transplantation(refal.previous_result, refal.next_result, refal.next_argument);
    return;
}
char first_0[] = {Z5 'F', 'I', 'R', 'S', 'T', (char)5};
G_L_B uint8_t refalab_first = '\122';
void (*first_1)(void) = first_;

static void last_(void)
{
    T_LINKCB *pn = refal.previous_argument->next;
    if (pn == refal.next_argument || pn->tag != TAGN)
    {
        refal.upshot = 2;
        return;
    }; // FAIL
    const uint32_t n = gcoden(pn);
    T_LINKCB *p = refal.next_argument;
    for (size_t k = 1; k <= n; k++)
    {
        p = p->previous;
        if (p == pn)
        {
            pn->tag = TAGO;
            pn->info.code = NULL;
            pn->info.infoc = '*';
            transplantation(refal.previous_result, pn, refal.next_argument);
            p = refal.next_result->previous;
            transplantation(p, refal.previous_argument, refal.next_argument);
            return;
        }
        if (p->tag == TAGRB)
            p = p->info.codep;
    }
    p = p->previous;
    refal.previous_argument->tag = TAGLB;
    refal.previous_argument->info.codep = pn;
    pn->tag = TAGRB;
    pn->info.codep = refal.previous_argument;
    transplantation(refal.previous_argument, p, refal.next_argument);
    transplantation(refal.previous_result, pn, refal.next_argument);
    p = refal.next_result->previous;
    transplantation(p, refal.next_result, refal.next_argument);
    return;
}
char last_0[] = {Z4 'L', 'A', 'S', 'T', (char)4};
G_L_B uint8_t refalab_last = '\122';
void (*last_1)(void) = last_;

static void lengr_(void)
{
    uint32_t n = 0;
    const T_LINKCB *p = refal.previous_argument->next;
    while (p != refal.next_argument)
    {
        n++;
        p = p->next;
    }
    refal.previous_argument->tag = TAGN;
    refal.previous_argument->info.code = NULL;
    pcoden(refal.previous_argument, n);
    transplantation(refal.previous_result, refal.next_result, refal.next_argument);
    return;
}
char lengr_0[] = {Z5 'L', 'E', 'N', 'G', 'R', (char)5};
G_L_B uint8_t refalab_lengr = '\122';
void (*lengr_1)(void) = lengr_;

static void lengw_(void)
{
    uint32_t n = 0;
    const T_LINKCB *p = refal.previous_argument->next;
    while (p != refal.next_argument)
    {
        n++;
        if (p->tag == TAGLB)
            p = p->info.codep;
        p = p->next;
    }
    refal.previous_argument->tag = TAGN;
    refal.previous_argument->info.code = NULL;
    pcoden(refal.previous_argument, n);
    transplantation(refal.previous_result, refal.next_result, refal.next_argument);
    return;
}
char lengw_0[] = {Z5 'L', 'E', 'N', 'G', 'W', (char)5};
G_L_B uint8_t refalab_lengw = '\122';
void (*lengw_1)(void) = lengw_;

static void multe_(void)
{
    T_LINKCB *pn = refal.previous_argument->next;
    if (pn == refal.next_argument || pn->tag != TAGN)
    {
        refal.upshot = 2;
        return;
    }; // FAIL
    uint32_t n = gcoden(pn);
    if (n == 0)
        return;
    T_LINKCB *p = pn->next;
    if (p == refal.next_argument)
        return;
    if (p->next != refal.next_argument)
    {
        do
        {
            p = refal.next_result->previous;
            if (!copy_expression(p, pn, refal.next_argument))
            {
                refal.upshot = 3;
                return;
            }; // LACK
            n--;
        } while (n >= 1);
    }
    else
    {
        if (!extended_insert_from_free_memory(refal.previous_result, n))
            return; //  LACK
        T_LINKCB *q = refal.previous_result;
        for (uint32_t k = 0; k < n; k++)
        {
            q = q->next;
            q->tag = p->tag;
            q->info.code = p->info.code;
        }
    }
    return;
}
char multe_0[] = {Z5 'M', 'U', 'L', 'T', 'E', (char)5};
G_L_B uint8_t refalab_multe = '\122';
void (*multe_1)(void) = multe_;

static void chr_(void)
{
    T_LINKCB *p = refal.previous_argument->next;
    while (p != refal.next_argument)
    {
        if (p->tag == TAGN)
        {
            p->tag = TAGO;
            const char c = (char)gcoden(p);
            p->info.code = NULL;
            p->info.infoc = c;
        }
        p = p->next;
    }
    transplantation(refal.previous_result, refal.previous_argument, refal.next_argument);
    return;
}
char chr_0[] = {Z3 'C', 'H', 'R', (char)3};
G_L_B uint8_t refalab_chr = '\122';
void (*chr_1)(void) = chr_;

static void ord_(void)
{
    T_LINKCB *p = refal.previous_argument->next;
    while (p != refal.next_argument)
    {
        if (p->tag == TAGO)
        {
            p->tag = TAGN;
            pcoden(p, (uint8_t)p->info.infoc);
        }
        p = p->next;
    }
    transplantation(refal.previous_result, refal.previous_argument, refal.next_argument);
    return;
}
char ord_0[] = {Z3 'O', 'R', 'D', (char)3};
G_L_B uint8_t refalab_ord = '\122';
void (*ord_1)(void) = ord_;

static void upper_(void)
{
    T_LINKCB *p = refal.previous_argument->next;
    while (p != refal.next_argument)
    {
        if (p->tag == TAGO)
            p->info.infoc = (char)toupper(p->info.infoc);
        p = p->next;
    }
    transplantation(refal.previous_result, refal.previous_argument, refal.next_argument);
    return;
}
char upper_0[] = {Z5 'U', 'P', 'P', 'E', 'R', (char)5};
G_L_B uint8_t refalab_upper = '\122';
void (*upper_1)(void) = upper_;

static void lower_(void)
{
    T_LINKCB *p = refal.previous_argument->next;
    while (p != refal.next_argument)
    {
        if (p->tag == TAGO)
            p->info.infoc = (char)tolower(p->info.infoc);
        p = p->next;
    }
    transplantation(refal.previous_result, refal.previous_argument, refal.next_argument);
    return;
}
char lower_0[] = {Z5 'L', 'O', 'W', 'E', 'R', (char)5};
G_L_B uint8_t refalab_lower = '\122';
void (*lower_1)(void) = lower_;

//-------------------- end of file  XMO.C ----------------
