// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-05-14
// https://github.com/Aleksandr3Bocharov/refalab

//---------------- file -- XAR.C -----------
//              Ariphmetics MO:
//       add, sub, mul, dr, div,
//       addn, subn, muln, drn, divn,
//       gcd, ncheck, nnorm, abs, minus,
//       get_max_number, p1, m1
//------------------------------------------

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "refalab.h"
#include "rfintf.h"

#define Oadd 1
#define Osub 2
#define Omul 3
#define Odr 5

#define SHIFT_MAX 32

static void arithmetic_operate(uint8_t operation, uint8_t type);

static void add_(void) { arithmetic_operate(Oadd, 0); }
char add_0[] = {Z3 'A', 'D', 'D', (char)3};
G_L_B uint8_t refalab_add = '\122';
void (*add_1)(void) = add_;

static void sub_(void) { arithmetic_operate(Osub, 0); }
char sub_0[] = {Z3 'S', 'U', 'B', (char)3};
G_L_B uint8_t refalab_sub = '\122';
void (*sub_1)(void) = sub_;

static void mul_(void) { arithmetic_operate(Omul, 0); }
char mul_0[] = {Z3 'M', 'U', 'L', (char)3};
G_L_B uint8_t refalab_mul = '\122';
void (*mul_1)(void) = mul_;

static void dr_(void) { arithmetic_operate(Odr, 0); }
char dr_0[] = {Z2 'D', 'R', (char)2};
G_L_B uint8_t refalab_dr = '\122';
void (*dr_1)(void) = dr_;

static void div_(void) { arithmetic_operate(Odr, 2); }
char div_0[] = {Z3 'D', 'I', 'V', (char)3};
G_L_B uint8_t refalab_div = '\122';
void (*div_1)(void) = div_;

static void addn_(void) { arithmetic_operate(Oadd, 1); }
char addn_0[] = {Z4 'A', 'D', 'D', 'N', (char)4};
G_L_B uint8_t refalab_addn = '\122';
void (*addn_1)(void) = addn_;

static void subn_(void) { arithmetic_operate(Osub, 1); }
char subn_0[] = {Z4 'S', 'U', 'B', 'N', (char)4};
G_L_B uint8_t refalab_subn = '\122';
void (*subn_1)(void) = subn_;

static void muln_(void) { arithmetic_operate(Omul, 1); }
char muln_0[] = {Z4 'M', 'U', 'L', 'N', (char)4};
G_L_B uint8_t refalab_muln = '\122';
void (*muln_1)(void) = muln_;

static void drn_(void) { arithmetic_operate(Odr, 1); }
char drn_0[] = {Z3 'D', 'R', 'N', (char)3};
G_L_B uint8_t refalab_drn = '\122';
void (*drn_1)(void) = drn_;

static void divn_(void) { arithmetic_operate(Odr, 3); }
char divn_0[] = {Z4 'D', 'I', 'V', 'N', (char)4};
G_L_B uint8_t refalab_divn = '\122';
void (*divn_1)(void) = divn_;

static inline void multiply(uint64_t *a, uint64_t *b)
{ // rezult: a - starshy, b - mladshy
    uint64_t result = *a * *b;
    *b = result & MAX_NUMBER;
    *a = result >> SHIFT_MAX;
    return;
}

static void normalization(T_LINKCB *Number_end, size_t Number_length, uint8_t power) //  normaliz. posledov. makrocifr
{                                                                                    //  Number_end - ukaz. na konec
    uint32_t transfer = 0;
    const uint8_t transfer_shift = SHIFT_MAX - power;
    const uint32_t mask = MAX_NUMBER >> power; // maska
    T_LINKCB *number_current = Number_end;
    for (size_t i = 0; i < Number_length; i++)
    {
        const uint32_t number = gcoden(number_current);
        const uint32_t new_number = (number & mask) << power | transfer;
        pcoden(number_current, new_number);
        transfer = number >> transfer_shift;
        number_current = number_current->previous;
    }
    return;
}

