// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-05-03
// https://github.com/Aleksandr3Bocharov/refalab

//-------------- file -- XRAND.C ------------
//        MO: random,
//            random_number
//-------------------------------------------

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "refalab.h"
#include "rfintf.h"

#define MIN_DELAY 33
#define MAX_DELAY 97

static bool init = false;
static uint8_t n_sub_max_delay = MAX_DELAY - 1;
static uint8_t n_sub_min_delay = MIN_DELAY - 1;
static uint32_t y[MAX_DELAY];

static uint32_t random_number(void)
{
    if (!init)
    {
        srand((unsigned int)time(NULL));
        for(uint8_t i = 0; i < MAX_DELAY; i++)
            y[i] = (uint32_t)((double)rand() / RAND_MAX * MAX_NUMBER);
        init = true;
    }
    y[n_sub_max_delay] += y[n_sub_min_delay];
    const uint32_t result = y[n_sub_max_delay];
    n_sub_max_delay = (n_sub_max_delay + MAX_DELAY - 1) % MAX_DELAY;
    n_sub_min_delay = (n_sub_min_delay + MAX_DELAY - 1) % MAX_DELAY;
    return result;
}

static uint32_t random_number_in_range(uint32_t limit)
{
    if (limit == 0)
        return 0;
    const uint32_t max_valid = MAX_NUMBER - MAX_NUMBER % limit;
    uint32_t random;
    do
    {
        random = random_number();
    } while (random >= max_valid);
    return random % limit;
}

static void random_(void)
{
    T_LINKCB *p = refal.previous_argument->next;
    do
    {
        if (p->next != refal.next_argument || p->tag != TAGN)
            break;
        uint32_t count = gcoden(p);
        if (count == 0)
            break;
        count = random_number_in_range(count) + 1;
        p = refal.previous_result;
        if (!extended_insert_from_free_memory(p, count))
            return;
        while (count > 0)
        {
            p = p->next;
            p->tag = TAGN;
            p->info.code = NULL;
            pcoden(p, random_number());
            count--;
        }
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
    T_LINKCB *p = refal.previous_argument->next;
    if (p->next != refal.next_argument || p->tag != TAGN)
    {
        refal.upshot = 2;
        return;
    }
    const uint32_t max = gcoden(p);
    uint32_t res;
    if (max != MAX_NUMBER)
        res = random_number_in_range(max + 1);
    else
        res = random_number();
    pcoden(p, res);
    transplantation(refal.previous_result, p->previous, p->next);
    return;
}
char random_number_0[] = {Z5 'R', 'A', 'N', 'D', 'O', 'M', '_', 'N', 'U', 'M', 'B', 'E', 'R', (char)13};
G_L_B uint8_t refalab_random_number = '\122';
void (*random_number_1)(void) = random_number_;

//--------------- end of file  XRAND.C -------------
