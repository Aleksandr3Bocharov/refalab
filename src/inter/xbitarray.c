// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-08-10
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file xbitarray.c  ----------
//                Bitwise MO:
//      Bit, Bitand, Bitor, Bitxor,
//      Bitnot, Bitsh, Bitrot
//----------------------------------------

#include <stddef.h>
#include <stdint.h>
#include "refalab.h"
#include "interface.h"

#define Oand 1
#define Oor 2
#define Oxor 3
#define Onot 4
#define Osh 1
#define Orot 2

static void bit_operate(uint8_t operation);
static void bit_shift_operate(uint8_t operation);

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

static void bitsh_(void) { bit_shift_operate(Osh); }
char bitsh_0[] = {Z5 'B', 'I', 'T', 'S', 'H', (char)5};
G_L_B uint8_t refalab_bitsh = '\122';
void (*bitsh_1)(void) = bitsh_;

static void bitrot_(void) { bit_shift_operate(Orot); }
char bitrot_0[] = {Z6 'B', 'I', 'T', 'R', 'O', 'T', (char)6};
G_L_B uint8_t refalab_bitrot = '\122';
void (*bitrot_1)(void) = bitrot_;

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

static void bit_shift_operate(uint8_t operation)
{
    do
    {
        T_LINKCB *current_argument = refal.previous_argument->next;
        if (current_argument->tag != TAGLB)
            break;
        T_LINKCB *right_bracket = current_argument->info.codep;
        T_LINKCB *begin_array = current_argument->next;
        T_LINKCB *end_array = right_bracket;
        uint64_t array_length = 0;
        for (current_argument = begin_array; current_argument != end_array; current_argument = current_argument->next)
        {
            if (current_argument->tag != TAGN)
                break;
            if (array_length == UINT64_MAX >> 5)
                break;
            array_length++;
        }
        if (current_argument != end_array || array_length == 0)
            break;
        const uint64_t array_bits = array_length << 5;
        current_argument = right_bracket->next;
        T_BIG_NUMBER shift_number;
        if (!read_big_number_expression(&shift_number, current_argument, refal.next_argument))
            break;
        if (shift_number.length == 0)
        {
            transplantation(refal.previous_result, begin_array->previous, end_array);
            return;
        }
        const bool shift_right = shift_number.sign == '-';
        uint64_t shift;
        if (shift_number.length <= 2)
        {
            shift = 0;
            for (current_argument = shift_number.begin; current_argument != shift_number.end->next; current_argument = current_argument->next)
                shift = shift << 32 | gcoden(current_argument);
            if (operation == Orot)
                shift %= array_bits;
            else if (shift >= array_bits)
            {
                for (current_argument = begin_array; current_argument != end_array; current_argument = current_argument->next)
                    pcoden(current_argument, 0);
                transplantation(refal.previous_result, begin_array->previous, end_array);
                return;
            }
        }
        else
        {
            if (operation == Osh)
            {
                for (current_argument = begin_array; current_argument != end_array; current_argument = current_argument->next)
                    pcoden(current_argument, 0);
                transplantation(refal.previous_result, begin_array->previous, end_array);
                return;
            }
            /*
             * Bitrot
             *
             * shift_number % (array_length * 32)
             *
             * remainder = 32 * q + low
             *
             *     q   < array_length
             *     low < 32.
             *
             * d = 32 * k + new_low
             *
             *     new_q =
             *       q * 2^32
             *       + low * 2^27
             *       + k
             *       (mod array_length).
             */
            const uint64_t modulus = array_length;
            uint64_t q = 0;
            uint32_t low = 0;
            for (current_argument = shift_number.begin; current_argument != shift_number.end->next; current_argument = current_argument->next)
            {
                const uint32_t digit = gcoden(current_argument);
                const uint64_t k = digit >> 5;
                const uint32_t new_low = digit & 31U;
                for (unsigned int i = 0; i < 32; i++)
                {
                    if (q >= modulus - q)
                        q -= modulus - q;
                    else
                        q += q;
                }
                uint64_t extra = ((uint64_t)low << 27) + k;
                extra %= modulus;
                if (q >= modulus - extra)
                    q -= modulus - extra;
                else
                    q += extra;
                low = new_low;
            }
            shift = q << 5 | low;
        }
        if (shift == 0)
        {
            transplantation(refal.previous_result, begin_array->previous, end_array);
            return;
        }
        uint64_t number_shift = shift >> 5;
        uint32_t bit_shift = (uint32_t)(shift & 31);
        /*
         * =====================================================
         * Bitsh
         * =====================================================
         */
        if (operation == Osh)
        {
            if (!shift_right)
            {
                /*
                 * A B C D E
                 *     2
                 *
                 * C D E A B
                 *
                 * C D E 0 0
                 */
                if (number_shift != 0)
                {
                    T_LINKCB *middle_array = begin_array;
                    for (uint64_t i = 0; i < number_shift; i++)
                        middle_array = middle_array->next;
                    transplantation(begin_array->previous, middle_array->previous, end_array);
                    current_argument = end_array->previous;
                    for (uint64_t i = 0; i < number_shift; i++)
                    {
                        pcoden(current_argument, 0);
                        current_argument = current_argument->previous;
                    }
                }
                if (bit_shift != 0)
                {
                    const uint32_t reverse_shift = 32 - bit_shift;
                    uint32_t carry = 0;
                    for (current_argument = begin_array; current_argument != end_array; current_argument = current_argument->next)
                    {
                        const uint32_t value = gcoden(current_argument);
                        pcoden(current_argument, value << bit_shift | carry);
                        carry = value >> reverse_shift;
                    }
                }
            }
            else
            {
                /*
                 * A B C D E
                 *     2
                 *
                 * D E A B C
                 *
                 * 0 0 A B C
                 */
                if (number_shift != 0)
                {
                    T_LINKCB *middle_array = begin_array;
                    for (uint64_t i = 0; i < array_length - number_shift; i++)
                        middle_array = middle_array->next;
                    transplantation(begin_array->previous, middle_array->previous, end_array);
                    current_argument = begin_array;
                    for (uint64_t i = 0; i < number_shift; i++)
                    {
                        pcoden(current_argument, 0);
                        current_argument = current_argument->next;
                    }
                }
                if (bit_shift != 0)
                {
                    const uint32_t reverse_shift = 32 - bit_shift;
                    uint32_t carry = 0;
                    for (current_argument = end_array->previous; current_argument != begin_array->previous; current_argument = current_argument->previous)
                    {
                        const uint32_t value = gcoden(current_argument);
                        pcoden(current_argument, value >> bit_shift | carry);
                        carry = value << reverse_shift;
                    }
                }
            }
            transplantation(refal.previous_result, begin_array->previous, end_array);
            return;
        }
        /*
         * =====================================================
         * Bitrot
         * =====================================================
         */
        if (shift_right)
            shift = array_bits - shift;
        number_shift = shift >> 5;
        bit_shift = (uint32_t)(shift & 31);
        /*
         * A B | C D E
         *
         * C D E | A B
         */
        if (number_shift != 0)
        {
            T_LINKCB *middle_array = begin_array;
            for (uint64_t i = 0; i < number_shift; i++)
                middle_array = middle_array->next;
            transplantation(begin_array->previous, middle_array->previous, end_array);
        }
        if (bit_shift != 0)
        {
            const uint32_t reverse_shift = 32 - bit_shift;
            const uint32_t carry = gcoden(begin_array) >> reverse_shift;
            for (current_argument = begin_array; current_argument != end_array; current_argument = current_argument->next)
            {
                const uint32_t value = gcoden(current_argument);
                const uint32_t next = current_argument->next == end_array ? carry : gcoden(current_argument->next);
                pcoden(current_argument, value << bit_shift | next >> reverse_shift);
            }
        }
        transplantation(refal.previous_result, begin_array->previous, end_array);
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
