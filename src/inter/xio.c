// Copyright 2025 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2025-05-09
// https://github.com/Aleksandr3Bocharov/RefalAB

//-------------- file -- XIO.C ------------
//           MO: input/output
//-----------------------------------------

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "refalab.h"
#include "rfintf.h"

static void card_(void)
{
    if (refal.preva->next != refal.nexta)
        rfpex("", refal.preva, refal.nexta, false);
    T_LINKCB *p = refal.prevr;
    int c = getchar();
    while (c != '\n')
    {
        if  (!slins(p, 1))
            return;
        p = p->next;
        p->info.codep = NULL;
        if (c == EOF)
        {
            p->tag = TAGN;
            return;
        }
        p->tag = TAGO;
        p->info.infoc = (char)c;
        c = getchar();
    }
    return;
}
char card_0[] = {Z4 'C', 'A', 'R', 'D', (char)4};
G_L_B uint8_t refalab_card = '\122';
void (*card_1)(void) = card_;

static void pr1_(void)
{
    rfpex("", refal.preva, refal.nexta, true);
    rftpl(refal.prevr, refal.preva, refal.nexta);
    return;
}
char pr1_0[] = {Z5 'P', 'R', 'I', 'N', 'T', (char)5};
G_L_B uint8_t refalab_print = '\122';
void (*pr1_1)(void) = pr1_;

static void pr2_(void)
{
    rfpexm("", refal.preva, refal.nexta, true);
    rftpl(refal.prevr, refal.preva, refal.nexta);
    return;
}
char pr2_0[] = {Z6 'P', 'R', 'I', 'N', 'T', 'M', (char)6};
G_L_B uint8_t refalab_printm = '\122';
void (*pr2_1)(void) = pr2_;

static void pr3_(void)
{
    rfpex("", refal.preva, refal.nexta, true);
    return;
}
char pr3_0[] = {Z5 'P', 'R', 'O', 'U', 'T', (char)5};
G_L_B uint8_t refalab_prout = '\122';
void (*pr3_1)(void) = pr3_;

static void pr4_(void)
{
    rfpexm("", refal.preva, refal.nexta, true);
    return;
}
char pr4_0[] = {Z6 'P', 'R', 'O', 'U', 'T', 'M', (char)6};
G_L_B uint8_t refalab_proutm = '\122';
void (*pr4_1)(void) = pr4_;

static void wr1_(void)
{
    rfpex("", refal.preva, refal.nexta, false);
    rftpl(refal.prevr, refal.preva, refal.nexta);
    return;
}
char wr1_0[] = {Z5 'W', 'R', 'I', 'T', 'E', (char)5};
G_L_B uint8_t refalab_write = '\122';
void (*wr1_1)(void) = wr1_;

static void wr2_(void)
{
    rfpexm("", refal.preva, refal.nexta, false);
    rftpl(refal.prevr, refal.preva, refal.nexta);
    return;
}
char wr2_0[] = {Z6 'W', 'R', 'I', 'T', 'E', 'M', (char)6};
G_L_B uint8_t refalab_writem = '\122';
void (*wr2_1)(void) = wr2_;

static void wr3_(void)
{
    rfpex("", refal.preva, refal.nexta, false);
    return;
}
char wr3_0[] = {Z5 'W', 'R', 'O', 'U', 'T', (char)5};
G_L_B uint8_t refalab_wrout = '\122';
void (*wr3_1)(void) = wr3_;

static void wr4_(void)
{
    rfpexm("", refal.preva, refal.nexta, false);
    return;
}
char wr4_0[] = {Z6 'W', 'R', 'O', 'U', 'T', 'M', (char)6};
G_L_B uint8_t refalab_wroutm = '\122';
void (*wr4_1)(void) = wr4_;

//------------------ end of file  XIO.C ----------------
