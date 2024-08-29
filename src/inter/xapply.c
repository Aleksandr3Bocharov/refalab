//-----------  file  --  XAPPLY.C ------------
//                 MO: apply
//       Last modification : 11.07.2024
//--------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "refal.def"
#include "rfintf.h"
#include "rfrun1.h"
#include "d.h"

void (*dba)(T_ST *) = NULL;

static void appl_(void)
{
    T_ST *s_st;
    size_t l = (size_t)&s_st;
    T_ST *upst;
    T_LINKCB *px;
    bool lack = false;
    if ((l & 0xffff) < 200)
        //{ // printf("\nStack overflow!");
        lack = true;
    //}
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
    //   printf("\nEnter: %ld",upst->step);
    s_st->stop = 0x7FFFFFFF;
    do
    {
        if (dba == NULL)
            rfrun(s_st); // net prokrutki
        else
            (*dba)(s_st); // prokrutka vkluchena
        if (s_st->state == 3)
            if (lincrm())
                s_st->state = 1;
    } while (s_st->state == 1 && s_st->dot != NULL);
    rftpl(upst->store, s_st->store, s_st->store);
    upst->step = --s_st->step;
    //   printf("\nOut: %ld %lx",upst->step,upst);
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
void (*appl_1)(void) = appl_;
G_L_B char apply = '\122';
char appl_0[] = {Z5 'A', 'P', 'P', 'L', 'Y', '\005'};

//----------  end of file XAPPLY.C  -----------
