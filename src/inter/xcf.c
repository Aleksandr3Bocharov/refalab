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

static uint16_t func_n = 0;
static adr *func_f = NULL;

static void ftochar_()
{
    T_LINKCB *p = refal.preva->next;
    if (p->tag != TAGF)
        goto HEOT;
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
HEOT:
    printf("\nFtochar: format error");
    refal.upshot = 2;
    return;
}
static char ftochar_0[] = {Z7 'F', 'T', 'O', 'C', 'H', 'A', 'R', '\007'};
G_L_B char ftochar = '\122';
static void (*ftochar_1)() = ftochar_;

static void functab_()
{
    const T_LINKCB *p = refal.preva->next;
    if (p->tag != TAGF)
        goto HEOT;
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
HEOT:
    printf("\nFunctab: format error");
    refal.upshot = 2;
    return;
}
static char functab_0[] = {Z7 'F', 'U', 'N', 'C', 'T', 'A', 'B', '\007'};
G_L_B char functab = '\122';
static void (*functab_1)() = functab_;

static void chartof_()
{
    T_LINKCB *p = refal.preva->next;
    if (p == refal.nexta)
        goto HEOT;
    unsigned int i;
    for (i = 0; p != refal.nexta; i++, p = p->next)
        if (p->tag != TAGO)
            goto HEOT;
    if (i > 255)
        goto HEOT;
    p = refal.preva->next;
    char *u = (char *)malloc(i + 2);
    for (i = 0; p != refal.nexta; i++, p = p->next)
        u[i] = rfcnv(p->info.infoc);
    u[i] = i;
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
        if ((i == d.w + 1) && (strncmp(u, func_f[k] - (d.w + 1), d.w) == 0))
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
HEOT:
    printf("\nChartof: format error");
    refal.upshot = 2;
    return;
}
static char chartof_0[] = {Z7 'C', 'H', 'A', 'R', 'T', 'O', 'F', '\007'};
G_L_B char chartof = '\122';
static void (*chartof_1)() = chartof_;

//------------------ end of file  XCF.C ---------------- 
