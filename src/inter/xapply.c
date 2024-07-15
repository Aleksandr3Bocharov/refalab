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

static void appl_()
{
    T_ST *s_st;
    uint32_t l = (uint32_t)&s_st;
    if ((l & 0xffffL) < 200L)
    { // printf("\nStack overflow!"); 
        goto LACK;
    }
    T_ST *upst = refal.currst;
    if (!slins(refal.prevr, 1))
        return;
    T_LINKCB *px = refal.prevr->next;
    if ((s_st = malloc(sizeof(T_ST))) == NULL)
        goto LACK;
    if (!lcre(s_st))
        goto LACK;
    if (!lins(s_st->view, 2))
    {
        rfcanc(s_st);
        goto LACK;
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
    s_st->stop = 0x7FFFFFFFl;
    do
    {
        if (dba == NULL)
            rfrun(s_st); // net prokrutki  
        else
            (*dba)(s_st); // prokrutka vkluchena  
        if (s_st->state == 3)
            if (lincrm())
                s_st->state = 1;
    } while ((s_st->state == 1) && (s_st->dot != NULL));
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
LACK:
    refal.upshot = 3;
    return;
}
static char appl_0[] = {Z5 'A', 'P', 'P', 'L', 'Y', '\005'};
G_L_B char apply = '\122';
static void (*appl_1)() = appl_;

//----------  end of file XAPPLY.C  ----------- 
