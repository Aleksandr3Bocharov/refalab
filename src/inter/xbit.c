// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-04-28
// https://github.com/Aleksandr3Bocharov/refalab

//---------------- file -- XBIT.C -----------
//                Bitwise MO:
//      band, bor, bxor, bnot, shl, shr
//-------------------------------------------

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "refalab.h"
#include "rfintf.h"

#define Oand 1
#define Oor 2
#define Oxor 3
#define Onot 4
#define Oshl 1
#define Oshr 2

static void bit_operate(uint8_t operation);
static void shift_operate(uint8_t operation);

static void band_(void) { bit_operate(Oand); }
char band_0[] = {Z4 'B', 'A', 'N', 'D', (char)4};
G_L_B uint8_t refalab_band = '\122';
void (*band_1)(void) = band_;

static void bor_(void) { bit_operate(Oor); }
char bor_0[] = {Z3 'B', 'O', 'R', (char)3};
G_L_B uint8_t refalab_bor = '\122';
void (*bor_1)(void) = bor_;

static void bxor_(void) { bit_operate(Oxor); }
char bxor_0[] = {Z4 'B', 'X', 'O', 'R', (char)4};
G_L_B uint8_t refalab_bxor = '\122';
void (*bxor_1)(void) = bxor_;

static void bnot_(void) { bit_operate(Onot); }
char bnot_0[] = {Z4 'B', 'N', 'O', 'T', (char)4};
G_L_B uint8_t refalab_bnot = '\122';
void (*bnot_1)(void) = bnot_;

static void shl_(void) { shift_operate(Oshl); }
char shl_0[] = {Z3 'S', 'H', 'L', (char)3};
G_L_B uint8_t refalab_shl = '\122';
void (*shl_1)(void) = shl_;

static void shr_(void) { shift_operate(Oshr); }
char shr_0[] = {Z3 'S', 'H', 'R', (char)3};
G_L_B uint8_t refalab_shr = '\122';
void (*shr_1)(void) = shr_;

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

static void exchange_numbers(void)
{
    T_LINKCB *temp_linkcb = X_begin;
    X_begin = Y_begin;
    Y_begin = temp_linkcb;
    temp_linkcb = X_end;
    X_end = Y_end;
    Y_end = temp_linkcb;
    const size_t temp_length = X_length;
    X_length = Y_length;
    Y_length = temp_length;
    const char temp_sign = X_sign;
    X_sign = Y_sign;
    Y_sign = temp_sign;
    return;
}

static void bit_operate(uint8_t operation)
{
    do
    {
        if (operation != Onot)
        {
            if (!read_numbers())
                break;
        }
        else
        {
            x_current = refal.previous_argument;
            y_current = refal.next_argument;
            if (read_number())
            {
                X_begin = begin;
                X_end = end;
                X_sign = sign;
                X_length = length;
            }
            else
                break;
        }
        bool result_zero = true;
        switch (operation)
        {
        case Oand:
            if (Y_length < X_length)
                exchange_numbers();
            if (X_length == 0)
                break;
            if (X_sign == '-' && Y_sign == '+')
                X_sign = '+';
            for (length = 0, y_current = Y_begin; length < Y_length - X_length; length++, y_current = y_current->next)
                ;
            for (x_current = X_begin; x_current != X_end->next; x_current = x_current->next, y_current = y_current->next)
            {
                pcoden(x_current, gcoden(x_current) & gcoden(y_current));
                if (result_zero && gcoden(x_current) != 0)
                    result_zero = false;
            }
            break;
        case Oor:
            if (Y_length > X_length)
                exchange_numbers();
            if (X_length == 0)
                break;
            result_zero = false;
            if (Y_length == 0)
                break;
            if (X_sign == '+' && Y_sign == '-')
                X_sign = '-';
            for (length = 0, x_current = X_begin; length < X_length - Y_length; length++, x_current = x_current->next)
                ;
            for (y_current = Y_begin; x_current != X_end->next; x_current = x_current->next, y_current = y_current->next)
                pcoden(x_current, gcoden(x_current) | gcoden(y_current));
            break;
        case Oxor:
            if (Y_length > X_length)
                exchange_numbers();
            if (X_length == 0)
                break;
            if (X_length > Y_length)
                result_zero = false;
            if (Y_length == 0)
                break;
            if (X_sign == '+' && Y_sign == '-')
                X_sign = '-';
            else if (X_sign == '-' && Y_sign == '-')
                X_sign = '+';
            for (length = 0, x_current = X_begin; length < X_length - Y_length; length++, x_current = x_current->next)
                ;
            for (y_current = Y_begin; x_current != X_end->next; x_current = x_current->next, y_current = y_current->next)
            {
                pcoden(x_current, gcoden(x_current) ^ gcoden(y_current));
                if (result_zero && gcoden(x_current) != 0)
                    result_zero = false;
            }
            break;
        case Onot:
            if (X_length == 0)
            {
                if (refal.previous_argument->next == refal.next_argument)
                    if (!extended_insert_from_free_memory_list(refal.previous_argument, 1))
                        return;
                result_zero = false;
                X_sign = '-';
                X_begin = refal.previous_argument->next;
                X_end = refal.previous_argument->next;
                X_begin->tag = TAGN;
                X_begin->info.code = NULL;
                pcoden(X_begin, MAX_NUMBER);
                break;
            }
            if (X_sign == '+')
                X_sign = '-';
            else
                X_sign = '+';
            for (x_current = X_begin; x_current != X_end->next; x_current = x_current->next)
            {
                pcoden(x_current, ~gcoden(x_current));
                if (result_zero && gcoden(x_current) != 0)
                    result_zero = false;
            }
        }
        if (result_zero)
        {
            x_current = refal.previous_argument->next;
            x_current->tag = TAGN;
            x_current->info.code = NULL;
            transplantation(refal.previous_result, x_current->previous, x_current->next);
            return;
        }
        //  wozwratim X
        // podawim wed. nuli
        for (x_current = X_begin; gcoden(x_current) == 0; x_current = x_current->next)
            ;
        if (X_sign == '-')
        {
            x_current = x_current->previous;
            x_current->tag = TAGO;
            x_current->info.code = NULL;
            x_current->info.infoc = '-';
        }
        //  perenosim reultat
        transplantation(refal.previous_result, x_current->previous, X_end->next);
        return;
    } while (false);
    refal.upshot = 2;
    return;
}

