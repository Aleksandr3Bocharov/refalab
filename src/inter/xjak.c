// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-04-14
// https://github.com/Aleksandr3Bocharov/refalab

//-------------- file -- XJAK.C ------------
//       MO: gtr, rdr, wtr, swr, new
//------------------------------------------

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "refalab.h"
#include "rfintf.h"

#define N_SWAP 0116

static bool enter(bool emp, T_LINKCB **pp, T_LINKCB **rp)
{
    T_LINKCB *r = refal.previous_argument->next;
    if (r == refal.next_argument)
        return false;
    if (emp && r->next != refal.next_argument)
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
        if (p->previous == NULL)
        {
            p->next = p;
            p->previous = p->next;
            p->info.codep = refal.static_boxes;
            p->tag = TAGO;
            refal.static_boxes = p;
        }
    }
    else
        return false;
    *pp = p;
    *rp = r;
    return true;
}

static void gtr_(void)
{
    const bool emp = true;
    T_LINKCB *p = NULL, *r;
    if (!enter(emp, &p, &r))
    {
        refal.upshot = 2;
        return;
    }; // FAIL
    rftpl(refal.previous_result, p, p);
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
    if (!lcopy(refal.previous_result, p, p))
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
    T_LINKCB *q = p->previous;
    rftpl(q, r, refal.next_argument);
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
    rftpl(p, r, refal.next_argument);
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
    rftpl(refal.previous_result, p, p);
    rftpl(p, r, refal.next_argument);
    return;
}
char swr_0[] = {Z3 'S', 'W', 'R', (char)3};
G_L_B uint8_t refalab_swr = '\122';
void (*swr_1)(void) = swr_;

static void new_(void)
{
    if (!insert_from_free_memory_list(refal.previous_result, 1))
    {
        refal.upshot = 3;
        return;
    }; // LACK
    T_LINKCB *r = refal.previous_result->next;
    r->info.codep = refal.previous_argument;
    r->tag = TAGR;
    T_LINKCB *p = refal.next_argument->previous;
    p->next = refal.previous_argument;
    refal.previous_argument->previous = p;
    refal.next_result->next = refal.next_argument;
    refal.next_argument->previous = refal.next_result;
    refal.previous_argument->info.codep = refal.dynamic_boxes;
    refal.previous_argument->tag = TAGO;
    refal.dynamic_boxes = refal.previous_argument;
    return;
}
char new_0[] = {Z3 'N', 'E', 'W', (char)3};
G_L_B uint8_t refalab_new = '\122';
void (*new_1)(void) = new_;

//----------------- end of file  XJAK.C ----------------
