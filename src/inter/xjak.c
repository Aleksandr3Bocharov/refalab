//-------------- file -- XJAK.C ------------ 
//       MO: br, dg, dgall, rp, cp           
//           gtr, rdr, wtr, swr, new         
//      Last edition date : 11.07.2024       
//------------------------------------------ 
#include <stdio.h>
#include <stdbool.h>
#include "refal.def"
#include "rfintf.h"

#define N_SWAP 0116

static bool enter(bool emp, linkcb **pp, linkcb **rp)
{
    linkcb *r = refal.preva->next;
    if (r == refal.nexta)
        return false;
    if (emp && (r->next != refal.nexta))
        return true;
    linkcb *p;
    if (r->tag == TAGR)
        p = r->info.codep;
    else if (r->tag == TAGF)
    {
        const unsigned char *q = r->info.codef;
        if (*q != N_SWAP)
            return false;
        q++;
        p = (linkcb *)q; //eg 
        if (p->prev == NULL)
        {
            p->prev = p->next = p;
            p->info.codep = refal.svar;
            p->tag = 0;
            refal.svar = p;
        }
    }
    else
        return false;
    *pp = p;
    *rp = r;
    return true;
}

static void br_()
{
    const st *ast = refal.currst;
    const linkcb *p = refal.preva;
    while ((p->tag != TAGO) || (p->info.infoc != '='))
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
    linkcb *pl = ast->store->next;
    linkcb *pr = pl->next;
    pl->info.codep = pr;
    pl->tag = TAGLB;
    pr->info.codep = pl;
    pr->tag = TAGRB;
    rftpl(pl, refal.preva, refal.nexta);
}
static char br_0[] = {Z2 'B', 'R', '\002'};
G_L_B char br = '\122';
static void (*br_1)() = br_;

static void dg_()
{
    const st *ast = refal.currst;
    linkcb *pr = ast->store;
DG1:
    linkcb *pl = pr->next;
    if (pl == ast->store)
        return;
    if (pl->tag != TAGLB)
    {
        refal.upshot = 2;
        return;
    }; // FAIL  
    pr = pl->info.codep;
    linkcb *q;
    if ((q = lldupl(refal.preva, refal.nexta, pl)) == 0)
        goto DG1;
    if ((q->tag != TAGO) || (q->info.infoc != '='))
        goto DG1;
    rftpl(refal.prevr, q, pr);
    pl = pl->prev;
    pr = pr->next;
    rfdel(pl, pr);
}
static char dg_0[] = {Z2 'D', 'G', '\002'};
G_L_B char dg = '\122';
static void (*dg_1)() = dg_;

static void dgall_()
{
    const st *ast = refal.currst;
    if (refal.preva->next != refal.nexta)
        refal.upshot = 2; // FAIL  
    else
        rftpl(refal.prevr, ast->store, ast->store);
}
static char dgal_0[] = {Z5 'D', 'G', 'A', 'L', 'L', '\005'};
G_L_B char dgall = '\122';
static void (*dgal_1)() = dgall_;

static void gtr_()
{
    const bool emp = true;
    linkcb *p, *r;
    if (!enter(emp, &p, &r))
    {
        refal.upshot = 2;
        return;
    }; // FAIL  
    rftpl(refal.prevr, p, p);
}
static char gtr_0[] = {Z3 'G', 'T', 'R', '\003'};
G_L_B char gtr = '\122';
static void (*gtr_1)() = gtr_;

static void rdr_()
{
    const bool emp = true;
    linkcb *p, *r;
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
}
static char rdr_0[] = {Z3 'R', 'D', 'R', '\003'};
G_L_B char rdr = '\122';
static void (*rdr_1)() = rdr_;

