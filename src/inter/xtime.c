// Copyright 2025 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2025-03-07
// https://github.com/Aleksandr3Bocharov/RefalAB

//-------------- file -- XTIME.C ------------
//        MO: time (date and time)
//            tm (current process time)
//-------------------------------------------

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <locale.h>
#include "refal.def"
#include "rfintf.h"

static T_TIMESPEC t0, t1;

static void time_(void)
{
    if (refal.preva->next != refal.nexta)
    {
        refal.upshot = 2;
        return;
    }
    setlocale(LC_TIME, "");
    const time_t tim = time(NULL);
    char s[70];
    strftime(s, sizeof(s), "%c", localtime(&tim));
    T_LINKCB *p = refal.prevr;
    if (!slins(p, strlen(s)))
        return;
    for (size_t i = 0; s[i] != '\0'; i++)
    {
        p = p->next;
        p->tag = TAGO;
        p->info.codep = NULL;
        p->info.infoc = s[i];
    }
    return;
}
char time_0[] = {Z4 'T', 'I', 'M', 'E', '\004'};
G_L_B uint8_t refalab_time = '\122';
void (*time_1)(void) = time_;

static void tm_(void)
{
    T_LINKCB *p = refal.preva->next;
    if (p->next == refal.nexta && p->tag == TAGO)
    {
        const char c = p->info.infoc;
        switch (c)
        {
        case 's':
        case 'S':
            timespec_get(&t0, TIME_UTC);
            return;
        case 'g':
        case 'G':
            timespec_get(&t1, TIME_UTC);
            int32_t in = (int32_t)(t1.tv_nsec - t0.tv_nsec);
            uint32_t is = (uint32_t)difftime(t1.tv_sec, t0.tv_sec);
            if (in < 0)
            {
                in += 1000000000;
                is--;
            }
            uint32_t im = is / 60;
            is %= 60;
            const uint32_t ih = im / 60;
            im %= 60;
            char s[30];
            sprintf(s, "%02u:%02u:%02u.%09d", ih, im, is, in);
            p = refal.prevr;
            if (!slins(p, strlen(s)))
                return;
            for (size_t i = 0; s[i] != '\0'; i++)
            {
                p = p->next;
                p->tag = TAGO;
                p->info.codep = NULL;
                p->info.infoc = s[i];
            }
            return;
        default:;
        }
    }
    refal.upshot = 2;
    return;
}
char tm_0[] = {Z2 'T', 'M', '\002'};
G_L_B uint8_t refalab_tm = '\122';
void (*tm_1)(void) = tm_;

//------------------ end of file  XTIME.C ----------------