static void arithmetic_operate(uint8_t operation, uint8_t type)
{
    T_LINKCB *x_current = refal.previous_argument->next;
    T_LINKCB *y_current = x_current->info.codep;
    T_BIG_NUMBER X, Y;
    if (x_current->tag != TAGLB || !read_big_numbers_expression(&X, &Y, x_current, y_current, refal.next_argument))
    {
        refal.upshot = 2;
        return;
    }
    uint32_t remainder, quotient;
    bool result_zero = false;
    bool dr_one_remainder_one_quotient = false;
    switch (operation)
    {
    case Osub: // izmenim znak i skladywaem
        if (Y.sign == '-')
            Y.sign = '+';
        else
            Y.sign = '-';
    case Oadd:
        if (X.length == 0 && Y.length == 0)
        {
            result_zero = true;
            break;
        }
        if (X.length == 0)
        { //  rezult - wtoroe chislo
            X.begin = Y.begin;
            X.end = Y.end;
            X.sign = Y.sign;
        }
        else if (Y.length != 0)
        { // summiruem
            if (X.sign == Y.sign)
            { //  skladywaem
                if (X.length < Y.length)
                    exchange_big_numbers(&X, &Y);
                //  X  dlinnee  Y  (ili =)
                X.begin = X.begin->previous; //  pripisywaem  0
                X.begin->tag = TAGN;
                X.begin->info.code = NULL;
                uint64_t transfer = 0;
                for (x_current = X.end, y_current = Y.end; x_current != X.begin->previous; x_current = x_current->previous)
                {
                    uint64_t sum;
                    if (y_current != Y.begin->previous)
                    {
                        sum = gcoden(x_current) + gcoden(y_current) + transfer;
                        y_current = y_current->previous;
                    }
                    else
                        sum = gcoden(x_current) + transfer;
                    transfer = sum / ((uint64_t)MAX_NUMBER + 1);
                    pcoden(x_current, (uint32_t)sum);
                } // for
            }
            else
            { // wychitaem
                const int8_t compare_absolute = compare_big_numbers_absolute(&X, &Y);
                if (compare_absolute == 0)
                {
                    result_zero = true;
                    break;
                }
                if (compare_absolute == -1)
                    exchange_big_numbers(&X, &Y); //  menjaem x i y
                int64_t transfer = 0;
                for (x_current = X.end, y_current = Y.end; x_current != X.begin->previous; x_current = x_current->previous)
                {
                    int64_t substraction = gcoden(x_current);
                    if (y_current != Y.begin->previous)
                    {
                        substraction -= (int64_t)gcoden(y_current) + transfer;
                        y_current = y_current->previous;
                    }
                    else
                        substraction -= transfer;
                    if (substraction < 0)
                    {
                        substraction += (int64_t)MAX_NUMBER + 1;
                        transfer = 1;
                    }
                    else
                        transfer = 0;
                    pcoden(x_current, (uint32_t)substraction);
                } // for
            } // if
        }
        break;
    case Omul:
        if (X.length == 0 || Y.length == 0)
        {
            result_zero = true;
            break;
        }
        T_LINKCB *result_begin = refal.previous_argument;
        T_LINKCB *result_end = result_begin->next;
        if (!extended_insert_from_free_memory(result_begin, X.length + Y.length + 1)) //  1 zweno dlja znaka
            return;
        result_begin = result_begin->next;
        result_end = result_end->previous;
        for (x_current = result_begin; x_current != result_end->next; x_current = x_current->next)
        {
            x_current->tag = TAGN;
            x_current->info.code = NULL;
        } //  zanulen rezultat
        if (X.length < Y.length)
            exchange_big_numbers(&X, &Y);
        //  dobawim 0 k X dlja summir. s perenosom
        X.begin = X.begin->previous;
        X.begin->tag = TAGN;
        X.begin->info.code = NULL;
        T_LINKCB *result_current;
        for (result_current = result_end, y_current = Y.end; y_current != Y.begin->previous; y_current = y_current->previous, result_current = result_current->previous)
        {
            const uint64_t b = gcoden(y_current);
            if (b != 0)
            { // umn. na 1 cifru
                uint64_t transfer = 0;
                for (x_current = X.end, result_begin = result_current; x_current != X.begin->previous; x_current = x_current->previous, result_begin = result_begin->previous)
                {
                    uint64_t a = gcoden(x_current);
                    uint64_t b_temp = b;
                    multiply(&a, &b_temp);
                    uint64_t sum = gcoden(result_begin) + b_temp + transfer;
                    transfer = sum / ((uint64_t)MAX_NUMBER + 1);
                    transfer += a;
                    pcoden(result_begin, (uint32_t)sum);
                } // for
            }
        }
        if (X.sign != Y.sign)
            X.sign = '-';
        else
            X.sign = '+';
        X.begin = result_begin;
        X.end = result_end;
        break;
    case Odr:
        if (Y.length == 0)
        {
            refal.upshot = 2;
            return;
        }
        if (X.length == 0)
        {
            remainder = 0;
            quotient = 0;
            X.sign = '+';
            Y.sign = '+';
            dr_one_remainder_one_quotient = true;
            break;
        }
        const int8_t compare_absolute = compare_big_numbers_absolute(&X, &Y);
        if (compare_absolute == 0)
        { //  rawny
            remainder = 0;
            quotient = 1;
            dr_one_remainder_one_quotient = true;
            break;
        }
        if (compare_absolute == -1)
        { //  delimoe < delitelja
            if ((type & 2) == 2)
            { // DIV, DIVN
                remainder = 0;
                quotient = 0;
                X.sign = '+';
                Y.sign = '+';
                dr_one_remainder_one_quotient = true;
                break;
            }
            if (X.sign == '-')
            {
                X.begin = X.begin->previous;
                X.begin->tag = TAGO;
                X.begin->info.code = NULL;
                X.begin->info.infoc = '-';
            }
            X.begin = X.begin->previous;
            X.end = X.end->next;
            X.begin->tag = TAGLB;
            X.end->tag = TAGRB;
            X.begin->info.codep = X.end;
            X.end->info.codep = X.begin;
            if (type == 0)
            {
                X.begin = X.begin->previous;
                X.begin->tag = TAGN;
                X.begin->info.code = NULL;
            }
            transplantation(refal.previous_result, X.begin->previous, X.end->next);
            return;
        }
        //   delimoe > delitelja
        if (X.length == 1)
        { //  oba po odnoj cifre
            remainder = gcoden(X.begin) % gcoden(Y.begin);
            quotient = gcoden(X.begin) / gcoden(Y.begin);
            dr_one_remainder_one_quotient = true;
            break;
        }
        //  delenie mnogih  cifr
        T_LINKCB *temp_linkcb = refal.previous_argument;
        if (!extended_insert_from_free_memory(temp_linkcb, X.length - Y.length + 2)) // t.k. k chastnomu dob. 0 i zweno na znak
            return;
        temp_linkcb = temp_linkcb->next;  //  dlja znaka
        result_begin = temp_linkcb->next; //  dlja  perwoj  cifry
        T_LINKCB *begin = result_begin;
        X.begin = X.begin->previous;
        X.begin->tag = TAGN;
        X.begin->info.code = NULL;
        X.length++;
        size_t i;
        for (i = 0, x_current = X.begin; i < Y.length; i++, x_current = x_current->next)
            ;
        y_current = Y.begin->previous;
        y_current->tag = TAGN;
        y_current->info.code = NULL;
        uint8_t power = 0;
        if (Y.length != 0)
        { // wozmovna normalizacija
            uint64_t exponentation = gcoden(Y.begin);
            for (power = 0; exponentation < 2147483648; power++, exponentation += exponentation)
                ;
            if (power != 0)
            {
                normalization(X.end, X.length, power);
                normalization(Y.end, Y.length, power);
            }
        }
        do
        {
            uint64_t a = gcoden(X.begin);
            const uint64_t a0 = gcoden(X.begin->next);
            uint64_t b = gcoden(Y.begin);
            uint64_t c;
            if (a == 0 && a0 < b)
                c = 0;
            else
            {
                if (a == 0 && a0 >= b)
                {
                    c = 1; //  t.k. b - normalizowano
                    a = a0;
                }
                else
                { // delim a,a0 na b
                    const uint64_t aa0 = a << 32 | a0;
                    c = aa0 / b;
                    a = aa0 % b;
                }
                const uint64_t b0 = gcoden(Y.begin->next);
                if (Y.length > 1 && b0 != 0)
                {
                    uint64_t x1 = b0;
                    uint64_t x2 = c;
                    multiply(&x1, &x2);
                    uint64_t y1 = a % b;
                    const uint64_t y2 = gcoden(X.begin->next->next);
                    bool state = false;
                    while (x1 > y1 || (x1 == y1 && x2 > y2))
                    {
                        c--;
                        state = true;
                        x1 = b0;
                        x2 = c;
                        multiply(&x1, &x2);
                        y1 += b;
                    }
                    if (state == true)
                        c++; // na wcjakij sluchaj
                }
            }
            // umnovenie  delitelja  na 'c' i wychit. iz X
            if (c != 0)
            {
                const T_LINKCB *Y_temp = Y.end;
                T_LINKCB *X_temp = x_current;
                uint64_t transfer = 0;
                for (; Y_temp != y_current->previous; X_temp = X_temp->previous, Y_temp = Y_temp->previous)
                {
                    b = gcoden(Y_temp);
                    a = c;
                    multiply(&a, &b);
                    b += transfer;
                    transfer = b >> SHIFT_MAX;
                    b &= MAX_NUMBER;
                    uint64_t x_temp = gcoden(X_temp);
                    if (x_temp < b)
                    {
                        x_temp += (uint64_t)MAX_NUMBER + 1;
                        transfer++;
                    }
                    pcoden(X_temp, (uint32_t)(x_temp - b));
                    transfer += a;
                }
                if (transfer != 0)
                    do
                    {
                        c--;
                        X_temp = x_current;
                        Y_temp = Y.end;
                        uint64_t new_transfer = 0;
                        for (; Y_temp != y_current->previous; X_temp = X_temp->previous, Y_temp = Y_temp->previous)
                        {
                            const uint64_t sum = gcoden(X_temp) + gcoden(Y_temp) + new_transfer;
                            new_transfer = sum / ((uint64_t)MAX_NUMBER + 1);
                            pcoden(X_temp, (uint32_t)sum);
                        }
                        transfer -= new_transfer;
                    } while (transfer != 0);
            }
            result_begin->tag = TAGN;
            result_begin->info.code = NULL;
            pcoden(result_begin, (uint32_t)c);
            result_begin = result_begin->next;
            x_current = x_current->next;
            X.begin = X.begin->next;
        } while (x_current != X.end->next);
        X.begin = X.begin->previous;
        result_begin = result_begin->previous;
        if (power != 0)
        { // denormalizacija ostatka
            uint32_t transfer = 0;
            const uint8_t transfer_shift = SHIFT_MAX - power;
            const uint32_t mask = MAX_NUMBER >> transfer_shift;
            for (x_current = X.begin; x_current != X.end->next; x_current = x_current->next)
            {
                const uint32_t number = gcoden(x_current);
                const uint32_t new_number = number >> power | transfer << transfer_shift;
                transfer = number & mask;
                pcoden(x_current, new_number);
            }
        }
        for (x_current = X.begin; x_current != X.end->next && gcoden(x_current) == 0; x_current = x_current->next)
            ;
        x_current = x_current->previous;
        if (x_current != X.end)
        {
            if (X.sign != Y.sign)
            {
                x_current->tag = TAGO;
                x_current->info.code = NULL;
                x_current->info.infoc = '-';
            }
            else
                x_current = x_current->next;
        }
        X.begin = x_current;
        for (x_current = begin; gcoden(x_current) == 0; x_current = x_current->next)
            ;
        if (X.sign == '-')
        {
            x_current = x_current->previous;
            x_current->tag = TAGO;
            x_current->info.code = NULL;
            x_current->info.infoc = '-';
        }
        if ((type & 1) == 0 || X.begin != X.end || gcoden(X.begin) != 0)
            X.begin = X.begin->previous;
        X.begin->tag = TAGLB;
        X.begin->info.codep = X.end->next;
        X.end = X.end->next;
        X.end->tag = TAGRB;
        X.end->info.codep = X.begin;
        if (result_begin->next != X.begin)
            transplantation(result_begin, X.begin->previous, X.end->next);
        if ((type & 2) == 0)
            transplantation(refal.previous_result, x_current->previous, X.end->next);
        else
            transplantation(refal.previous_result, x_current->previous, X.begin);
        return;
    } // end case
    if (result_zero)
    {
        if (type == 1)
            return; // dlja n-operacij
        x_current = refal.previous_argument->next;
        x_current->tag = TAGN;
        x_current->info.code = NULL;
        transplantation(refal.previous_result, x_current->previous, x_current->next);
        return;
    }
    if (!dr_one_remainder_one_quotient)
    {
        //  wozwratim X
        // podawim wed. nuli
        for (x_current = X.begin; gcoden(x_current) == 0; x_current = x_current->next)
            ;
        if (type == 1 && x_current == X.end && gcoden(x_current) == 0)
            return;
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
    }
    // wywod rezultata delenija, kogda ostatok i chastnoe
    // rawno po odnoj makrocifre remainder - ost., quotient - chastnoe
    if (!extended_insert_from_free_memory(refal.previous_argument, 2))
        return;
    // in bad case: /1/() - 3 zwena est uje + name
    x_current = refal.previous_argument;
    if (X.sign != Y.sign)
    {
        x_current->tag = TAGO;
        x_current->info.code = NULL;
        x_current->info.infoc = '-';
        x_current = x_current->next;
    }
    if (quotient != 0 || (type & 1) == 0)
    { // div/dr
        x_current->tag = TAGN;
        x_current->info.code = NULL;
        pcoden(x_current, quotient);
        x_current = x_current->next;
    }
    y_current = x_current->next;
    if (remainder != 0)
        if (X.sign != '+')
        {
            y_current->tag = TAGO;
            y_current->info.code = NULL;
            y_current->info.infoc = '-';
            y_current = y_current->next;
        }
    if (remainder != 0 || (type & 1) == 0)
    { // div/dr
        y_current->tag = TAGN;
        y_current->info.code = NULL;
        pcoden(y_current, remainder);
        y_current = y_current->next;
    }
    x_current->tag = TAGLB;
    y_current->tag = TAGRB;
    x_current->info.codep = y_current;
    y_current->info.codep = x_current;
    if ((type & 2) == 0)
        // dr/n
        transplantation(refal.previous_result, refal.previous_argument->previous, y_current->next);
    else
        // div/n
        transplantation(refal.previous_result, refal.previous_argument->previous, x_current);
    return;
}

