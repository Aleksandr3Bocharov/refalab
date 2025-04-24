// Copyright 2025 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2025-04-23
// https://github.com/Aleksandr3Bocharov/RefalAB

//-------------- file -- XMO.C -------------
//                 General MO:
//     p1, m1, numb, symb, first, last,
//     lengr, lengw, multe, delf, lrel,
//     char,
//------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "refalab.h"
#include "rfintf.h"

static void p1_(void)
{
    T_LINKCB *p = refal.preva->next;
    uint32_t l;
    bool neot = false;
    if (p->next != refal.nexta || p->tag != TAGN)
        neot = true;
    else
    {

        l = gcoden(p) + 1;
        if (l > 16777215)
            neot = true;
    }
    if (neot)
    {
        refal.upshot = 2;
        return;
    }
    pcoden(p, l);
    rftpl(refal.prevr, p->prev, p->next);
    return;
}
char p1_0[] = {Z2 'P', '1', (char)2};
G_L_B uint8_t refalab_p1 = '\122';
void (*p1_1)(void) = p1_;

static void m1_(void)
{
    T_LINKCB *p = refal.preva->next;
    int32_t l;
    bool neot = false;
    if (p->next != refal.nexta || p->tag != TAGN)
        neot = true;
    else
    {
        l = (int32_t)gcoden(p) - 1;
        if (l < 0)
            neot = true;
    }
    if (neot)
    {
        refal.upshot = 2;
        return;
    }
    pcoden(p, (uint32_t)l);
    rftpl(refal.prevr, p->prev, p->next);
    return;
}
char m1_0[] = {Z2 'M', '1', (char)2};
G_L_B uint8_t refalab_m1 = '\122';
void (*m1_1)(void) = m1_;

static void numb_(void)
{
    T_LINKCB *p = refal.preva->next;
    const char zn = p->info.infoc;
    const T_LINKCB *pz = p;
    if (p->tag == TAGO && (zn == '-' || zn == '+'))
    {
        p = p->next;
        if (zn == '+')
            pz = p;
    }
    T_LINKCB *pp = p;
    while (p->tag == TAGO && p->info.infoc == '0')
        p = p->next;
    char str[11];
    size_t i;
    bool neot = false;
    for (i = 0; p != refal.nexta; i++)
    {
        if (p->tag != TAGO || i == 10)
        {
            neot = true;
            break;
        }
        str[i] = p->info.infoc;
        if (str[i] < '0' || str[i] > '9')
        {
            neot = true;
            break;
        }
        if (i == 9 && strncmp(str, "2147483647", i + 1) > 0)
        {
            neot = true;
            break;
        }
        p = p->next;
    }
    if (neot)
    {
        refal.upshot = 2;
        return;
    }
    str[i] = '\0';
    if (strlen(str) == 0)
    {
        if (!slins(refal.nexta->prev, 1))
            return;
        pp = refal.nexta->prev;
        pz = pp;
    }
    uint32_t l = (uint32_t)atol(str);
    pp->tag = TAGN;
    pp->info.codep = NULL;
    if (l > 16777215)
    {
        pcoden(pp, l >> 24);
        pp = pp->next;
        pp->tag = TAGN;
        pp->info.codep = NULL;
        l &= 0xffffff;
    }
    pcoden(pp, l);
    rftpl(refal.prevr, pz->prev, pp->next);
    return;
}
char numb_0[] = {Z4 'N', 'U', 'M', 'B', (char)4};
G_L_B uint8_t refalab_numb = '\122';
void (*numb_1)(void) = numb_;