static void shift_operate(uint8_t operation)
{
    do
    {
        x_current = refal.previous_argument->next;
        if (x_current->tag != TAGLB)
            break;
        y_current = x_current->info.codep;
        if (read_number())
        {
            X_begin = begin;
            X_end = end;
            X_sign = sign;
            X_length = length;
        }
        else
            break;
        y_current = y_current->next;
        if (y_current->next != refal.next_argument || y_current->tag != TAGN)
            break;
        uint32_t shift_bits = gcoden(y_current);
        length = shift_bits / 32;
        shift_bits %= 32;
        const uint32_t transfer_shift_bits = 32 - shift_bits;
        bool result_zero = true;
        switch (operation)
        {
        case Oshl:
            if (X_length == 0)
                break;
            result_zero = false;
            const size_t result_need = length + (shift_bits == 0 ? 0 : 1) + (X_sign == '-' ? 1 : 0);
            size_t argument_length = 4;
            for (x_current = X_begin->previous; x_current->tag != TAGLB && result_need > argument_length; x_current = x_current->previous, argument_length++)
                ;
            if (result_need > argument_length)
                if (!extended_insert_from_free_memory_list(refal.next_result, result_need - argument_length))
                    return;
            transplantation(refal.next_result, X_end, refal.next_argument);
            if (length != 0)
            {
                y_current = X_begin;
                for (Y_length = 0; Y_length < length; Y_length++)
                {
                    X_begin = X_begin->previous;
                    X_begin->tag = TAGN;
                    X_begin->info.code = NULL;
                }
                for (x_current = X_begin; y_current != X_end->next; x_current = x_current->next, y_current = y_current->next)
                {
                    pcoden(x_current, gcoden(y_current));
                    y_current->info.code = NULL;
                }
                Y_end = x_current->previous;
            }
            else
                Y_end = X_end;
            if (shift_bits == 0)
                break;
            X_begin = X_begin->previous;
            X_begin->tag = TAGN;
            X_begin->info.code = NULL;
            for (x_current = X_begin->next; x_current != Y_end->next; x_current = x_current->next)
            {
                const uint32_t transfer = gcoden(x_current) >> transfer_shift_bits;
                pcoden(x_current->previous, gcoden(x_current->previous) | transfer);
                pcoden(x_current, gcoden(x_current) << shift_bits);
            }
            break;
        case Oshr:
            if (length >= X_length)
                break;
            if (length != 0)
            {
                for (x_current = X_end, Y_length = 0; Y_length < length; x_current = x_current->previous, Y_length++)
                    ;
                for (y_current = X_end; x_current != X_begin->previous; x_current = x_current->previous, y_current = y_current->previous)
                    pcoden(y_current, gcoden(x_current));
                X_begin = y_current->next;
            }
            if (shift_bits == 0)
            {
                result_zero = false;
                break;
            }
            for (x_current = X_end; x_current != X_begin; x_current = x_current->previous)
            {
                pcoden(x_current, gcoden(x_current) >> shift_bits);
                const uint32_t transfer = gcoden(x_current->previous) << transfer_shift_bits;
                pcoden(x_current, gcoden(x_current) | transfer);
            }
            pcoden(X_begin, gcoden(X_begin) >> shift_bits);
            if (X_begin != X_end || gcoden(X_begin) != 0)
                result_zero = false;
        }
        if (result_zero)
        {
            x_current = refal.previous_argument->next;
            x_current->tag = TAGN;
            x_current->info.code = NULL;
            transplantation(refal.previous_result, x_current->previous, x_current->next);
            return;
        }
        //  wozwratim X
        // podawim wed. nuli
        for (x_current = X_begin; gcoden(x_current) == 0; x_current = x_current->next)
            ;
        if (X_sign == '-')
        {
            x_current = x_current->previous;
            x_current->tag = TAGO;
            x_current->info.code = NULL;
            x_current->info.infoc = '-';
        }
        //  perenosim reultat
        transplantation(refal.previous_result, x_current->previous, X_end->next);
        return;
    } while (false);
    refal.upshot = 2;
    return;
}

//-------------------- end of file  XBIT.C ----------------