static void shift_right(T_BIG_NUMBER *big_number, uint64_t shifts)
{
    const uint64_t numbers_count = shifts / 32;
    const uint64_t shifts_right = shifts % 32;
    uint64_t length;
    T_LINKCB *big_number_current;
    if (numbers_count != 0)
    {
        for (big_number_current = big_number->end, length = 0; length < numbers_count; big_number_current = big_number_current->previous, length++, big_number->length--)
            ;
        transplantation(big_number->begin->previous, big_number_current, big_number->end->next);
        big_number->end = big_number_current;
    }
    if (shifts_right != 0)
    {
        const uint64_t transfer_shift_bits = 32 - shifts_right;
        for (big_number_current = big_number->end; big_number_current != big_number->begin; big_number_current = big_number_current->previous)
            pcoden(big_number_current, gcoden(big_number_current) >> shifts_right | gcoden(big_number_current->previous) << transfer_shift_bits);
        pcoden(big_number->begin, gcoden(big_number->begin) >> shifts_right);
    }
    // podawim wed. nuli
    for (; gcoden(big_number->begin) == 0; big_number->begin = big_number->begin->next, big_number->length--)
        ;
    return;
}

static uint64_t count_trailing_zeros(const T_BIG_NUMBER *big_number)
{
    uint64_t trailing_zeros_count = 0;
    for (T_LINKCB *big_number_current = big_number->end;; big_number_current = big_number_current->previous)
    {
        uint32_t number = gcoden(big_number_current);
        if (number == 0)
        {
            trailing_zeros_count += 32;
            continue;
        }
        uint64_t count_number_trailing_zeros = 0;
        if ((number & 0xFFFF) == 0)
        {
            count_number_trailing_zeros += 16;
            number >>= 16;
        }
        if ((number & 0xFF) == 0)
        {
            count_number_trailing_zeros += 8;
            number >>= 8;
        }
        if ((number & 0xF) == 0)
        {
            count_number_trailing_zeros += 4;
            number >>= 4;
        }
        if ((number & 0x3) == 0)
        {
            count_number_trailing_zeros += 2;
            number >>= 2;
        }
        if ((number & 0x1) == 0)
            count_number_trailing_zeros += 1;
        trailing_zeros_count += count_number_trailing_zeros;
        return trailing_zeros_count;
    }
}

