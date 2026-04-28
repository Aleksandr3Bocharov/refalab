// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-04-25
// https://github.com/Aleksandr3Bocharov/refalab

//---------------- file -- XAR.C -----------
//              Ariphmetics MO:
//       add, sub, mul, dr, div,
//       addn, subn, muln, drn, divn,
//       gcd, p1, m1
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

static void operate(uint32_t operation, uint32_t type);

static void add_(void) { operate(Oadd, 0); }
char add_0[] = {Z3 'A', 'D', 'D', (char)3};
G_L_B uint8_t refalab_add = '\122';
void (*add_1)(void) = add_;

static void sub_(void) { operate(Osub, 0); }
char sub_0[] = {Z3 'S', 'U', 'B', (char)3};
G_L_B uint8_t refalab_sub = '\122';
void (*sub_1)(void) = sub_;

static void mul_(void) { operate(Omul, 0); }
char mul_0[] = {Z3 'M', 'U', 'L', (char)3};
G_L_B uint8_t refalab_mul = '\122';
void (*mul_1)(void) = mul_;

static void dr_(void) { operate(Odr, 0); }
char dr_0[] = {Z2 'D', 'R', (char)2};
G_L_B uint8_t refalab_dr = '\122';
void (*dr_1)(void) = dr_;

static void div_(void) { operate(Odr, 2); }
char div_0[] = {Z3 'D', 'I', 'V', (char)3};
G_L_B uint8_t refalab_div = '\122';
void (*div_1)(void) = div_;

static void addn_(void) { operate(Oadd, 1); }
char addn_0[] = {Z4 'A', 'D', 'D', 'N', (char)4};
G_L_B uint8_t refalab_addn = '\122';
void (*addn_1)(void) = addn_;

static void subn_(void) { operate(Osub, 1); }
char subn_0[] = {Z4 'S', 'U', 'B', 'N', (char)4};
G_L_B uint8_t refalab_subn = '\122';
void (*subn_1)(void) = subn_;

static void muln_(void) { operate(Omul, 1); }
char muln_0[] = {Z4 'M', 'U', 'L', 'N', (char)4};
G_L_B uint8_t refalab_muln = '\122';
void (*muln_1)(void) = muln_;

static void drn_(void) { operate(Odr, 1); }
char drn_0[] = {Z3 'D', 'R', 'N', (char)3};
G_L_B uint8_t refalab_drn = '\122';
void (*drn_1)(void) = drn_;

static void divn_(void) { operate(Odr, 3); }
char divn_0[] = {Z4 'D', 'I', 'V', 'N', (char)4};
G_L_B uint8_t refalab_divn = '\122';
void (*divn_1)(void) = divn_;

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

static void multiply(int64_t *a, int64_t *b)
{ // rezult: a - starshy, b - mladshy
    if (*a == 0)
    {
        *b = 0;
        return;
    }
    if (*b == 0)
    {
        *a = 0;
        return;
    }
    const int64_t a1 = *a >> 16;
    const int64_t b1 = *b >> 16;
    const int64_t a0 = *a & 0xFFFF;
    const int64_t b0 = *b & 0xFFFF;
    const int64_t a0b0 = a0 * b0;
    *b = a0b0 & 0xFFFF;
    int64_t r3 = a0b0 >> 16;
    const int64_t a1b0 = a1 * b0;
    r3 += a1b0 & 0xFFFF;
    int64_t r2 = a1b0 >> 16;
    const int64_t a0b1 = a0 * b1;
    r3 += a0b1 & 0xFFFF;
    r2 += a0b1 >> 16;
    const int64_t a1b1 = a1 * b1;
    r2 += a1b1 & 0xFFFF;
    const int64_t r1 = a1b1 >> 16;
    *a = (r1 << 16) + r2 + (r3 >> 16);
    *b += (r3 & 0xFFFF) << 16;
    return;
}

static void normalization(T_LINKCB *Number_end, size_t Number_length, uint8_t power) //  normaliz. posledov. makrocifr
{                                                                                    //  Number_end - ukaz. na konec
    int64_t transfer = 0;
    const uint8_t transfer_shift = SHIFT_MAX - power;
    const int64_t mask = MAX_NUMBER >> power; // maska
    T_LINKCB *number_current = Number_end;
    for (size_t i = 0; i < Number_length; i++)
    {
        const int64_t number = gcoden(number_current);
        const int64_t new_number = (number & mask) << power | transfer;
        pcoden(number_current, (uint32_t)new_number);
        transfer = number >> transfer_shift;
        number_current = number_current->previous;
    }
    return;
}

