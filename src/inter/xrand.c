// Copyright 2025 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2025-09-14
// https://github.com/Aleksandr3Bocharov/RefalAB

//-------------- file -- XRAND.C ------------
//        MO: random,
//            random_number
//-------------------------------------------

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "refalab.h"
#include "rfintf.h"

#define CMINDELAY 24
#define CMAXDELAY 55

static bool init = false;
static size_t k = CMAXDELAY - 1;
static size_t j = CMINDELAY - 1;
static uint32_t y[CMAXDELAY];

static uint32_t random_number(void)
{
    if (!init)
    {
        uint32_t seed = (uint32_t)time(NULL);
        for (size_t i = 0; i < CMAXDELAY; ++i)
        {
            seed = seed * 1103515245 + 12345;
            y[i] = seed;
        }
        init = true;
    }
    y[k] += y[j];
    const uint32_t result = y[k];
    k = (k + CMAXDELAY - 1) % CMAXDELAY;
    j = (j + CMAXDELAY - 1) % CMAXDELAY;
    return result % (MAX_NUMBER + 1);
}

static uint32_t random_number_in_range(uint32_t limit)
{
    if (limit == 0)
        return 0;
    const uint32_t max = MAX_NUMBER;
    const uint32_t max_valid = max - max % limit;
    uint32_t random;
    do
    {
        random = random_number();
    } while (random >= max_valid);
    return random % limit;
}

static void random_(void)
{
    T_LINKCB *p = refal.preva->next;
    do
    {
        if (p->next != refal.nexta || p->tag != TAGN)
            break;
        uint32_t count = gcoden(p);
        if (count == 0)
            break;
        count = random_number_in_range(count) + 1;
        p = refal.prevr;
        if (!slins(p, count))
            return;
        while (count > 0)
        {
            p = p->next;
            p->tag = TAGN;
            p->info.codep = NULL;
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
    T_LINKCB *p = refal.preva->next;
    if (p->next != refal.nexta || p->tag != TAGN)
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
    rftpl(refal.prevr, p->prev, p->next);
    return;
}
char random_number_0[] = {Z5 'R', 'A', 'N', 'D', 'O', 'M', '_', 'N', 'U', 'M', 'B', 'E', 'R', (char)13};
G_L_B uint8_t refalab_random_number = '\122';
void (*random_number_1)(void) = random_number_;

//--------------- end of file  XRAND.C -------------