static void gcd_(void)
{
    T_LINKCB *x_current = refal.previous_argument->next;
    T_LINKCB *y_current = x_current->info.codep;
    T_BIG_NUMBER X, Y;
    if (x_current->tag != TAGLB || !read_big_numbers_expression(&X, &Y, x_current, y_current, refal.next_argument) || (X.length == 0 && Y.length == 0))
    {
        refal.upshot = 2;
        return;
    }
    uint64_t shifts_left = 0;
    enum
    {
        IS_FIN,
        COND,
        TWO_EVEN,
        ONE_ODD,
        TWO_ODD,
        FIN
    } gcd_state = IS_FIN;
    while (true)
        switch (gcd_state)
        {
        case IS_FIN:
            if (X.length == 0 || (Y.length == 1 && gcoden(Y.end) == 1))
            {
                exchange_big_numbers(&X, &Y);
                gcd_state = FIN;
            }
            else if (Y.length == 0 || (X.length == 1 && gcoden(X.end) == 1))
                gcd_state = FIN;
            else
                gcd_state = COND;
            break;
        case COND:
            if ((gcoden(X.end) & 1) == 0 && (gcoden(Y.end) & 1) == 0)
                gcd_state = TWO_EVEN;
            else if ((gcoden(X.end) & 1) == 0)
                gcd_state = ONE_ODD;
            else if ((gcoden(Y.end) & 1) == 0)
            {
                exchange_big_numbers(&X, &Y);
                gcd_state = ONE_ODD;
            }
            else
            {
                const int8_t compare_absolute = compare_big_numbers_absolute(&X, &Y);
                if (compare_absolute == 1)
                    gcd_state = TWO_ODD;
                else if (compare_absolute == -1)
                {
                    exchange_big_numbers(&X, &Y);
                    gcd_state = TWO_ODD;
                }
                else
                    gcd_state = FIN;
            }
            break;
        case TWO_EVEN:
        {
            const uint64_t X_count_trailing_zeros = count_trailing_zeros(&X);
            const uint64_t Y_count_trailing_zeros = count_trailing_zeros(&Y);
            const uint64_t min_count_trailing_zeros = X_count_trailing_zeros < Y_count_trailing_zeros ? X_count_trailing_zeros : Y_count_trailing_zeros;
            shift_right(&X, min_count_trailing_zeros);
            shift_right(&Y, min_count_trailing_zeros);
            shifts_left += min_count_trailing_zeros;
        }
            gcd_state = IS_FIN;
            break;
        case ONE_ODD:
        {
            const uint64_t X_count_trailing_zeros = count_trailing_zeros(&X);
            shift_right(&X, X_count_trailing_zeros);
        }
            gcd_state = IS_FIN;
            break;
        case TWO_ODD:
        {
            int64_t transfer = 0; // wychitaem
            for (x_current = X.end, y_current = Y.end; x_current != X.begin->previous; x_current = x_current->previous)
            {
                int64_t substraction = gcoden(x_current);
                if (y_current != Y.begin->previous)
                {
                    substraction -= (int64_t)gcoden(y_current) + transfer;
                    y_current = y_current->previous;
                }
                else
                    substraction -= transfer;
                if (substraction < 0)
                {
                    substraction += (int64_t)MAX_NUMBER + 1;
                    transfer = 1;
                }
                else
                    transfer = 0;
                pcoden(x_current, (uint32_t)substraction);
            } // for
        }
            // podawim wed. nuli
            for (; gcoden(X.begin) == 0; X.begin = X.begin->next, X.length--)
                ;
            const uint64_t X_count_trailing_zeros = count_trailing_zeros(&X);
            shift_right(&X, X_count_trailing_zeros);
            gcd_state = IS_FIN;
            break;
        case FIN:
            if (shifts_left != 0)
            {
                const uint64_t numbers_count = shifts_left / 32;
                shifts_left %= 32;
                uint64_t length;
                if (numbers_count != 0)
                {
                    x_current = X.begin;
                    for (length = 0; length < numbers_count; length++)
                    {
                        x_current = x_current->previous;
                        x_current->info.code = NULL;
                    }
                    T_LINKCB *new_end = X.begin->previous;
                    transplantation(X.end, x_current->previous, X.begin);
                    X.end = new_end;
                }
                if (shifts_left != 0)
                {
                    X.begin = X.begin->previous;
                    X.begin->tag = TAGN;
                    X.begin->info.code = NULL;
                    const uint64_t transfer_shift_bits = 32 - shifts_left;
                    for (x_current = X.begin->next; x_current != X.end->next; x_current = x_current->next)
                    {
                        pcoden(x_current->previous, gcoden(x_current->previous) | gcoden(x_current) >> transfer_shift_bits);
                        pcoden(x_current, gcoden(x_current) << shifts_left);
                    }
                    if (gcoden(X.begin) == 0)
                        X.begin = X.begin->next;
                }
            }
            transplantation(refal.previous_result, X.begin->previous, X.end->next);
            return;
        }
    return;
}
char gcd_0[] = {Z3 'G', 'C', 'D', (char)3};
G_L_B uint8_t refalab_gcd = '\122';
void (*gcd_1)(void) = gcd_;