static void operate(uint32_t operation, uint32_t type)
{
    if (!read_numbers())
    {
        refal.upshot = 2;
        return;
    }
    int64_t remainder, quotient;
    bool rezult_zero = false;
    bool dr_one_remainder_one_quotient = false;
    switch (operation)
    {
    case Osub: // izmenim znak i skladywaem
        if (Y_sign == '-')
            Y_sign = '+';
        else
            Y_sign = '-';
    case Oadd:
        if (X_length == 0 && Y_length == 0)
        {
            rezult_zero = true;
            break;
        }
        if (X_length == 0)
        { //  rezult - wtoroe chislo
            X_begin = Y_begin;
            X_end = Y_end;
            X_sign = Y_sign;
        }
        else if (Y_length != 0)
        { // summiruem
            if (X_sign == Y_sign)
            { //  skladywaem
                if (X_length < Y_length)
                    exchange_numbers();
                //  X  dlinnee  Y  (ili =)
                X_begin = X_begin->previous; //  pripisywaem  0
                X_begin->tag = TAGN;
                X_begin->info.code = NULL;
                int64_t transfer = 0, sum;
                for (x_current = X_end, y_current = Y_end; x_current != X_begin->previous; x_current = x_current->previous)
                {
                    if (y_current != Y_begin->previous)
                    {
                        sum = (int64_t)gcoden(x_current) + gcoden(y_current) + transfer;
                        y_current = y_current->previous;
                    }
                    else
                        sum = (int64_t)gcoden(x_current) + transfer;
                    if (sum >= MAX_NUMBER + 1)
                    {
                        sum -= MAX_NUMBER + 1;
                        transfer = 1;
                    }
                    else
                        transfer = 0;
                    pcoden(x_current, (uint32_t)sum);
                } // for
            }
            else
            { // wychitaem
                if (compare_numbers() == 2)
                {
                    rezult_zero = true;
                    break;
                }
                if (compare_numbers() == 1)
                    exchange_numbers();      //  menjaem x i y
                X_begin = X_begin->previous; //  pripisywaem 0
                X_begin->tag = TAGN;
                X_begin->info.code = NULL;
                int64_t transfer = 0, substraction;
                for (x_current = X_end, y_current = Y_end; x_current != X_begin->previous; x_current = x_current->previous)
                {
                    substraction = gcoden(x_current);
                    if (y_current != Y_begin->previous)
                    {
                        substraction -= (int64_t)gcoden(y_current) + transfer;
                        y_current = y_current->previous;
                    }
                    else
                        substraction -= transfer;
                    if (substraction < 0)
                    {
                        substraction += MAX_NUMBER + 1;
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
        if (X_length == 0 || Y_length == 0)
        {
            rezult_zero = true;
            break;
        }
        T_LINKCB *result_begin = refal.previous_argument;
        T_LINKCB *result_end = result_begin->next;
        if (!extended_insert_from_free_memory_list(result_begin, X_length + Y_length + 1)) //  1 zweno dlja znaka
            return;
        result_begin = result_begin->next;
        result_end = result_end->previous;
        for (x_current = result_begin; x_current != result_end->next; x_current = x_current->next)
        {
            x_current->tag = TAGN;
            x_current->info.code = NULL;
        } //  zanulen rezultat
        if (X_length < Y_length)
            exchange_numbers();
        //  dobawim 0 k X dlja summir. s perenosom
        X_begin = X_begin->previous;
        X_begin->tag = TAGN;
        X_begin->info.code = NULL;
        T_LINKCB *result_current;
        for (result_current = result_end, y_current = Y_end; y_current != Y_begin->previous; y_current = y_current->previous, result_current = result_current->previous)
        {
            int64_t b = gcoden(y_current);
            if (b != 0)
            { // umn. na 1 cifru
                int64_t transfer = 0;
                const int64_t b11 = b >> 16;
                const int64_t b00 = b & 0xFFFF;
                for (x_current = X_end, result_begin = result_current; x_current != X_begin->previous; x_current = x_current->previous, result_begin = result_begin->previous)
                {
                    int64_t a = gcoden(x_current);
                    if (a == 0)
                        b = 0;
                    else
                    {
                        const int64_t a11 = a >> 16;
                        const int64_t a00 = a & 0xFFFF;
                        const int64_t a0b0 = a00 * b00;
                        b = a0b0 & 0xFFFF;
                        int64_t r3 = a0b0 >> 16;
                        const int64_t a1b0 = a11 * b00;
                        r3 += a1b0 & 0xFFFF;
                        int64_t r2 = a1b0 >> 16;
                        const int64_t a0b1 = a00 * b11;
                        r3 += a0b1 & 0xFFFF;
                        r2 += a0b1 >> 16;
                        const int64_t a1b1 = a11 * b11;
                        r2 += a1b1 & 0xFFFF;
                        const int64_t r1 = a1b1 >> 16;
                        a = (r1 << 16) + r2 + (r3 >> 16);
                        b += (r3 & 0xFFFF) << 16;
                    }
                    int64_t sum = (int64_t)gcoden(result_begin) + b + transfer;
                    transfer = 0;
                    if (sum >= MAX_NUMBER + 1)
                    {
                        sum -= MAX_NUMBER + 1;
                        transfer++;
                    }
                    transfer += a;
                    pcoden(result_begin, (uint32_t)sum);
                } // for
            }
        }
        if (X_sign != Y_sign)
            X_sign = '-';
        else
            X_sign = '+';
        X_begin = result_begin;
        X_end = result_end;
        break;
    case Odr:
        if (Y_length == 0)
        {
            refal.upshot = 2;
            return;
        }
        if (X_length == 0)
        {
            remainder = 0;
            quotient = 0;
            X_sign = '+';
            Y_sign = '+';
            dr_one_remainder_one_quotient = true;
            break;
        }
        if (compare_numbers() == 2)
        { //  rawny
            remainder = 0;
            quotient = 1;
            dr_one_remainder_one_quotient = true;
            break;
        }
        if (compare_numbers() == 1)
        { //  delimoe < delitelja
            if ((type & 2) == 2)
            { // DIV, DIVN
                remainder = 0;
                quotient = 0;
                X_sign = '+';
                Y_sign = '+';
                dr_one_remainder_one_quotient = true;
                break;
            }
            if (X_sign == '-')
            {
                X_begin = X_begin->previous;
                X_begin->tag = TAGO;
                X_begin->info.code = NULL;
                X_begin->info.infoc = '-';
            }
            X_begin = X_begin->previous;
            X_end = X_end->next;
            X_begin->tag = TAGLB;
            X_end->tag = TAGRB;
            X_begin->info.codep = X_end;
            X_end->info.codep = X_begin;
            if (type == 0)
            {
                X_begin = X_begin->previous;
                X_begin->tag = TAGN;
                X_begin->info.code = NULL;
            }
            transplantation(refal.previous_result, X_begin->previous, X_end->next);
            return;
        }
        //   delimoe > delitelja
        if (X_length == 1)
        { //  oba po odnoj cifre
            remainder = gcoden(X_begin) % gcoden(Y_begin);
            quotient = gcoden(X_begin) / gcoden(Y_begin);
            dr_one_remainder_one_quotient = true;
            break;
        }
        //  delenie mnogih  cifr
        T_LINKCB *temp_linkcb = refal.previous_argument;
        if (!extended_insert_from_free_memory_list(temp_linkcb, X_length - Y_length + 2)) // t.k. k chastnomu dob. 0 i zweno na znak
            return;
        temp_linkcb = temp_linkcb->next;  //  dlja znaka
        result_begin = temp_linkcb->next; //  dlja  perwoj  cifry
        begin = result_begin;
        X_begin = X_begin->previous;
        X_begin->tag = TAGN;
        X_begin->info.code = NULL;
        X_length++;
        size_t i;
        for (i = 0, x_current = X_begin; i < Y_length; i++, x_current = x_current->next)
            ;
        y_current = Y_begin->previous;
        y_current->tag = TAGN;
        y_current->info.code = NULL;
        uint8_t power = 0;
        if (Y_length != 0)
        { // wozmovna normalizacija
            int64_t exponentation = gcoden(Y_begin);
            for (power = 0; exponentation < 2147483648; power++, exponentation += exponentation)
                ;
            if (power != 0)
            {
                normalization(X_end, X_length, power);
                normalization(Y_end, Y_length, power);
            }
        }
        do
        {
            int64_t a = gcoden(X_begin);
            const int64_t a0 = gcoden(X_begin->next);
            int64_t b = gcoden(Y_begin);
            int64_t c;
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
                    a = (a << 7) + (a0 >> 25);
                    c = a / b << 25;
                    a = (a % b << 7) + (a0 >> 18 & 0x7F);
                    c += a / b << 18;
                    a = (a % b << 7) + (a0 >> 11 & 0x7F);
                    c += a / b << 11;
                    a = (a % b << 7) + (a0 >> 4 & 0x7F);
                    c += a / b << 4;
                    a = (a % b << 4) + (a0 & 0xF);
                    c += a / b;
                }
                const int64_t b0 = gcoden(Y_begin->next);
                if (Y_length > 1 && b0 != 0)
                {
                    int64_t x1 = b0;
                    int64_t x2 = c;
                    multiply(&x1, &x2);
                    int64_t y1 = a % b;
                    const int64_t y2 = gcoden(X_begin->next->next);
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
                const T_LINKCB *Y_temp = Y_end;
                T_LINKCB *X_temp = x_current;
                int64_t transfer = 0;
                for (; Y_temp != y_current->previous; X_temp = X_temp->previous, Y_temp = Y_temp->previous)
                {
                    b = gcoden(Y_temp);
                    a = c;
                    multiply(&a, &b);
                    b += transfer;
                    transfer = b >> SHIFT_MAX;
                    b &= MAX_NUMBER;
                    int64_t x_temp = gcoden(X_temp);
                    if (x_temp < b)
                    {
                        x_temp += MAX_NUMBER + 1;
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
                        Y_temp = Y_end;
                        int64_t new_transfer = 0;
                        for (; Y_temp != y_current->previous; X_temp = X_temp->previous, Y_temp = Y_temp->previous)
                        {
                            int64_t sum = (int64_t)gcoden(X_temp) + gcoden(Y_temp) + new_transfer;
                            new_transfer = 0;
                            if (sum >= MAX_NUMBER + 1)
                            {
                                sum -= MAX_NUMBER + 1;
                                new_transfer = 1;
                            }
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
            X_begin = X_begin->next;
        } while (x_current != X_end->next);
        X_begin = X_begin->previous;
        result_begin = result_begin->previous;
        if (power != 0)
        { // denormalizacija ostatka
            int64_t transfer = 0;
            const uint8_t transfer_shift = SHIFT_MAX - power;
            const int64_t mask = MAX_NUMBER >> transfer_shift;
            for (x_current = X_begin; x_current != X_end->next; x_current = x_current->next)
            {
                const int64_t number = gcoden(x_current);
                const int64_t new_number = number >> power | transfer << transfer_shift;
                transfer = number & mask;
                pcoden(x_current, (uint32_t)new_number);
            }
        }
        for (x_current = X_begin; x_current != X_end->next && gcoden(x_current) == 0; x_current = x_current->next)
            ;
        x_current = x_current->previous;
        if (x_current != X_end)
        {
            if (X_sign != Y_sign)
            {
                x_current->tag = TAGO;
                x_current->info.code = NULL;
                x_current->info.infoc = '-';
            }
            else
                x_current = x_current->next;
        }
        X_begin = x_current;
        for (x_current = begin; gcoden(x_current) == 0; x_current = x_current->next)
            ;
        if (X_sign == '-')
        {
            x_current = x_current->previous;
            x_current->tag = TAGO;
            x_current->info.code = NULL;
            x_current->info.infoc = '-';
        }
        if ((type & 1) == 0 || X_begin != X_end || gcoden(X_begin) != 0)
            X_begin = X_begin->previous;
        X_begin->tag = TAGLB;
        X_begin->info.codep = X_end->next;
        X_end = X_end->next;
        X_end->tag = TAGRB;
        X_end->info.codep = X_begin;
        if (result_begin->next != X_begin)
            transplantation(result_begin, X_begin->previous, X_end->next);
        if ((type & 2) == 0)
            transplantation(refal.previous_result, x_current->previous, X_end->next);
        else
            transplantation(refal.previous_result, x_current->previous, X_begin);
        return;
    } // end case
    if (rezult_zero)
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
        for (x_current = X_begin; gcoden(x_current) == 0; x_current = x_current->next)
            ;
        if (type == 1 && x_current == X_end && gcoden(x_current) == 0)
            return;
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
    }
    // wywod rezultata delenija, kogda ostatok i chastnoe
    // rawno po odnoj makrocifre remainder - ost., quotient - chastnoe
    if (!extended_insert_from_free_memory_list(refal.previous_argument, 2))
        return;
    // in bad case: /1/() - 3 zwena est uje + name
    x_current = refal.previous_argument;
    if (X_sign != Y_sign)
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
        pcoden(x_current, (uint32_t)quotient);
        x_current = x_current->next;
    }
    y_current = x_current->next;
    if (remainder != 0)
        if (X_sign != '+')
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
        pcoden(y_current, (uint32_t)remainder);
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

static void gcd_(void)
{
    //   sint. control
    T_LINKCB *temp_linkcb = refal.previous_argument->next;
    T_LINKCB *number_tail[2], *number_current[2], *number_head[2];
    size_t number_length[2];
    enum
    {
        OC,
        M12,
        M21,
        SHD,
        FIN1,
        NEOT
    } gcd_state = OC;
    if (temp_linkcb->tag != TAGLB)
        gcd_state = NEOT;
    else
    {
        number_tail[0] = temp_linkcb->info.codep;
        number_tail[1] = refal.next_argument;
        number_current[0] = temp_linkcb;
        number_current[1] = number_tail[0];
        for (uint8_t i = 0; i < 2; i++)
        {
            number_current[i] = number_current[i]->next;
            if (number_current[i]->tag == TAGO && (number_current[i]->info.infoc == '+' || number_current[i]->info.infoc == '-'))
                number_current[i] = number_current[i]->next;
            number_head[i] = number_current[i];
            number_length[i] = 0;
            while (number_current[i] != number_tail[i])
            {
                if (number_current[i]->tag != TAGN)
                {
                    gcd_state = NEOT;
                    break;
                }
                number_length[i]++;
                number_current[i] = number_current[i]->next;
            }
            if (gcd_state == NEOT)
                break;
            number_tail[i] = number_current[i]->previous;
        }
    }
    while (true)
        switch (gcd_state)
        {
        case OC:
            //*******   ob. cikl  ***********
            //   unicht. lewyh nulej
            if (number_length[0] != 0)
                while (gcoden(number_head[0]) == 0 && number_length[0] > 0)
                {
                    number_head[0] = number_head[0]->next;
                    number_length[0]--;
                }
            if (number_length[1] != 0)
                while (gcoden(number_head[1]) == 0 && number_length[1] > 0)
                {
                    number_head[1] = number_head[1]->next;
                    number_length[1]--;
                }
            uint8_t number = 0;
            if (number_length[0] == 0)
            {
                number = 1;
                gcd_state = FIN1;
                break;
            }
            if (number_length[1] == 0)
            {
                number = 0;
                gcd_state = FIN1;
                break;
            }
            //   delaem 1 > 2
            if (number_length[0] == number_length[1])
            {
                number_current[0] = number_head[0];
                number_current[1] = number_head[1];
                for (size_t i = 0; i < number_length[0]; i++)
                {
                    const int64_t number1 = gcoden(number_current[0]);
                    const int64_t number2 = gcoden(number_current[1]);
                    if (number1 < number2)
                    {
                        gcd_state = M12;
                        break;
                    }
                    if (number1 > number2)
                    {
                        gcd_state = M21;
                        break;
                    }
                    number_current[0] = number_current[0]->next;
                    number_current[1] = number_current[1]->next;
                }
                if (gcd_state != OC)
                    break;
                number = 0;
                gcd_state = FIN1;
                break;
            }
            if (number_length[0] < number_length[1])
            {
                gcd_state = M12;
                break;
            }
            gcd_state = M21;
            break;
        case M12:
            temp_linkcb = number_head[0];
            number_head[0] = number_head[1];
            number_head[1] = temp_linkcb;
            temp_linkcb = number_tail[0];
            number_tail[0] = number_tail[1];
            number_tail[1] = temp_linkcb;
            const size_t temp_length = number_length[0];
            number_length[0] = number_length[1];
            number_length[1] = temp_length;
            gcd_state = M21;
            break;
        case M21:;
            //   wybor metoda
            int64_t A = 0;
            number_current[0] = number_head[0];
            size_t k;
            for (k = 0; k < number_length[0]; k++)
            {
                if (A >= 2147483648)
                    break;
                A <<= SHIFT_MAX;
                A += gcoden(number_current[0]);
                number_current[0] = number_current[0]->next;
            }
            if (number_length[0] == 1 || (number_length[0] == 2 && k == 2))
            {
                // Evklid nad korotkimi
                // UTV: number_length[0] >= number_length[1]
                int64_t B = 0;
                number_current[1] = number_head[1];
                for (k = 0; k < number_length[1]; k++)
                {
                    B <<= SHIFT_MAX;
                    B += gcoden(number_current[1]);
                    number_current[1] = number_current[1]->next;
                }
                while (B != 0)
                {
                    const int64_t q = A / B;
                    const int64_t r = A - q * B;
                    A = B;
                    B = r;
                }
                // UTV: rezult v A
                temp_linkcb = refal.previous_argument->next;
                const int64_t a1 = A >> SHIFT_MAX;
                if (a1 != 0)
                {
                    temp_linkcb->tag = TAGN;
                    temp_linkcb->info.code = NULL;
                    pcoden(temp_linkcb, (uint32_t)a1);
                    temp_linkcb = temp_linkcb->next;
                    A &= MAX_NUMBER;
                }
                temp_linkcb->tag = TAGN;
                temp_linkcb->info.code = NULL;
                pcoden(temp_linkcb, (uint32_t)A);
                temp_linkcb = temp_linkcb->next;
                transplantation(refal.previous_result, refal.previous_argument, temp_linkcb);
                return;
            }
            //    A - pribligenie
            //    k={ 1/2 }
            const int64_t length_difference = (int64_t)number_length[1] - (int64_t)number_length[0] + (int64_t)k;
            int64_t xi[2], yi[2];
            if (length_difference <= 0)
            {
                gcd_state = SHD;
                break;
            }
            // UTV:  number_length[1] > hvosta,
            // UTV:  number_length[0] = {1/2}
            int64_t B = 0;
            number_current[1] = number_head[1];
            for (k = 0; k < (size_t)length_difference; k++)
            {
                B <<= SHIFT_MAX;
                B += gcoden(number_current[1]);
                number_current[1] = number_current[1]->next;
            }
            if (A / (B + 1) != (A + 1) / B)
            {
                gcd_state = SHD;
                break;
            }
            //  metod Lemera
            //  A i B s nedostatkom
            int64_t AL = A;
            int64_t AH = A + 1;
            int64_t BL = B;
            int64_t BH = B + 1;
            xi[0] = 1;
            xi[1] = 0;
            yi[0] = 0;
            yi[1] = 1;
            //  vychisl koeff. X i Y
            while (BL != 0)
            {
                const int64_t Q = AL / BH;
                //  UTV:   Q>0
                if (Q != AH / BL)
                    break;
                const int64_t RL = AL - Q * BH;
                const int64_t RH = AH - Q * BL;
                //  UTV:   RL>=0
                //  UTV:   RH>0
                AL = BL;
                AH = BH;
                BL = RL;
                BH = RH;
                const int64_t xn = xi[0] - Q * xi[1];
                const int64_t yn = yi[0] - Q * yi[1];
                xi[0] = xi[1];
                yi[0] = yi[1];
                xi[1] = xn;
                yi[1] = yn;
            }
            //   vyravnivanie dlin
            if (number_length[0] != number_length[1])
            {
                number_head[1] = number_head[1]->previous;
                number_head[1]->tag = TAGN;
                number_head[1]->info.code = NULL;
                number_length[1]++;
            }
            number_current[0] = number_tail[0];
            number_current[1] = number_tail[1];
            int64_t r[] = {0, 0};
            for (k = 0; k < number_length[0]; k++)
            {
                const int64_t current_number[] = {gcoden(number_current[0]), gcoden(number_current[1])};
                for (uint8_t i = 0; i < 2; i++)
                {
                    int64_t vs1 = current_number[0];
                    int64_t vs2 = current_number[1];
                    int64_t vs3, vs4;
                    if (xi[i] < 0)
                    {
                        vs3 = -xi[i];
                        vs4 = yi[i];
                    }
                    else
                    {
                        vs3 = xi[i];
                        vs4 = -yi[i];
                    }
                    multiply(&vs1, &vs3);
                    multiply(&vs2, &vs4);
                    if (xi[i] < 0)
                    {
                        vs1 = -vs1;
                        vs3 = -vs3;
                    }
                    else
                    {
                        vs2 = -vs2;
                        vs4 = -vs4;
                    }
                    int64_t r0 = r[i] + vs3 + vs4;
                    if (r0 < 0)
                    {
                        vs3 = r0 / (MAX_NUMBER + 1);
                        r0 %= MAX_NUMBER + 1;
                    }
                    else
                    {
                        vs3 = r0 >> SHIFT_MAX;
                        r0 &= MAX_NUMBER;
                    }
                    r[i] = vs1 + vs2 + vs3;
                    if (r0 < 0)
                    {
                        r[i]--;
                        r0 += MAX_NUMBER + 1;
                    }
                    pcoden(number_current[i], (uint32_t)r0);
                    number_current[i] = number_current[i]->previous;
                }
            }
            //   UTV: r[0] i r[1] ===0
            gcd_state = OC;
            break;
            //  shag delenija (normal)
            //  A nabrano s nedostatkom
            //  number_length[0] > number_length[1]   number_length[1] >0
            //  B nabiraem s izbytkom
        case SHD:
            //  delenie mnogih  cifr
            number_head[0] = number_head[0]->previous;
            number_head[0]->tag = TAGN;
            number_head[0]->info.code = NULL;
            number_length[0]++;
            size_t i;
            for (i = 0, number_current[0] = number_head[0]; i < number_length[1]; i++, number_current[0] = number_current[0]->next)
                ;
            number_current[1] = number_head[1]->previous;
            number_current[1]->tag = TAGN;
            number_current[1]->info.code = NULL;
            uint8_t power = 0;
            if (number_length[1] != 0)
            { // wozmovna normalizacija
                int64_t exponentation = gcoden(number_head[1]);
                for (power = 0; exponentation < 2147483648; power++, exponentation += exponentation)
                    ;
                if (power != 0)
                {
                    normalization(number_tail[0], number_length[0], power);
                    normalization(number_tail[1], number_length[1], power);
                }
            }
            do
            {
                int64_t a = gcoden(number_head[0]);
                const int64_t a0 = gcoden(number_head[0]->next);
                int64_t b = gcoden(number_head[1]);
                int64_t c;
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
                        a = (a << 7) + (a0 >> 25);
                        c = a / b << 25;
                        a = (a % b << 7) + (a0 >> 18 & 0x7F);
                        c += a / b << 18;
                        a = (a % b << 7) + (a0 >> 11 & 0x7F);
                        c += a / b << 11;
                        a = (a % b << 7) + (a0 >> 4 & 0x7F);
                        c += a / b << 4;
                        a = (a % b << 4) + (a0 & 0xF);
                        c += a / b;
                    }
                    const int64_t b0 = gcoden(number_head[1]->next);
                    if (number_length[1] > 1 && b0 != 0)
                    {
                        xi[0] = b0;
                        xi[1] = c;
                        multiply(&xi[0], &xi[1]);
                        yi[0] = a % b;
                        yi[1] = gcoden(number_head[0]->next->next);
                        bool state = false;
                        while (xi[0] > yi[0] || (xi[0] == yi[0] && xi[1] > yi[1]))
                        {
                            c--;
                            state = true;
                            xi[0] = b0;
                            xi[1] = c;
                            multiply(&xi[0], &xi[1]);
                            yi[0] += b;
                        }
                        if (state == true)
                            c++; // na wcjakij sluchaj
                    }
                }
                // umnovenie  delitelja  na 'c' i wychit. iz X
                if (c != 0)
                {
                    T_LINKCB *number_temp[2];
                    number_temp[1] = number_tail[1];
                    number_temp[0] = number_current[0];
                    int64_t transfer = 0;
                    for (; number_temp[1] != number_current[1]->previous; number_temp[0] = number_temp[0]->previous, number_temp[1] = number_temp[1]->previous)
                    {
                        b = gcoden(number_temp[1]);
                        a = c;
                        multiply(&a, &b);
                        b += transfer;
                        transfer = b >> SHIFT_MAX;
                        b &= MAX_NUMBER;
                        int64_t number0_temp = gcoden(number_temp[0]);
                        if (number0_temp < b)
                        {
                            number0_temp += MAX_NUMBER + 1;
                            transfer++;
                        }
                        pcoden(number_temp[0], (uint32_t)(number0_temp - b));
                        transfer += a;
                    }
                    if (transfer != 0)
                    { // cifra welika
                        do
                        {
                            c--;
                            number_temp[0] = number_current[0];
                            number_temp[1] = number_tail[1];
                            int64_t new_transfer = 0;
                            for (; number_temp[1] != number_current[1]->previous; number_temp[0] = number_temp[0]->previous, number_temp[1] = number_temp[1]->previous)
                            {
                                int64_t sum = (int64_t)gcoden(number_temp[0]) + gcoden(number_temp[1]) + new_transfer;
                                new_transfer = 0;
                                if (sum >= MAX_NUMBER + 1)
                                {
                                    sum -= MAX_NUMBER + 1;
                                    new_transfer = 1;
                                }
                                pcoden(number_temp[0], (uint32_t)sum);
                            }
                            transfer -= new_transfer;
                        } while (transfer != 0);
                    }
                }
                number_current[0] = number_current[0]->next;
                number_head[0] = number_head[0]->next;
                number_length[0]--;
            } while (number_current[0] != number_tail[0]->next);
            number_head[0] = number_head[0]->previous;
            number_length[0]++;
            if (power != 0)
            {
                int64_t transfer = 0;
                const uint8_t transfer_shift = SHIFT_MAX - power;
                const int64_t mask = MAX_NUMBER >> transfer_shift;
                // denormalizacija ostatka
                for (number_current[0] = number_head[0]; number_current[0] != number_tail[0]->next; number_current[0] = number_current[0]->next)
                {
                    const int64_t current_number0 = gcoden(number_current[0]);
                    const int64_t new_current_number0 = current_number0 >> power | transfer << transfer_shift;
                    transfer = current_number0 & mask;
                    pcoden(number_current[0], (uint32_t)new_current_number0);
                }
                // denormalizacija delitelja
                transfer = 0;
                for (number_current[1] = number_head[1]; number_current[1] != number_tail[1]->next; number_current[1] = number_current[1]->next)
                {
                    const int64_t current_number1 = gcoden(number_current[1]);
                    const int64_t new_current_number1 = current_number1 >> power | transfer << transfer_shift;
                    transfer = current_number1 & mask;
                    pcoden(number_current[1], (uint32_t)new_current_number1);
                }
            }
            gcd_state = OC;
            break;
        case FIN1:
            // number: odno iz chisel
            if (number_length[number] == 0)
            {
                gcd_state = NEOT;
                break;
            }
            transplantation(refal.previous_result, number_head[number]->previous, number_tail[number]->next);
            return;
        case NEOT:
            refal.upshot = 2;
            return;
        }
}
char gcd_0[] = {Z3 'G', 'C', 'D', (char)3};
G_L_B uint8_t refalab_gcd = '\122';
void (*gcd_1)(void) = gcd_;

static void p1_(void)
{
    do
    {
        T_LINKCB *p = refal.previous_argument->next;
        if (p->next != refal.next_argument || p->tag != TAGN)
            break;
        uint32_t l = gcoden(p) + 1;
        if (l == 0)
            break;
        pcoden(p, l);
        transplantation(refal.previous_result, refal.previous_argument, refal.next_argument);
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
        T_LINKCB *p = refal.previous_argument->next;
        if (p->next != refal.next_argument || p->tag != TAGN)
            break;
        uint32_t l = gcoden(p) - 1;
        if (l == MAX_NUMBER)
            break;
        pcoden(p, l);
        transplantation(refal.previous_result, refal.previous_argument, refal.next_argument);
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char m1_0[] = {Z2 'M', '1', (char)2};
G_L_B uint8_t refalab_m1 = '\122';
void (*m1_1)(void) = m1_;

//-------------------- end of file  XAR.C ----------------