static void symb_(void)
{
    T_LINKCB *p = refal.preva->next;
    const char zn = p->info.infoc;
    T_LINKCB *pz = p;
    if (p->tag == TAGO && (zn == '-' || zn == '+'))
    {
        p = p->next;
        if (zn == '+')
            pz = p;
    }
    T_LINKCB *pp = p;
    while (p->tag == TAGN && gcoden(p) == 0)
        p = p->next;
    size_t i;
    bool neot = false;
    for (i = 0; p != refal.nexta; i++, p = p->next)
        if (p->tag != TAGN || i == 2)
        {
            neot = true;
            break;
        }
    if (!neot)
    {
        p = p->prev;
        if (i == 2 && gcoden(pp) >= 128)
            neot = true;
    }
    if (neot)
    {
        refal.upshot = 2;
        return;
    }
    uint32_t l = gcoden(p);
    if (i == 2)
        l += 16777216 * gcoden(pp);
    if (i == 0 || l == 0)
    {
        pz = pp;
        l = 0;
    }
    char str[12];
    sprintf(str, "%u", l);
    const size_t j = strlen(str);
    if (!lrqlk(j))
        if (!lincrm())
        {
            refal.upshot = 3;
            return;
        }
    if (pz != refal.nexta)
        lins(pp, j);
    else
    {
        pz = pz->prev;
        lins(pz, j);
        pz = pz->next;
        pp = pz;
    }
    for (i = 0, p = pp; i < j; i++, p = p->next)
    {
        p->tag = TAGO;
        p->info.codep = NULL;
        p->info.infoc = str[i];
    }
    rftpl(refal.prevr, pz->prev, p);
    return;
}
char symb_0[] = {Z4 'S', 'Y', 'M', 'B', (char)4};
G_L_B uint8_t refalab_symb = '\122';
void (*symb_1)(void) = symb_;

static void first_(void)
{
    T_LINKCB *pn = refal.preva->next;
    if (pn == refal.nexta || pn->tag != TAGN)
    {
        refal.upshot = 2;
        return;
    }; // FAIL
    const uint32_t n = gcoden(pn);
    T_LINKCB *p = pn;
    for (size_t k = 1; k <= n; k++)
    {
        p = p->next;
        if (p == refal.nexta)
        {
            pn->info.codep = NULL;
            pn->info.infoc = '*';
            pn->tag = TAGO;
            rftpl(refal.prevr, refal.preva, refal.nexta);
            return;
        }
        if (p->tag == TAGLB)
            p = p->info.codep;
    }
    p = p->next;
    refal.preva->tag = TAGLB;
    refal.preva->info.codep = pn;
    pn->tag = TAGRB;
    pn->info.codep = refal.preva;
    rftpl(refal.preva, pn, p);
    rftpl(refal.prevr, refal.nextr, refal.nexta);
    return;
}
char first_0[] = {Z5 'F', 'I', 'R', 'S', 'T', (char)5};
G_L_B uint8_t refalab_first = '\122';
void (*first_1)(void) = first_;

static void last_(void)
{
    T_LINKCB *pn = refal.preva->next;
    if (pn == refal.nexta || pn->tag != TAGN)
    {
        refal.upshot = 2;
        return;
    }; // FAIL
    const uint32_t n = gcoden(pn);
    T_LINKCB *p = refal.nexta;
    for (size_t k = 1; k <= n; k++)
    {
        p = p->prev;
        if (p == pn)
        {
            pn->tag = TAGO;
            pn->info.codep = NULL;
            pn->info.infoc = '*';
            rftpl(refal.prevr, pn, refal.nexta);
            p = refal.nextr->prev;
            rftpl(p, refal.preva, refal.nexta);
            return;
        }
        if (p->tag == TAGRB)
            p = p->info.codep;
    }
    p = p->prev;
    refal.preva->tag = TAGLB;
    refal.preva->info.codep = pn;
    pn->tag = TAGRB;
    pn->info.codep = refal.preva;
    rftpl(refal.preva, p, refal.nexta);
    rftpl(refal.prevr, pn, refal.nexta);
    p = refal.nextr->prev;
    rftpl(p, refal.nextr, refal.nexta);
    return;
}
char last_0[] = {Z4 'L', 'A', 'S', 'T', (char)4};
G_L_B uint8_t refalab_last = '\122';
void (*last_1)(void) = last_;

static void lengr_(void)
{
    uint32_t n = 0;
    const T_LINKCB *p = refal.preva->next;
    while (p != refal.nexta)
    {
        n++;
        if (n > 16777215)
        {
            refal.upshot = 2;
            return;
        }
        p = p->next;
    }
    refal.preva->tag = TAGN;
    refal.preva->info.codep = NULL;
    pcoden(refal.preva, n);
    rftpl(refal.prevr, refal.nextr, refal.nexta);
    return;
}
char lengr_0[] = {Z5 'L', 'E', 'N', 'G', 'R', (char)5};
G_L_B uint8_t refalab_lengr = '\122';
void (*lengr_1)(void) = lengr_;

