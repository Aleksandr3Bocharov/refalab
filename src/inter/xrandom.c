// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-06-05
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file xrandom.c  ---------
//        MO: random,
//            random_number
//-------------------------------------

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "refalab.h"
#include "interface.h"

#define MIN_DELAY 33
#define MAX_DELAY 97

static bool randomizer_init = false;
static uint8_t n_sub_max_delay = MAX_DELAY - 1;
static uint8_t n_sub_min_delay = MIN_DELAY - 1;
static uint32_t x[MAX_DELAY];

static uint32_t random_number(void)
{
    if (!randomizer_init)
    {
        srand((unsigned int)time(NULL));
        for (uint8_t i = 0; i < MAX_DELAY; i++)
            x[i] = (uint32_t)((double)rand() / RAND_MAX * MAX_NUMBER);
        randomizer_init = true;
    }
    x[n_sub_max_delay] += x[n_sub_min_delay];
    const uint32_t number = x[n_sub_max_delay];
    n_sub_max_delay = (uint8_t)((n_sub_max_delay + MAX_DELAY - 1) % MAX_DELAY);
    n_sub_min_delay = (uint8_t)((n_sub_min_delay + MAX_DELAY - 1) % MAX_DELAY);
    return number;
}

static uint32_t random_number_in_range(uint32_t limit)
{
    if (limit == 0)
        return 0;
    const uint32_t max_number_valid = MAX_NUMBER - MAX_NUMBER % limit;
    uint32_t number_random;
    do
    {
        number_random = random_number();
    } while (number_random >= max_number_valid);
    return number_random % limit;
}

static void random_(void)
{
    do
    {
        const T_LINKCB *symbol_number = refal.previous_argument->next;
        if (symbol_number->next != refal.next_argument || symbol_number->tag != TAGN)
            break;
        const uint32_t numbers_max_count = gcoden(symbol_number);
        if (numbers_max_count == 0)
            break;
        uint32_t numbers_count = random_number_in_range(numbers_max_count) + 1;
        if (numbers_count > 2)
            if (!extended_insert_from_free_memory(refal.next_result, numbers_count - 2))
                return;
        T_LINKCB *current_argument = refal.next_result;
        while (numbers_count > 0)
        {
            current_argument = current_argument->next;
            current_argument->tag = TAGN;
            current_argument->info.code = NULL;
            pcoden(current_argument, random_number());
            numbers_count--;
        }
        transplantation(refal.previous_result, refal.next_result, current_argument->next);
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char random_0[] = {Z6 'R', 'A', 'N', 'D', 'O', 'M', (char)6};
G_L_B uint8_t refalab_random = '\122';
void (*random_1)(void) = random_;

static void random_number_(void)
{
    T_LINKCB *symbol_number = refal.previous_argument->next;
    if (symbol_number->next != refal.next_argument || symbol_number->tag != TAGN)
    {
        refal.upshot = 2;
        return;
    }
    const uint32_t max_number = gcoden(symbol_number);
    uint32_t number_random;
    if (max_number != MAX_NUMBER)
        number_random = random_number_in_range(max_number + 1);
    else
        number_random = random_number();
    pcoden(symbol_number, number_random);
    transplantation(refal.previous_result, symbol_number->previous, symbol_number->next);
    return;
}
char random_number_0[] = {Z5 'R', 'A', 'N', 'D', 'O', 'M', '_', 'N', 'U', 'M', 'B', 'E', 'R', (char)13};
G_L_B uint8_t refalab_random_number = '\122';
void (*random_number_1)(void) = random_number_;

//-----------  end of file xrandom.c  -----------
