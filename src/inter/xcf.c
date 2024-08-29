//------------ file -- XCF.C ---------------
//   MO: ftochar, rftime, functab, chartof
//       Last edition date : 11.07.24
//------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "refal.def"
#include "rfintf.h"

typedef char *adr;

static size_t func_n = 0;
static adr *func_f = NULL;

static void ftochar_(void)
{
    T_LINKCB *p = refal.preva->next;
    if (p->tag != TAGF)
    {
        printf("\nFtochar: format error");
        refal.upshot = 2;
        return;
    }
    const char *u = (char *)p->info.codef - 1;
    union
    {
        char b[2];
        uint16_t w;
    } d;
    d.b[0] = *u;
    d.b[1] = 0; // d.w - dlina
    u -= d.w;
    p = refal.prevr;
    if (!lrqlk(d.w))
        if (!lincrm())
        {
            rfdel(refal.prevr, refal.nextr);
            refal.upshot = 3;
            return;
        }
    lins(p, d.w);
    for (size_t i = 0; i < d.w; i++)
    {
        p = p->next;
        p->tag = TAGO;
        p->info.codep = NULL;
        p->info.infoc = *(u + i);
    }
    return;
}
void (*ftochar_1)(void) = ftochar_;
G_L_B char ftochar = '\122';
char ftochar_0[] = {Z7 'F', 'T', 'O', 'C', 'H', 'A', 'R', '\007'};

static void functab_(void)
{
    const T_LINKCB *p = refal.preva->next;
    if (p->tag != TAGF)
    {
        printf("\nFunctab: format error");
        refal.upshot = 2;
        return;
    }
    char *u = (char *)p->info.codef;
    for (size_t i = 0; i < func_n; i++)
        if (u == func_f[i])
            return;
    if (func_n == 0)
        func_f = (adr *)malloc(sizeof(adr));
    else
        func_f = (adr *)realloc(func_f, (func_n + 1) * sizeof(adr));
    func_f[func_n] = u;
    func_n++;
    return;
}
void (*functab_1)(void) = functab_;
G_L_B char functab = '\122';
char functab_0[] = {Z7 'F', 'U', 'N', 'C', 'T', 'A', 'B', '\007'};

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
        printf("\nChartof: format error");
        refal.upshot = 2;
        return;
    }
    p = refal.preva->next;
    char *u = (char *)malloc(i + 2);
    for (i = 0; p != refal.nexta; i++, p = p->next)
        u[i] = rfcnv(p->info.infoc);
    u[i] = (char)i;
    ++i;
    u[i] = 2; // HEOT
    char *j = u + i;
    union
    {
        char b[2];
        uint16_t w;
    } d;
    d.b[1] = 0;
    for (size_t k = 0; k < func_n; k++)
    {
        d.b[0] = *(func_f[k] - 1);
        if (i == (size_t)d.w + 1 && strncmp(u, func_f[k] - (d.w + 1), d.w) == 0)
        {
            // identificator iz tablicy ne preobr. w zaglawnye!!!
            // poetomu w m.o. imja d.b. napisano zaglawnymi!
            p = refal.preva->next;
            p->tag = TAGF;
            p->info.codef = (uint8_t *)func_f[k];
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
    func_f[func_n] = j;
    func_n++;
    p = refal.preva->next;
    p->tag = TAGF;
    p->info.codef = (uint8_t *)j;
    if (p->next != refal.nexta)
        rfdel(p, refal.nexta);
    rftpl(refal.prevr, p->prev, p->next);
    return;
}
void (*chartof_1)(void) = chartof_;
G_L_B char chartof = '\122';
char chartof_0[] = {Z7 'C', 'H', 'A', 'R', 'T', 'O', 'F', '\007'};

//------------------ end of file  XCF.C ----------------