static void lengw_(void)
{
    uint32_t n = 0;
    const T_LINKCB *p = refal.preva->next;
    while (p != refal.nexta)
    {
        n++;
        if (n > 16777215)
        {
            refal.upshot = 2;
            return;
        }
        if (p->tag == TAGLB)
            p = p->info.codep;
        p = p->next;
    }
    refal.preva->tag = TAGN;
    refal.preva->info.codep = NULL;
    pcoden(refal.preva, n);
    rftpl(refal.prevr, refal.nextr, refal.nexta);
    return;
}
char lengw_0[] = {Z5 'L', 'E', 'N', 'G', 'W', (char)5};
G_L_B uint8_t refalab_lengw = '\122';
void (*lengw_1)(void) = lengw_;

static void multe_(void)
{
    const T_LINKCB *pn = refal.preva->next;
    if (pn == refal.nexta || pn->tag != TAGN)
    {
        refal.upshot = 2;
        return;
    }; // FAIL
    uint32_t n = gcoden(pn);
    if (n == 0)
        return;
    T_LINKCB *p = pn->next;
    if (p == refal.nexta)
        return;
    if (p->next != refal.nexta)
    {
        do
        {
            p = refal.nextr->prev;
            if (!lcopy(p, pn, refal.nexta))
            {
                refal.upshot = 3;
                return;
            }; // LACK
            n--;
        } while (n >= 1);
    }
    else
    {
        if (!slins(refal.prevr, n))
            return; //  LACK
        T_LINKCB *q = refal.prevr;
        for (uint32_t k = 0; k < n; k++)
        {
            q = q->next;
            q->tag = p->tag;
            q->info.codep = p->info.codep;
        }
    }
    return;
}
char multe_0[] = {Z5 'M', 'U', 'L', 'T', 'E', (char)5};
G_L_B uint8_t refalab_multe = '\122';
void (*multe_1)(void) = multe_;

static void delf_(void)
{
    if (refal.preva->next != refal.nexta)
    {
        refal.upshot = 2;
        return;
    } // FAIL
    const T_LINKCB *dot = refal.nexta;
    const T_LINKCB *dot1 = refal.nextr->info.codep;
    const T_LINKCB *sk;
    T_LINKCB *nd;
    while (true)
    {
        sk = dot->info.codep;
        dot = sk->info.codep;
        if (dot == NULL)
            rfabe("delf: sign '#' missing");
        nd = dot->next;
        if (nd->info.infoc != '#')
            continue;
        break;
    }
    while (dot1 != dot)
    {
        sk = dot1->info.codep;
        nd = sk->info.codep;
        rfdel(sk->prev, dot1->next);
        dot1 = nd;
    }
    sk = dot1->info.codep;
    nd = sk->info.codep;
    dot1 = dot1->next;
    rfdel(sk->prev, dot1->next);
    refal.nextr->info.codep = nd;
    return;
}
char delf_0[] = {Z4 'D', 'E', 'L', 'F', (char)4};
G_L_B uint8_t refalab_delf = '\122';
void (*delf_1)(void) = delf_;

static void lrel_(void)
{
    T_LINKCB *p = refal.preva->next;
    do
    {
        if (p->tag != TAGLB)
            break;
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
    } while (false);
    refal.upshot = 2;
    return;
}
char lrel_0[] = {Z4 'L', 'R', 'E', 'L', (char)4};
G_L_B uint8_t refalab_lrel = '\122';
void (*lrel_1)(void) = lrel_;

static void char_(void)
{
    T_LINKCB *p = refal.preva->next;
    while (p != refal.nexta)
    {
        if (p->tag == TAGN)
        {
            p->tag = TAGO;
            p->info.infoc = (char)gcoden(p);
        }
        p = p->next;
    }
    rftpl(refal.prevr, refal.preva, refal.nexta);
    return;
}
char char_0[] = {Z4 'C', 'H', 'A', 'R', (char)4};
G_L_B uint8_t refalab_char = '\122';
void (*char_1)(void) = char_;

static void ord_(void)
{
    T_LINKCB *p = refal.preva->next;
    while (p != refal.nexta)
    {
        if (p->tag == TAGO)
        {
            p->tag = TAGN;
            pcoden(p, (uint8_t)p->info.infoc);
        }
        p = p->next;
    }
    rftpl(refal.prevr, refal.preva, refal.nexta);
    return;
}
char ord_0[] = {Z3 'O', 'R', 'D', (char)3};
G_L_B uint8_t refalab_ord = '\122';
void (*ord_1)(void) = ord_;

//-------------------- end of file  XMO.C ----------------