static void ncheck_(void)
{
    T_BIG_NUMBER X;
    if (!read_big_number_expression(&X, refal.previous_argument, refal.next_argument))
    {
        refal.upshot = 2;
        return;
    }
    transplantation(refal.previous_result, refal.previous_argument, refal.next_argument);
    return;
}
char ncheck_0[] = {Z6 'N', 'C', 'H', 'E', 'C', 'K', (char)6};
G_L_B uint8_t refalab_ncheck = '\122';
void (*ncheck_1)(void) = ncheck_;

static void nnorm_(void)
{
    T_BIG_NUMBER X;
    if (!read_big_number_expression(&X, refal.previous_argument, refal.next_argument))
    {
        refal.upshot = 2;
        return;
    }
    if (X.length == 0)
    {
        refal.previous_argument->tag = TAGN;
        refal.previous_argument->info.code = NULL;
        pcoden(refal.previous_argument, 0);
        transplantation(refal.previous_result, refal.previous_argument->previous, refal.previous_argument->next);
    }
    else if (X.sign == '-')
    {
        T_LINKCB *X_sign = X.begin->previous;
        X_sign->tag = TAGO;
        X_sign->info.code = NULL;
        X_sign->info.infoc = '-';
        transplantation(refal.previous_result, X_sign->previous, X.end->next);
    }
    else
        transplantation(refal.previous_result, X.begin->previous, X.end->next);
    return;
}
char nnorm_0[] = {Z5 'N', 'N', 'O', 'R', 'M', (char)5};
G_L_B uint8_t refalab_nnorm = '\122';
void (*nnorm_1)(void) = nnorm_;

