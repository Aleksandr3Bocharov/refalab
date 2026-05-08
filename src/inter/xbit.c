// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-05-07
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

static void bit_operate(uint8_t operation)
{
    do
    {
        T_LINKCB *x_current = refal.previous_argument->next;
        const T_LINKCB *y_current = x_current->info.codep;
        T_BIG_NUMBER X, Y;
        if (operation != Onot)
        {
            if (x_current->tag != TAGLB || !read_big_numbers_expression(&X, &Y, x_current, y_current, refal.next_argument))
                break;
        }
        else if (!read_big_number_expression(&X, refal.previous_argument, refal.next_argument))
            break;
        bool result_zero = true;
        switch (operation)
        {
        case Oand:
            if (Y.length < X.length)
                exchange_big_numbers(&X, &Y);
            if (X.length == 0)
                break;
            if (X.sign == '-' && Y.sign == '+')
                X.sign = '+';
            size_t length;
            for (length = 0, y_current = Y.begin; length < Y.length - X.length; length++, y_current = y_current->next)
                ;
            for (x_current = X.begin; x_current != X.end->next; x_current = x_current->next, y_current = y_current->next)
            {
                pcoden(x_current, gcoden(x_current) & gcoden(y_current));
                if (result_zero && gcoden(x_current) != 0)
                    result_zero = false;
            }
            break;
        case Oor:
            if (Y.length > X.length)
                exchange_big_numbers(&X, &Y);
            if (X.length == 0)
                break;
            result_zero = false;
            if (Y.length == 0)
                break;
            if (X.sign == '+' && Y.sign == '-')
                X.sign = '-';
            for (length = 0, x_current = X.begin; length < X.length - Y.length; length++, x_current = x_current->next)
                ;
            for (y_current = Y.begin; x_current != X.end->next; x_current = x_current->next, y_current = y_current->next)
                pcoden(x_current, gcoden(x_current) | gcoden(y_current));
            break;
        case Oxor:
            if (Y.length > X.length)
                exchange_big_numbers(&X, &Y);
            if (X.length == 0)
                break;
            if (X.length > Y.length)
                result_zero = false;
            if (Y.length == 0)
                break;
            if (X.sign == '+' && Y.sign == '-')
                X.sign = '-';
            else if (X.sign == '-' && Y.sign == '-')
                X.sign = '+';
            for (length = 0, x_current = X.begin; length < X.length - Y.length; length++, x_current = x_current->next)
                ;
            for (y_current = Y.begin; x_current != X.end->next; x_current = x_current->next, y_current = y_current->next)
            {
                pcoden(x_current, gcoden(x_current) ^ gcoden(y_current));
                if (result_zero && gcoden(x_current) != 0)
                    result_zero = false;
            }
            break;
        case Onot:
            if (X.length == 0)
            {
                if (refal.previous_argument->next == refal.next_argument)
                    if (!extended_insert_from_free_memory(refal.previous_argument, 1))
                        return;
                result_zero = false;
                X.sign = '-';
                X.begin = refal.previous_argument->next;
                X.end = refal.previous_argument->next;
                X.begin->tag = TAGN;
                X.begin->info.code = NULL;
                pcoden(X.begin, MAX_NUMBER);
                break;
            }
            if (X.sign == '+')
                X.sign = '-';
            else
                X.sign = '+';
            for (x_current = X.begin; x_current != X.end->next; x_current = x_current->next)
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
        for (x_current = X.begin; gcoden(x_current) == 0; x_current = x_current->next)
            ;
        if (X.sign == '-')
        {
            x_current = x_current->previous;
            x_current->tag = TAGO;
            x_current->info.code = NULL;
            x_current->info.infoc = '-';
        }
        //  perenosim reultat
        transplantation(refal.previous_result, x_current->previous, X.end->next);
        return;
    } while (false);
    refal.upshot = 2;
    return;
}

static void shift_operate(uint8_t operation)
{
    do
    {
        T_LINKCB *x_current = refal.previous_argument->next;
        T_LINKCB *current = x_current->info.codep;
        T_BIG_NUMBER X;
        if (x_current->tag != TAGLB || !read_big_number_expression(&X, x_current, current))
            break;
        current = current->next;
        if (current->next != refal.next_argument || current->tag != TAGN)
            break;
        uint32_t shift_bits = gcoden(current);
        const size_t numbers_count = shift_bits / 32;
        shift_bits %= 32;
        const uint32_t transfer_shift_bits = 32 - shift_bits;
        bool result_zero = true;
        switch (operation)
        {
        case Oshl:
            if (X.length == 0)
                break;
            result_zero = false;
            const size_t result_need = numbers_count + (shift_bits == 0 ? 0 : 1) + (X.sign == '-' ? 1 : 0);
            size_t argument_length = 4;
            for (x_current = X.begin->previous; x_current->tag != TAGLB && result_need > argument_length; x_current = x_current->previous, argument_length++)
                ;
            if (result_need > argument_length)
                if (!extended_insert_from_free_memory(refal.next_result, result_need - argument_length))
                    return;
            transplantation(refal.next_result, X.end, refal.next_argument);
            size_t length;
            const T_LINKCB *end;
            if (numbers_count != 0)
            {
                current = X.begin;
                for (length = 0; length < numbers_count; length++)
                {
                    X.begin = X.begin->previous;
                    X.begin->tag = TAGN;
                    X.begin->info.code = NULL;
                }
                for (x_current = X.begin; current != X.end->next; x_current = x_current->next, current = current->next)
                {
                    pcoden(x_current, gcoden(current));
                    current->info.code = NULL;
                }
                end = x_current->previous;
            }
            else
                end = X.end;
            if (shift_bits == 0)
                break;
            X.begin = X.begin->previous;
            X.begin->tag = TAGN;
            X.begin->info.code = NULL;
            for (x_current = X.begin->next; x_current != end->next; x_current = x_current->next)
            {
                const uint32_t transfer = gcoden(x_current) >> transfer_shift_bits;
                pcoden(x_current->previous, gcoden(x_current->previous) | transfer);
                pcoden(x_current, gcoden(x_current) << shift_bits);
            }
            break;
        case Oshr:
            if (numbers_count >= X.length)
                break;
            if (numbers_count != 0)
            {
                for (x_current = X.end, length = 0; length < numbers_count; x_current = x_current->previous, length++)
                    ;
                for (current = X.end; x_current != X.begin->previous; x_current = x_current->previous, current = current->previous)
                    pcoden(current, gcoden(x_current));
                X.begin = current->next;
            }
            if (shift_bits == 0)
            {
                result_zero = false;
                break;
            }
            for (x_current = X.end; x_current != X.begin; x_current = x_current->previous)
            {
                pcoden(x_current, gcoden(x_current) >> shift_bits);
                const uint32_t transfer = gcoden(x_current->previous) << transfer_shift_bits;
                pcoden(x_current, gcoden(x_current) | transfer);
            }
            pcoden(X.begin, gcoden(X.begin) >> shift_bits);
            if (X.begin != X.end || gcoden(X.begin) != 0)
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
        for (x_current = X.begin; gcoden(x_current) == 0; x_current = x_current->next)
            ;
        if (X.sign == '-')
        {
            x_current = x_current->previous;
            x_current->tag = TAGO;
            x_current->info.code = NULL;
            x_current->info.infoc = '-';
        }
        //  perenosim reultat
        transplantation(refal.previous_result, x_current->previous, X.end->next);
        return;
    } while (false);
    refal.upshot = 2;
    return;
}

//-------------------- end of file  XBIT.C ----------------
