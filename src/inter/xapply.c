// Copyright 2025 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2025-02-17
// https://github.com/Aleksandr3Bocharov/RefalAB

//-----------  file  --  XAPPLY.C ------------
//                 MO: apply
//--------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "refal.def"
#include "rfintf.h"
#include "rfrun1.h"
#include "d.h"

static void appl_(void)
{
    T_ST *s_st;
    size_t l = (size_t)&s_st;
    T_ST *upst;
    T_LINKCB *px;
    bool lack = false;
    if ((l & 0xffff) < 200)
        lack = true;
    else
    {
        upst = refal.currst;
        if (!slins(refal.prevr, 1))
            return;
        px = refal.prevr->next;
        s_st = malloc(sizeof(T_ST));
        if (s_st == NULL)
            lack = true;
        else if (!lcre(s_st))
            lack = true;
        else if (!lins(s_st->view, 2))
        {
            rfcanc(s_st);
            lack = true;
        }
    }
    if (lack)
    {
        refal.upshot = 3;
        return;
    }
    T_LINKCB *pk = s_st->view->next;
    T_LINKCB *pd = pk->next;
    pk->info.codep = NULL;
    pk->tag = TAGK;
    pd->info.codep = pk;
    pd->tag = TAGD;
    s_st->dot = pd;
    rftpl(pk, refal.preva, refal.nexta);
    rftpl(s_st->store, upst->store, upst->store);
    s_st->step = ++upst->step;
    s_st->stop = 10;
    // s_st->stop = 0x7FFFFFFF;
#ifdef mdebug
    const uint32_t s_stop = s_st->stop;
#endif
    do
    {
        if (dba == NULL)
        {
#ifdef mdebug
            //if (s_st->step >= s_stop)
            //    s_st->step = 0;
            s_st->stop = s_st->step + 1;
            const T_LINKCB *pk = s_st->dot->info.codep;
            const T_LINKCB *prevk = pk->prev;
            const T_LINKCB *nextd = s_st->dot->next;
            printf("\n Step: %d", s_st->stop);
            rfpexm(" Term: ", prevk, nextd);
            rfrun(s_st);
            if (s_st->state == 1)
                rfpexm(" Result: ", prevk, nextd);
#else
            rfrun(s_st); // net prokrutki
            if (s_st->state == 1 && s_st->dot != NULL)
            {
                s_st->step = 0;
                continue;
            }
#endif
        }
        else
            (*dba)(s_st); // prokrutka vkluchena
        if (s_st->state == 3)
            if (lincrm())
                s_st->state = 1;
    } while (s_st->state == 1 && s_st->dot != NULL);
    rftpl(upst->store, s_st->store, s_st->store);
    upst->step = --s_st->step;
    switch (s_st->state)
    {
    case 1:
        px->info.infoc = 'N';
        rftpl(px, s_st->view, s_st->view);
        break;
    case 2:
        px->info.infoc = 'R';
        pd = s_st->dot;
        pk = pd->info.codep;
        rftpl(px, pk, pd);
        break;
    case 3:
        px->info.infoc = 'S';
        break;
    }
    rfcanc(s_st);
    free(s_st);
    return;
}
char appl_0[] = {Z5 'A', 'P', 'P', 'L', 'Y', '\005'};
G_L_B uint8_t apply = '\122';
void (*appl_1)(void) = appl_;

//----------  end of file XAPPLY.C  -----------
