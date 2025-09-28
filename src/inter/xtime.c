// Copyright 2025 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2025-09-28
// https://github.com/Aleksandr3Bocharov/RefalAB

//-------------- file -- XTIME.C -----------------
//        MO: time (date and time),
//            tm (timer),
//            tm_elapsed (current program time)
//------------------------------------------------

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <locale.h>
#include "refalab.h"
#include "rfintf.h"

extern uint8_t refalab_null;

static T_TIMESPEC t0, t1, tm_e;

static void time_(void)
{
    if (refal.preva->next != refal.nexta)
    {
        refal.upshot = 2;
        return;
    }
    setlocale(LC_TIME, "");
    const time_t tim = time(NULL);
    const struct tm *lt = localtime(&tim);
    if (lt == NULL)
    {
        refal.preva->info.codef = &refalab_null;
        rftpl(refal.prevr, refal.nextr, refal.preva->next);
        return;
    }
    char s[256];
    const size_t sl = strftime(s, sizeof(s), "%c", lt);
    if (sl == 0)
        return;
    if (!slins(refal.nextr, sl - 1))
        return;
    rfrstr(s, refal.nextr);
    rftpl(refal.prevr, refal.nextr, refal.nexta);
    return;
}
char time_0[] = {Z4 'T', 'I', 'M', 'E', (char)4};
G_L_B uint8_t refalab_time = '\122';
void (*time_1)(void) = time_;

static void tm_(void)
{
    do
    {
        T_LINKCB *p = refal.preva->next;
        if (p->next != refal.nexta || p->tag != TAGO)
            break;
        const char c = p->info.infoc;
        if (c == 'S' || c == 's')
            timespec_get(&t0, TIME_UTC);
        else if (c == 'G' || c == 'g')
        {
            timespec_get(&t1, TIME_UTC);
            long int in = t1.tv_nsec - t0.tv_nsec;
            long long int is = (long long int)difftime(t1.tv_sec, t0.tv_sec);
            if (in < 0)
            {
                in += 1000000000;
                is--;
            }
            long long int im = is / 60;
            is %= 60;
            const long long int ih = im / 60;
            im %= 60;
            char s[64];
            sprintf(s, "%02lld:%02lld:%02lld.%09ld", ih, im, is, in);
            if (!slins(refal.nextr, strlen(s) - 2))
                return;
            rfrstr(s, refal.nextr);
            rftpl(refal.prevr, refal.nextr, refal.nexta);
        }
        else
            break;
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char tm_0[] = {Z2 'T', 'M', (char)2};
G_L_B uint8_t refalab_tm = '\122';
void (*tm_1)(void) = tm_;

static void tm_elapsed_(void)
{
    if (refal.preva->next != refal.nexta)
    {
        refal.upshot = 2;
        return;
    }
    if (!refal.tm.mode)
        return;
    timespec_get(&tm_e, TIME_UTC);
    long int in = tm_e.tv_nsec - refal.tm.start.tv_nsec;
    long long int is = (long long int)difftime(tm_e.tv_sec, refal.tm.start.tv_sec);
    if (in < 0)
    {
        in += 1000000000;
        is--;
    }
    long long int im = is / 60;
    is %= 60;
    const long long int ih = im / 60;
    im %= 60;
    char s[64];
    sprintf(s, "%02lld:%02lld:%02lld.%09ld", ih, im, is, in);
    if (!slins(refal.nextr, strlen(s) - 1))
        return;
    rfrstr(s, refal.nextr);
    rftpl(refal.prevr, refal.nextr, refal.nexta);
}
char tm_elapsed_0[] = {Z2 'T', 'M', '_', 'E', 'L', 'A', 'P', 'S', 'E', 'D', (char)10};
G_L_B uint8_t refalab_tm_elapsed = '\122';
void (*tm_elapsed_1)(void) = tm_elapsed_;

//-------------- end of file  XTIME.C --------------
