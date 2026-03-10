// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-03-10
// https://github.com/Aleksandr3Bocharov/refalab

//------------ file -- XCF.C ---------------
//   MO: ftochar, functab, chartof
//------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include "refalab.h"
#include "rfintf.h"

typedef uint8_t *adr;

static size_t func_n = 0;
static adr *func_f = NULL;

static void ftochar_(void)
{
    T_LINKCB *p = refal.preva->next;
    if (p->next != refal.nexta || p->tag != TAGF)
    {
        refal.upshot = 2;
        return;
    }
    const uint8_t *lp = p->info.codef - 1;
    const uint8_t l = *lp;
    const char *u = (char *)lp - l;
    p = refal.prevr;
    if (!lrqlk(l))
        if (!lincrm())
        {
            rfdel(refal.prevr, refal.nextr);
            refal.upshot = 3;
            return;
        }
    lins(p, l);
    for (uint8_t i = 0; i < l; i++)
    {
        p = p->next;
        p->tag = TAGO;
        p->info.codep = NULL;
        p->info.infoc = *(u + i);
    }
    return;
}
char ftochar_0[] = {Z7 'F', 'T', 'O', 'C', 'H', 'A', 'R', (char)7};
G_L_B uint8_t refalab_ftochar = '\122';
void (*ftochar_1)(void) = ftochar_;

static void functab_(void)
{
    const T_LINKCB *p = refal.preva->next;
    if (p->next != refal.nexta || p->tag != TAGF)
    {
        refal.upshot = 2;
        return;
    }
    uint8_t *u = p->info.codef;
    for (size_t i = 0; i < func_n; i++)
        if (u == func_f[i])
            return;
    if (func_n == 0)
        func_f = (adr *)malloc(sizeof(adr));
    else
        func_f = (adr *)realloc(func_f, (func_n + 1) * sizeof(adr));
    if (func_f == NULL)
        rfabe("functab: malloc or realloc error");
    func_f[func_n] = u;
    func_n++;
    return;
}
char functab_0[] = {Z7 'F', 'U', 'N', 'C', 'T', 'A', 'B', (char)7};
G_L_B uint8_t refalab_functab = '\122';
void (*functab_1)(void) = functab_;

static void chartof_(void)
{
    T_LINKCB *p = refal.preva->next;
    size_t i;
    bool heot = false;
    if (p == refal.nexta)
        heot = true;
    else
    {
        for (i = 0; p != refal.nexta; i++, p = p->next)
            if (p->tag != TAGO)
            {
                heot = true;
                break;
            }
        if (i > 255)
            heot = true;
    }
    if (heot)
    {
        refal.upshot = 2;
        return;
    }
    p = refal.preva->next;
    char *u = (char *)malloc(i + 2);
    if (u == NULL)
        rfabe("chartof: malloc error");
    for (i = 0; p != refal.nexta; i++, p = p->next)
        u[i] = (char)toupper(p->info.infoc);
    u[i] = (char)i;
    ++i;
    u[i] = 2; // HEOT
    uint8_t *j = (uint8_t *)(u + i);
    uint8_t l;
    const uint8_t *lp;
    for (size_t k = 0; k < func_n; k++)
    {
        lp = func_f[k] - 1;
        l = *lp;
        if (i == (size_t)l + 1 && strncmp(u, (char *)lp - l, l) == 0)
        {
            // identificator iz tablicy ne preobr. w zaglawnye!!!
            // poetomu w m.o. imja d.b. napisano zaglawnymi!
            p = refal.preva->next;
            p->tag = TAGF;
            p->info.codef = func_f[k];
            if (p->next != refal.nexta)
                rfdel(p, refal.nexta);
            rftpl(refal.prevr, p->prev, p->next);
            free(u);
            return;
        }
    }
    if (func_n == 0)
        func_f = (adr *)malloc(sizeof(adr));
    else
        func_f = (adr *)realloc(func_f, (func_n + 1) * sizeof(adr));
    if (func_f == NULL)
        rfabe("chartof: malloc or realloc error");
    func_f[func_n] = j;
    func_n++;
    p = refal.preva->next;
    p->tag = TAGF;
    p->info.codef = j;
    if (p->next != refal.nexta)
        rfdel(p, refal.nexta);
    rftpl(refal.prevr, p->prev, p->next);
    return;
}
char chartof_0[] = {Z7 'C', 'H', 'A', 'R', 'T', 'O', 'F', (char)7};
G_L_B uint8_t refalab_chartof = '\122';
void (*chartof_1)(void) = chartof_;

//------------------ end of file  XCF.C ----------------
