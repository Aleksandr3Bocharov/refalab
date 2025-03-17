// Copyright 2025 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2025-03-17
// https://github.com/Aleksandr3Bocharov/RefalAB

//-------------  file  --  CS.C  ---------------
//         Work with identifier table
//----------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "refal.def"
#include "cs.h"
#include "cerr.h"
#include "cj.h"
#include "cgop.h"
#include "clu.h"
#include "refal.h"
#include "ccst.h"

typedef struct arr_lbl
{
    struct arr_lbl *nextl;
    T_I_LBL lbl[16];
} T_ARR_LBL;

static T_ARR_LBL *first_arr_lbl = NULL;
static size_t n_lbl = 15;
static T_I_LBL *pfail = NULL;    // statememt FAIL label
static T_I_LBL *next_stm = NULL; // next statement label
// static T_I_LBL *next_nos = NULL; // next halfword label with
//  a number of statements
static void func_end(void);
static void fnhead(const char *idp, size_t lid);

static void p504(const char *idp, size_t lid)
{
    pchosj("504 label", idp, lid, " is already defined");
    return;
}

static void p505(const char *idp, size_t lid)
{
    pchosj("505 label", idp, lid, " is yet not defined");
    return;
}

static void p500(void)
{
    pchosh("500 no statement label");
    return;
}

static T_I_LBL *alloc_lbl(void)
{
    T_ARR_LBL *q;
    if (n_lbl == 15)
    {
        q = calloc(1, sizeof(T_ARR_LBL));
#ifdef mdebug
        printf("calloc(cs)_lbl: q=%p\n", (void *)q);
#endif
        if (q == NULL)
            uns_sto();
        else
        {
            q->nextl = first_arr_lbl;
            first_arr_lbl = q;
            n_lbl = (size_t)-1;
        }
    }
    ++n_lbl;
    T_I_LBL *p = &first_arr_lbl->lbl[n_lbl];
    p->model = '\000';
    return p;
}

T_I_LBL *genlbl(void)
{
    T_I_LBL *p = alloc_lbl();
    return p;
}

void fndef(const char *idp, size_t lid)
{
    if (lid != 0)
    { // new function
        func_end();
        T_U *p = lookup(idp, lid);
        // scn_.curr_stmnmb = 0;
        next_stm = alloc_lbl();
        p->type = (p->type) | '\100';
        if ((p->mode) & '\020')
            p504(idp, lid);
        else
        {
            fnhead(idp, lid);
            p->def = scn_.nomkar;
            jlabel(p);
            /*if (options.stmnmb)
            {
                next_nos = alloc_lbl();
                gopl(n_setnos, (uint8_t *)next_nos);
            };*/
            gopl(n_sjump, (uint8_t *)next_stm);
        }
    }
    else
    { //  next statement in function
        if (next_stm != NULL)
            jlabel((T_U *)next_stm);
        else
            p500();
        next_stm = alloc_lbl();
        gopl(n_sjump, (uint8_t *)next_stm);
    };
    return;
}

static void func_end(void)
{
    if (next_stm != NULL)
    {
        if (pfail != NULL)
            jequ((T_U *)next_stm, (T_U *)pfail);
        else
        {
            pfail = next_stm;
            jlabel((T_U *)next_stm);
            jbyte(n_fail);
        }
        next_stm = NULL;
        /*if (options.stmnmb)
        {
            jlabel((T_U *)next_nos);
            ghw(scn_.curr_stmnmb);
        }*/
    }
    return;
}

void sempty(const char *idp, size_t lid)
{
    T_U *p = lookup(idp, lid);
    p->type = (p->type) | '\100';
    if (p->mode & '\020')
        p504(idp, lid);
    else
    {
        fnhead(idp, lid);
        p->def = scn_.nomkar;
        jlabel(p);
        jbyte(n_fail);
    }
    return;
}

void sswap(const char *idp, size_t lid)
{
    T_U *p = lookup(idp, lid);
    p->type = p->type | '\100';
    if (p->mode & '\020')
        p504(idp, lid);
    else
    { //  align box head on the 8-byte board
        size_t j0 = jwhere();
        size_t l0;
        if (options.extname)
            l0 = 255 > scn_.modnmlen + lid + 1 ? scn_.modnmlen + lid + 1 : 255;
        else
            l0 = lid;
        j0 = (j0 + l0 + 2) % 8;
        if (j0 != 0)
            j0 = 8 - j0;
        for (size_t k0 = 1; k0 <= j0; k0++)
            jbyte(' ');
        fnhead(idp, lid);
        p->def = scn_.nomkar;
        jlabel(p);
        jbyte(n_swap);
        const size_t kk = SMBL + LBLL * 2;
        for (size_t k0 = 1; k0 <= kk; k0++)
            jbyte('\000');
    }
    return;
}

