// Copyright 2025 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2025-10-31
// https://github.com/Aleksandr3Bocharov/refalab

//------------ file -- XCOMP.C ---------------
//   MO: nrel, lrel
//--------------------------------------------

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "refalab.h"
#include "rfintf.h"

static T_LINKCB *x, *y, *Xn, *Xk, *nach, *kon, *Yn, *Yk;
static size_t dl, Xdl, Ydl;
static char zn, Xzn, Yzn;

static bool dajch(void)
{
    zn = '+';
    kon = y->prev;
    if (x == kon)
    { // pustoe chislo
        dl = 0;
        return true;
    }
    x = x->next;
    if (x->tag == TAGO &&
        (x->info.infoc == '+' || x->info.infoc == '-'))
    {
        zn = x->info.infoc;
        x = x->next;
        if (x == y)
            return false; //  w chisle - lish znak
    }
    for (; x->tag == TAGN && gcoden(x) == 0; x = x->next)
        ;
    if (x == y)
        dl = 0; //  wse cifry - nuli
    else
    {
        for (dl = 0, nach = x; x->tag == TAGN; x = x->next, dl++)
            ;
        if (x != y)
            return false; // ne makrocifra
    }
    return true;
}

static bool dajarg(void)
{
    x = refal.preva->next;
    if (x->tag != TAGLB)
        return false;
    y = x->info.codep;
    if (dajch())
    {
        Xn = nach;
        Xk = kon;
        Xzn = zn;
        Xdl = dl;
    }
    else
        return false;
    x = y;
    y = refal.nexta;
    if (dajch())
    {
        Yn = nach;
        Yk = kon;
        Yzn = zn;
        Ydl = dl;
        return true;
    }
    else
        return false;
}

static uint32_t xmy(void)
{ //  if X < Y then true  ( po modulju)
    if (Xdl < Ydl)
        return 1;
    if (Xdl > Ydl)
        return 0;
    for (x = Xn, y = Yn; x != Xk->next; x = x->next, y = y->next)
    {
        if (gcoden(x) < gcoden(y))
            return 1;
        if (gcoden(x) > gcoden(y))
            return 0;
    }
    return 2; // x=y
}

static void nrel_(void)
{
    if (!dajarg())
    {
        refal.upshot = 2;
        return;
    }
    char c;
    if (Xdl == 0 && Ydl == 0)
        c = '=';
    else
    {
        if (Xzn == Yzn && xmy() == 2)
            c = '=';
        else
        {
            if ((Xzn == '-' && Yzn == '+') ||
                (Xzn == '-' && Yzn == '-' && xmy() == 0) ||
                (Xzn == '+' && Yzn == '+' && xmy() == 1))
                c = '<';
            else
                c = '>';
        }
    }
    refal.preva->tag = TAGO;
    refal.preva->info.codep = NULL;
    refal.preva->info.infoc = c;
    rftpl(refal.prevr, refal.preva->prev, refal.nexta);
    return;
}
char nrel_0[] = {Z4 'N', 'R', 'E', 'L', (char)4};
G_L_B uint8_t refalab_nrel = '\122';
void (*nrel_1)(void) = nrel_;

static void lrel_(void)
{
    T_LINKCB *p = refal.preva->next;
    if (p->tag != TAGLB)
    {
        refal.upshot = 2;
        return;
    }
    const T_LINKCB *pp = p->info.codep;
    p = p->next;
    T_LINKCB *q = pp->next;
    const T_LINKCB *q1 = refal.nexta;
    char c = '=';
    for (; c == '=' && p != pp && q != q1; p = p->next, q = q->next)
        if ((p->tag == TAGLB && q->tag == TAGLB) ||
            (p->tag == TAGRB && q->tag == TAGRB))
            continue;
        else if (p->tag == TAGLB || q->tag == TAGRB)
            c = '>';
        else if (p->tag == TAGRB || q->tag == TAGLB)
            c = '<';
        else if (p->tag > q->tag)
            c = '>';
        else if (p->tag < q->tag)
            c = '<';
        else if ((size_t)p->info.codep > (size_t)q->info.codep)
            c = '>';
        else if ((size_t)p->info.codep < (size_t)q->info.codep)
            c = '<';
    if (c == '=')
    {
        if (p == pp && q != q1)
            c = '<';
        else if (q == q1 && p != pp)
            c = '>';
    }
    p = refal.preva->next;
    p->tag = TAGO;
    p->info.codep = NULL;
    p->info.infoc = c;
    q = p->next;
    rftpl(refal.prevr, refal.preva, q);
    return;
}
char lrel_0[] = {Z4 'L', 'R', 'E', 'L', (char)4};
G_L_B uint8_t refalab_lrel = '\122';
void (*lrel_1)(void) = lrel_;

//------------------ end of file  XCOMP.C ----------------
