// Copyright 2024 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2024-09-27
// https://github.com/Aleksandr3Bocharov/RefalAB

//-------------- file -- XTIME.C ------------
//        MO: rftime (date and time)
//            rftm (current process time)
//-------------------------------------------

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <locale.h>
#include "refal.def"
#include "rfintf.h"

typedef struct timespec timespec;
static timespec t0, t1;

static void rftime_(void)
{
    setlocale(LC_TIME, setlocale(LC_TIME, ""));
    const time_t tim = time(NULL);
    char s[70];
    strftime(s, sizeof(s), "%c", localtime(&tim));
    T_LINKCB *p = refal.prevr;
    const char *c = s;
    while (*c >= ' ')
    {
        if (!slins(p, 1))
        {
            refal.upshot = 2;
            return;
        }
        p = p->next;
        p->tag = TAGO;
        p->info.codep = NULL;
        p->info.infoc = *c;
        c++;
    }
    return;
}
void (*rftime_1)(void) = rftime_;
G_L_B char rftime = '\122';
char rftime_0[] = {Z6 'R', 'F', 'T', 'I', 'M', 'E', '\006'};

static void rftm_(void)
{
    T_LINKCB *p = refal.preva->next;
    if (p != refal.nexta)
        if (p->tag == TAGO)
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
                int32_t in = t1.tv_nsec - t0.tv_nsec;
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
                for (size_t i = 0; s[i] != 0; i++)
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
void (*rftm_1)(void) = rftm_;
G_L_B char rftm = '\122';
char rftm_0[] = {Z4 'R', 'F', 'T', 'M', '\004'};

//------------------ end of file  XTIME.C ----------------