static void abs_(void)
{
    T_BIG_NUMBER X;
    if (!read_big_number_expression(&X, refal.previous_argument, refal.next_argument))
    {
        refal.upshot = 2;
        return;
    }
    if (X.length == 0)
    {
        refal.previous_argument->tag = TAGN;
        refal.previous_argument->info.code = NULL;
        pcoden(refal.previous_argument, 0);
        transplantation(refal.previous_result, refal.previous_argument->previous, refal.previous_argument->next);
    }
    else
        transplantation(refal.previous_result, X.begin->previous, X.end->next);
    return;
}
char abs_0[] = {Z3 'A', 'B', 'S', (char)3};
G_L_B uint8_t refalab_abs = '\122';
void (*abs_1)(void) = abs_;

static void minus_(void)
{
    T_BIG_NUMBER X;
    if (!read_big_number_expression(&X, refal.previous_argument, refal.next_argument))
    {
        refal.upshot = 2;
        return;
    }
    if (X.length == 0)
    {
        refal.previous_argument->tag = TAGN;
        refal.previous_argument->info.code = NULL;
        pcoden(refal.previous_argument, 0);
        transplantation(refal.previous_result, refal.previous_argument->previous, refal.previous_argument->next);
    }
    else if (X.sign == '+')
    {
        T_LINKCB *X_sign = X.begin->previous;
        X_sign->tag = TAGO;
        X_sign->info.code = NULL;
        X_sign->info.infoc = '-';
        transplantation(refal.previous_result, X_sign->previous, X.end->next);
    }
    else
        transplantation(refal.previous_result, X.begin->previous, X.end->next);
    return;
}
char minus_0[] = {Z5 'M', 'I', 'N', 'U', 'S', (char)5};
G_L_B uint8_t refalab_minus = '\122';
void (*minus_1)(void) = minus_;

