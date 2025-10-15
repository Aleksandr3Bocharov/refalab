// Copyright 2025 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2025-03-20
// https://github.com/Aleksandr3Bocharov/refalab

//-------------- file -- XJAK.C ------------
//       MO: br, dg, dgall, rp, cp
//           gtr, rdr, wtr, swr, new
//------------------------------------------

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "refalab.h"
#include "rfintf.h"

#define N_SWAP 0116

static bool enter(bool emp, T_LINKCB **pp, T_LINKCB **rp)
{
    T_LINKCB *r = refal.preva->next;
    if (r == refal.nexta)
        return false;
    if (emp && r->next != refal.nexta)
        return true;
    T_LINKCB *p;
    if (r->tag == TAGR)
        p = r->info.codep;
    else if (r->tag == TAGF)
    {
        const uint8_t *q = r->info.codef;
        if (*q != N_SWAP)
            return false;
        q++;
        p = (T_LINKCB *)q;
        if (p->prev == NULL)
        {
            p->next = p;
            p->prev = p->next;
            p->info.codep = refal.svar;
            p->tag = TAGO;
            refal.svar = p;
        }
    }
    else
        return false;
    *pp = p;
    *rp = r;
    return true;
}

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

static void gtr_(void)
{
    const bool emp = true;
    T_LINKCB *p = NULL, *r;
    if (!enter(emp, &p, &r))
    {
        refal.upshot = 2;
        return;
    }; // FAIL
    rftpl(refal.prevr, p, p);
    return;
}
char gtr_0[] = {Z3 'G', 'T', 'R', (char)3};
G_L_B uint8_t refalab_gtr = '\122';
void (*gtr_1)(void) = gtr_;

static void rdr_(void)
{
    const bool emp = true;
    T_LINKCB *p = NULL, *r;
    if (!enter(emp, &p, &r))
    {
        refal.upshot = 2;
        return;
    }; // FAIL
    if (!lcopy(refal.prevr, p, p))
    {
        refal.upshot = 3;
        return;
    }; // LACK
    return;
}
char rdr_0[] = {Z3 'R', 'D', 'R', (char)3};
G_L_B uint8_t refalab_rdr = '\122';
void (*rdr_1)(void) = rdr_;

static void ptr_(void)
{
    const bool emp = false;
    T_LINKCB *p, *r;
    if (!enter(emp, &p, &r))
    {
        refal.upshot = 2;
        return;
    }; // FAIL
    T_LINKCB *q = p->prev;
    rftpl(q, r, refal.nexta);
    return;
}
char ptr_0[] = {Z3 'P', 'T', 'R', (char)3};
G_L_B uint8_t refalab_ptr = '\122';
void (*ptr_1)(void) = ptr_;

static void wtr_(void)
{
    const bool emp = false;
    T_LINKCB *p, *r;
    if (!enter(emp, &p, &r))
    {
        refal.upshot = 2;
        return;
    }; // FAIL
    rfdel(p, p);
    rftpl(p, r, refal.nexta);
    return;
}
char wtr_0[] = {Z3 'W', 'T', 'R', (char)3};
G_L_B uint8_t refalab_wtr = '\122';
void (*wtr_1)(void) = wtr_;

static void swr_(void)
{
    const bool emp = false;
    T_LINKCB *p, *r;
    if (!enter(emp, &p, &r))
    {
        refal.upshot = 2;
        return;
    }; // FAIL
    rftpl(refal.prevr, p, p);
    rftpl(p, r, refal.nexta);
    return;
}
char swr_0[] = {Z3 'S', 'W', 'R', (char)3};
G_L_B uint8_t refalab_swr = '\122';
void (*swr_1)(void) = swr_;

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

static void new_(void)
{
    if (!lins(refal.prevr, 1))
    {
        refal.upshot = 3;
        return;
    }; // LACK
    T_LINKCB *r = refal.prevr->next;
    r->info.codep = refal.preva;
    r->tag = TAGR;
    T_LINKCB *p = refal.nexta->prev;
    p->next = refal.preva;
    refal.preva->prev = p;
    refal.nextr->next = refal.nexta;
    refal.nexta->prev = refal.nextr;
    refal.preva->info.codep = refal.dvar;
    refal.preva->tag = TAGO;
    refal.dvar = refal.preva;
    return;
}
char new_0[] = {Z3 'N', 'E', 'W', (char)3};
G_L_B uint8_t refalab_new = '\122';
void (*new_1)(void) = new_;

//----------------- end of file  XJAK.C ----------------