static void ptr_()
{
    const bool emp = false;
    linkcb *p, *r;
    if (!enter(emp, &p, &r))
    {
        refal.upshot = 2;
        return;
    }; // FAIL  
    linkcb *q = p->prev;
    rftpl(q, r, refal.nexta);
}
// BLF  
#ifdef UNIX
static char ptr_0[] = {Z3 'P', 'T', '_', '\003'};
G_L_B char pt_ = '\122';
static void (*ptr_1)() = ptr_;
#else
static char ptr_0[] = {Z3 'P', 'T', 'R', '\003'};
G_L_B char ptr = '\122';
static void (*ptr_1)() = ptr_;
#endif

static void wtr_()
{
    const bool emp = false;
    linkcb *p, *r;
    if (!enter(emp, &p, &r))
    {
        refal.upshot = 2;
        return;
    }; // FAIL  
    rfdel(p, p);
    rftpl(p, r, refal.nexta);
}
static char wtr_0[] = {Z3 'W', 'T', 'R', '\003'};
G_L_B char wtr = '\122';
static void (*wtr_1)() = wtr_;

static void swr_()
{
    const bool emp = false;
    linkcb *p, *r;
    if (!enter(emp, &p, &r))
    {
        refal.upshot = 2;
        return;
    }; // FAIL  
    rftpl(refal.prevr, p, p);
    rftpl(p, r, refal.nexta);
}
static char swr_0[] = {Z3 'S', 'W', 'R', '\003'};
G_L_B char swr = '\122';
static void (*swr_1)() = swr_;

static void rp_()
{
    const st *ast = refal.currst;
    linkcb *p = refal.preva;
    while ((p->tag != TAGO) || (p->info.infoc != '='))
    {
        p = p->next;
        if (p == refal.nexta)
            goto FAIL;
    };
    linkcb *pr = ast->store;
RP1:
    linkcb *pl = pr->next;
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
            goto FAIL;
        pr = pl->info.codep;
        linkcb *q;
        if ((q = lldupl(refal.preva, p, pl)) == 0)
            goto RP1;
        if ((q->tag != TAGO) || (q->info.infoc != '='))
            goto RP1;
        rfdel(q, pr);
        rftpl(q, p, refal.nexta);
    }
    return;
FAIL:
    refal.upshot = 2;
    return;
}

// BLF  
#ifdef UNIX
static char rp_0[] = {Z2 'R', '_', '\002'};
G_L_B char r_ = '\122';
static void (*rp_1)() = rp_;
#else
static char rp_0[] = {Z2 'R', 'P', '\002'};
G_L_B char rp = '\122';
static void (*rp_1)() = rp_;
#endif

static void cp_()
{
    const st *ast = refal.currst;
    const linkcb *pr = ast->store;
CP1:
    const linkcb *pl = pr->next;
    if (pl == ast->store)
        return;
    if (pl->tag != TAGLB)
    {
        refal.upshot = 2;
        return;
    }; // FAIL  
    pr = pl->info.codep;
    const linkcb *q;
    if ((q = lldupl(refal.preva, refal.nexta, pl)) == 0)
        goto CP1;
    if ((q->tag != TAGO) || (q->info.infoc != '='))
        goto CP1;
    if (!lcopy(refal.prevr, q, pr))
        refal.upshot = 3; // LACK  
}
static char cp_0[] = {Z2 'C', 'P', '\002'};
G_L_B char cp = '\122';
static void (*cp_1)() = cp_;

static void new_()
{
    if (!lins(refal.prevr, 1))
    {
        refal.upshot = 3;
        return;
    }; // LACK  
    linkcb *r = refal.prevr->next;
    r->info.codep = refal.preva;
    r->tag = TAGR;
    linkcb *p = refal.nexta->prev;
    p->next = refal.preva;
    refal.preva->prev = p;
    refal.nextr->next = refal.nexta;
    refal.nexta->prev = refal.nextr;
    refal.preva->info.codep = refal.dvar;
    refal.preva->tag = 0;
    refal.dvar = refal.preva;
}
static char new_0[] = {Z3 'N', 'E', 'W', '\003'};
G_L_B char new = '\122';
static void (*new_1)() = new_;

//----------------- end of file  XJAK.C ---------------- 
