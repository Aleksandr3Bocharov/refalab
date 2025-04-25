// Copyright 2025 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2025-04-25
// https://github.com/Aleksandr3Bocharov/RefalAB

//-------------- file -- XRAND.C ------------
//        MO: random,
//            random_number
//-------------------------------------------

// #include <stdio.h>
#include <stdint.h>
#include "refalab.h"
#include "rfintf.h"

/*
static r05_number random_digit_in_range(r05_number limit)
{
    const r05_number MAX = ~0;
    r05_number max_valid;
    r05_number random;

    if (limit == 0)
    {
        return 0;
    }

    max_valid = MAX - MAX % limit;

    do
    {
        random = random_digit();
    } while (random >= max_valid);

    return random % limit;
}

static r05_number random_digit(void)
{
    enum
    {
        cMinDelay = 24,
        cMaxDelay = 55
    };

    static int init = 0;
    static size_t k = cMaxDelay - 1;
    static size_t j = cMinDelay - 1;
    static r05_number y[cMaxDelay];

    r05_number result;

    if (!init)
    {
        r05_number seed = (r05_number)time(NULL);
        size_t i;

        for (i = 0; i < cMaxDelay; ++i)
        {
            seed = seed * 1103515245 + 12345;
            y[i] = seed;
        }

        init = 1;
    }

    result = y[k] = y[k] + y[j];
    k = (k + cMaxDelay - 1) % cMaxDelay;
    j = (j + cMaxDelay - 1) % cMaxDelay;

    return result;
}
*/

static void random_(void)
{
    /*    struct r05_node *callable = arg_begin->next;
        struct r05_node *pcount = callable->next;
        r05_number count;

        if (R05_DATATAG_NUMBER != pcount->tag || pcount->next != arg_end)
        {
            r05_recognition_impossible();
        }

        count = pcount->info.number;
        count = count > 0 ? count - 1 : 1;
        count = random_digit_in_range(count) + 1;

        r05_reset_allocator();
        while (count > 0)
        {
            r05_alloc_number(random_digit());
            --count;
        }

        r05_splice_from_freelist(arg_begin);
        r05_splice_to_freelist(arg_begin, arg_end);
    */
    const T_LINKCB *p = refal.preva->next;
    uint32_t count = gcoden(p);
    if (p->next != refal.nexta || p->tag != TAGN || count == 0)
    {
        refal.upshot = 2;
        return;
    }
    count = random_digit_in_range(count) + 1;
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
    if (max != 16777215)
        res = random_digit_in_range(max + 1);
    else
        res = random_digit();
    pcoden(p, res);
    rftpl(refal.prevr, refal.preva, refal.nexta);
    return;
}
char random_number_0[] = {Z5 'R', 'A', 'N', 'D', 'O', 'M', '_', 'N', 'U', 'M', 'B', 'E', 'R', (char)13};
G_L_B uint8_t refalab_random_number = '\122';
void (*random_number_1)(void) = random_number_;

//--------------- end of file  XRAND.C -------------