void sentry(const char *idp, size_t lidp, const char *ide, size_t lide)
{
    T_U *p = lookup(idp, lidp);
    jentry(p, ide, lide);
    return;
}

void sextrn(const char *idp, size_t lidp, const char *ide, size_t lide)
// idp internal name
// ide external name
{
    T_U *p = lookup(idp, lidp);
    if (p->mode & '\020')
        p504(idp, lidp);
    else
    {
        p->def = scn_.nomkar;
        jextrn(p, ide, lide);
    }
    return;
}

T_U *fnref(const char *idp, size_t lid)
{
    T_U *p = lookup(idp, lid);
    p->type |= '\100';
    return p;
}

T_U *spref(const char *idp, size_t lid, char d)
{
    T_U *p = lookup(idp, lid);
    p->type |= '\200';
    if (d != ')' && (p->mode & '\020') != '\020')
        p505(idp, lid);
    return p;
}

void spdef(const char *idp, size_t lid)
{
    if (lid == 0)
        p500();
    else
    { // label exist
        T_U *p = lookup(idp, lid);
        p->type |= '\200';
        if (p->mode & '\020')
            p504(idp, lid);
        else
        {
            p->def = scn_.nomkar;
            jlabel(p);
        }
    }
    return;
}

void sequ(const char *id1, size_t lid1, const char *id0, size_t lid0)
{
    T_U *p0 = lookup(id0, lid0);
    if (lid1 == 0)
    {
        p500();
        return;
    }
    T_U *p1 = lookup(id1, lid1);
    if (p0 == p1)
        return;
    if ((p1->mode & '\300') == '\000')
    {
        p0->type |= p1->type;
        p1->def = scn_.nomkar;
        jequ(p1, p0);
    }
    else if ((p0->mode & '\300') == '\000')
    {
        p1->type |= p0->type;
        p0->def = scn_.nomkar;
        jequ(p0, p1);
    }
    else
        pchosh("501 both labels already defined ");
    return;
}

static void fnhead(const char *idp, size_t lid)
{
    if (options.names)
    {
        size_t k0, l0, ll;
        if (options.extname)
        {
            const char *idpm = scn_.modname_var;
            l0 = scn_.modnmlen;
            for (k0 = 0; k0 < l0; k0++)
                jbyte(*(idpm + k0));
            jbyte(':');
            ll = k0 + 1;
        }
        else
            ll = 0;
        l0 = lid;
        for (k0 = 0; k0 < l0; k0++)
            jbyte(*(idp + k0));
        jbyte((char)(255 < ll + l0 ? 255 : ll + l0));
    }
    else
        jbyte('\0');
    return;
}

static void check_id(const T_U *pp) // check identifier attributes on confirmness
{
    const T_U *q = pp;
    while ((q->mode & '\300') == '\300')
        q = q->info.infop;
    if ((pp->mode & '\300') == '\000')
        pchosj("512 label", pp->id, pp->l, " not defined");
    if ((pp->mode & '\040') == '\040' && (pp->mode & '\300') == '\200')
        pchosj("511 label", pp->id, pp->l, " both extern and entry");
    if ((q->mode & '\300') == '\300')
        pchosj("502 label", pp->id, pp->l, " boht specifier and function");
    return;
}

void s_end(void)
{
    func_end();
    through(check_id);
    return;
}

void s_init(void)
{ // module initiation
    first_arr_lbl = NULL;
    n_lbl = 15;
    pfail = NULL;
    next_stm = NULL;
    return;
}

void s_term(void)
{ // module termination
    T_ARR_LBL *p = first_arr_lbl;
    while (p != NULL)
    {
        T_ARR_LBL *p1 = p->nextl;
#ifdef mdebug
        printf("free(cs): p=%p\n", (void *)p);
#endif
        free(p);
        p = p1;
    }
    luterm();
    return;
}

//----------  end of file CS.C  -------------------
