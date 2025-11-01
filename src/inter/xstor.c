// Copyright 2025 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2025-11-01
// https://github.com/Aleksandr3Bocharov/refalab

//-------------- file -- XSTOR.C -----------
//       MO: br, dg, dgall, rp, cp
//------------------------------------------

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "refalab.h"
#include "rfintf.h"

static void br_(void)
{
    const T_ST *ast = refal.currst;
    const T_LINKCB *p = refal.preva;
    while (p->tag != TAGO || p->info.infoc != '=')
    {
        p = p->next;
        if (p == refal.nexta)
        {
            refal.upshot = 2;
            return;
        }; // FAIL
    }
    if (!lins(ast->store, 2))
    {
        refal.upshot = 3;
        return;
    }; // LACK
    T_LINKCB *pl = ast->store->next;
    T_LINKCB *pr = pl->next;
    pl->info.codep = pr;
    pl->tag = TAGLB;
    pr->info.codep = pl;
    pr->tag = TAGRB;
    rftpl(pl, refal.preva, refal.nexta);
    return;
}
char br_0[] = {Z2 'B', 'R', (char)2};
G_L_B uint8_t refalab_br = '\122';
void (*br_1)(void) = br_;

static void dg_(void)
{
    const T_ST *ast = refal.currst;
    T_LINKCB *pr = ast->store;
    T_LINKCB *q, *pl;
    while (true)
    {
        pl = pr->next;
        if (pl == ast->store)
            return;
        if (pl->tag != TAGLB)
        {
            refal.upshot = 2;
            return;
        }; // FAIL
        pr = pl->info.codep;
        q = lldupl(refal.preva, refal.nexta, pl);
        if (q == NULL)
            continue;
        if (q->tag != TAGO || q->info.infoc != '=')
            continue;
        break;
    }
    rftpl(refal.prevr, q, pr);
    pl = pl->prev;
    pr = pr->next;
    rfdel(pl, pr);
    return;
}
char dg_0[] = {Z2 'D', 'G', (char)2};
G_L_B uint8_t refalab_dg = '\122';
void (*dg_1)(void) = dg_;

static void dgall_(void)
{
    const T_ST *ast = refal.currst;
    if (refal.preva->next != refal.nexta)
        refal.upshot = 2; // FAIL
    else
        rftpl(refal.prevr, ast->store, ast->store);
    return;
}
char dgal_0[] = {Z5 'D', 'G', 'A', 'L', 'L', (char)5};
G_L_B uint8_t refalab_dgall = '\122';
void (*dgal_1)(void) = dgall_;

static void rp_(void)
{
    const T_ST *ast = refal.currst;
    T_LINKCB *p = refal.preva;
    bool fail = false;
    while (p->tag != TAGO || p->info.infoc != '=')
    {
        p = p->next;
        if (p == refal.nexta)
        {
            fail = true;
            break;
        }
    };
    if (!fail)
    {
        T_LINKCB *pr = ast->store;
        while (true)
        {
            T_LINKCB *pl = pr->next;
            if (pl == ast->store)
            {
                if (!lins(ast->store, 2))
                {
                    refal.upshot = 3;
                    return;
                }; // LACK
                pl = ast->store->next;
                pr = pl->next;
                pl->info.codep = pr;
                pl->tag = TAGLB;
                pr->info.codep = pl;
                pr->tag = TAGRB;
                rftpl(pl, refal.preva, refal.nexta);
            }
            else
            {
                if (pl->tag != TAGLB)
                    break;
                pr = pl->info.codep;
                T_LINKCB *q = lldupl(refal.preva, p, pl);
                if (q == NULL)
                    continue;
                if (q->tag != TAGO || q->info.infoc != '=')
                    continue;
                rfdel(q, pr);
                rftpl(q, p, refal.nexta);
            }
            return;
        }
    }
    refal.upshot = 2;
    return;
}
char rp_0[] = {Z2 'R', 'P', (char)2};
G_L_B uint8_t refalab_rp = '\122';
void (*rp_1)(void) = rp_;

static void cp_(void)
{
    const T_ST *ast = refal.currst;
    const T_LINKCB *pr = ast->store;
    const T_LINKCB *q;
    while (true)
    {
        const T_LINKCB *pl = pr->next;
        if (pl == ast->store)
            return;
        if (pl->tag != TAGLB)
        {
            refal.upshot = 2;
            return;
        }; // FAIL
        pr = pl->info.codep;
        q = lldupl(refal.preva, refal.nexta, pl);
        if (q == NULL)
            continue;
        if (q->tag != TAGO || q->info.infoc != '=')
            continue;
        break;
    }
    if (!lcopy(refal.prevr, q, pr))
        refal.upshot = 3; // LACK
    return;
}
char cp_0[] = {Z2 'C', 'P', (char)2};
G_L_B uint8_t refalab_cp = '\122';
void (*cp_1)(void) = cp_;

//----------------- end of file  XSTOR.C ---------------