static void get_max_number_(void)
{
    if (refal.previous_argument->next != refal.next_argument)
    {
        refal.upshot = 2;
        return;
    }
    refal.previous_argument->tag = TAGN;
    refal.previous_argument->info.code = NULL;
    pcoden(refal.previous_argument, MAX_NUMBER);
    transplantation(refal.previous_result, refal.previous_argument->previous, refal.previous_argument->next);
    return;
}
char get_max_number_0[] = {Z6 'G', 'E', 'T', '_', 'M', 'A', 'X', '_', 'N', 'U', 'M', 'B', 'E', 'R', (char)14};
G_L_B uint8_t refalab_get_max_number = '\122';
void (*get_max_number_1)(void) = get_max_number_;

static void p1_(void)
{
    do
    {
        T_LINKCB *number = refal.previous_argument->next;
        if (number->next != refal.next_argument || number->tag != TAGN)
            break;
        uint32_t next_number = gcoden(number) + 1;
        if (next_number == 0)
            break;
        pcoden(number, next_number);
        transplantation(refal.previous_result, number->previous, number->next);
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char p1_0[] = {Z2 'P', '1', (char)2};
G_L_B uint8_t refalab_p1 = '\122';
void (*p1_1)(void) = p1_;

static void m1_(void)
{
    do
    {
        T_LINKCB *number = refal.previous_argument->next;
        if (number->next != refal.next_argument || number->tag != TAGN)
            break;
        uint32_t previous_number = gcoden(number) - 1;
        if (previous_number == MAX_NUMBER)
            break;
        pcoden(number, previous_number);
        transplantation(refal.previous_result, number->previous, number->next);
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char m1_0[] = {Z2 'M', '1', (char)2};
G_L_B uint8_t refalab_m1 = '\122';
void (*m1_1)(void) = m1_;

//-------------------- end of file  XAR.C ----------------
