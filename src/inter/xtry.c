// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-04-14
// https://github.com/Aleksandr3Bocharov/refalab

//-----------  file  --  XTRY.C --------------
//                 MO: try
//--------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "refalab.h"
#include "rfintf.h"
#include "rfrun.h"
#include "d.h"

static void try_(void)
{
    T_STATUS_TABLE *s_st;
    size_t l = (size_t)&s_st;
    T_STATUS_TABLE *upst;
    T_LINKCB *px;
    bool lack = false;
    if ((l & 0xffff) < 200)
        lack = true;
    else
    {
        upst = refal.current_status_table;
        if (!extended_insert_from_free_memory_list(refal.previous_result, 1))
            return;
        px = refal.previous_result->next;
        s_st = malloc(sizeof(T_STATUS_TABLE));
        if (s_st == NULL)
            lack = true;
        else if (!create_status_table(s_st))
            lack = true;
        else if (!insert_from_free_memory_list(s_st->view, 2))
        {
            delete_status_table(s_st);
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
    pk->info.code = NULL;
    pk->tag = TAGK;
    pd->info.codep = pk;
    pd->tag = TAGD;
    s_st->dot = pd;
    transplantation(pk, refal.previous_argument, refal.next_argument);
    transplantation(s_st->store, upst->store, upst->store);
    s_st->step = ++upst->step;
    s_st->stop = MAX_STOP;
#if defined mdebug
    const uint32_t s_stop = s_st->stop;
#endif
    do
    {
        if (status_table_debugger == NULL)
        {
#if defined mdebug
            if (s_st->step >= s_stop)
                s_st->step = 0;
            s_st->stop = s_st->step + 1;
            const T_LINKCB *pk1 = s_st->dot->info.codep;
            const T_LINKCB *prevk = pk1->previous;
            const T_LINKCB *nextd = s_st->dot->next;
            printf(" Step: %d\n", s_st->stop);
            print_expression_m(" Term: ", prevk, nextd, true);
            refal_run(s_st);
            if (s_st->state == 1)
                print_expression_m(" Result: ", prevk, nextd, true);
#else
            refal_run(s_st); // net prokrutki
            if (s_st->state == 1 && s_st->dot != NULL)
            {
                s_st->step = 0;
                continue;
            }
#endif
        }
        else
            (*status_table_debugger)(s_st); // prokrutka vkluchena
        if (s_st->state == 3)
            if (more_free_memory())
                s_st->state = 1;
    } while (s_st->state == 1 && s_st->dot != NULL);
    transplantation(upst->store, s_st->store, s_st->store);
    upst->step = --s_st->step;
#if defined mdebug
    if (status_table_debugger == NULL)
        upst->stop = upst->step + 1;
#endif
    switch (s_st->state)
    {
    case 1:
        px->info.infoc = 'N';
        transplantation(px, s_st->view, s_st->view);
        break;
    case 2:
        px->info.infoc = 'R';
        pd = s_st->dot;
        pk = pd->info.codep;
        transplantation(px, pk, pd);
        break;
    case 3:
        px->info.infoc = 'S';
        break;
    }
    delete_status_table(s_st);
    free(s_st);
    return;
}
char try_0[] = {Z3 'T', 'R', 'Y', (char)3};
G_L_B uint8_t refalab_try = '\122';
void (*try_1)(void) = try_;

//----------  end of file XTRY.C  ---------------
