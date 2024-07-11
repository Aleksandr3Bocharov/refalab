/*-------------  file  CCST2.C  ------------*/
/*     The second of two compiler files     */
/*        Last edition date : 11.07.24      */
/*------------------------------------------*/
#include <stdio.h>
#include "refal.def"
#include "ccst2.h"
#include "ccst1.h"
#include "cj.h"
#include "cgop.h"
#include "cerr.h"
#include "refal.h"

static unsigned int ortgn(unsigned short int n1, unsigned short int n2);

void isk_v()
{
    for (i = 1; i <= kol_per; i++)
        if (v[i].ci == scn_e.ci)
            return;
    i = ++kol_per;
    v[i].ci = scn_e.ci;
    v[i]._t = 0;
    v[i].rem = 1;
    v[i].last = 0;
    v[i]._v = scn_e.v;
}

/*   generation of stoped brackets and setting boards   */
void gen_bsb()
{
    switch (lrbxy)
    {
    case 0:
        goto SW0;
    case 1:
        goto SW1;
    case 2:
        goto SW2;
    };
SW0: /* no stoped brackets */
    if (nh != nh_x)
        goto GEN_SB;
    return;
SW1: /* left stoped brackets */
    if (nh == nh_x)
        jbyte(n_lb);
    else if (nh == nh_y)
        jbyte(n_lby);
    else
    {
        jbyte(n_lb);
        goto GEN_SB;
    };
    return;
SW2: /* right stoped brackets */
    if (nh == nh_x)
        jbyte(n_rb);
    else if (nh == nh_y)
        jbyte(n_rby);
    else
    {
        jbyte(n_rb);
        goto GEN_SB;
    };
    return;
GEN_SB:
    gopnm(n_sb1b2, (char)x[n1].q, (char)x[n2].p);
}

void pch303()
{
    pchosa("303 differents for variable ", v[i].ci);
}

void pch406()
{
    pchosa("406 in left part missing variable ", v[i].ci);
}

/*    attempt to extract left support group     */
unsigned int lsg_p()
{
LSG_:
    n++;
    if (n == n2)
        goto GEN_LE;
    if (x[n].t != t_lb)
        goto LSG1;
    n = x[n].pair;
    goto LSG_;
LSG1:
    if (x[n].t != t_e)
        goto LSG_;
    i = x[n].i;
    if ((i == ie) || (v[i].last != 0))
        goto LSG_;
    if ((x[n].spec.info.codef != NULL) || (v[i].rem != 1))
        goto GEN_LE;
    if (ortgn(n1, n) == 0)
        goto GEN_LE;
    x[n].eoemrk = 1;
    x[n].e_level = e_level;
GEN_LE:
    n1++;
    n = n1;
    i = x[n].i;
    v[i]._q = nel + 1;
    x[n].next = v[i].last;
    v[i].last = n;
    (v[i].rem)--;
    x[n].p = nel;
    x[n].q = nel + 1;
    nel += 2;
    e_level++;
    not_nil = x[n].v;
    if (x[n].spec.info.codef == NULL)
        return TRUE;
    gpev(n_plespc, n_plv);
    gopl(n_lespc, x[n].spec.info.codef);
    return FALSE;
}

/*        attempt to extract right support group     */
unsigned int rsg_p()
{
RSG_:
    n--;
    if (n == n1)
        goto GEN_RE;
    if (x[n].t != t_rb)
        goto RSG1;
    n = x[n].pair;
    goto RSG_;
RSG1:
    if (x[n].t != t_e)
        goto RSG_;
    i = x[n].i;
    if ((i == ie) || (v[i].last != 0))
        goto RSG_;
    if ((x[n].spec.info.codef != NULL) ||
        (v[i].rem != 1))
        goto GEN_RE;
    if (ortgn(n, n2) == 0)
        goto GEN_RE;
    x[n].eoemrk = 1;
    x[n].e_level = e_level;
GEN_RE:
    n2--;
    n = n2;
    i = x[n].i;
    v[i]._q = nel + 1;
    x[n].next = v[i].last;
    v[i].last = n;
    (v[i].rem)--;
    x[n].p = nel;
    x[n].q = nel + 1;
    nel += 2;
    e_level++;
    not_nil = x[n].v;
    if (x[n].spec.info.codef == NULL)
        return TRUE;
    gpev(n_prespc, n_prv);
    gopl(n_respc, x[n].spec.info.codef);
    return FALSE;
}

/*    check ortogonality of this sentence against left part */
static unsigned int ortgn(unsigned short int n1, unsigned short int n2)
{
    unsigned short int n = n1;
ORT1:
    n++;
    if (n == n2)
        goto ORT1E;
    if (x[n].t <= 3)
        goto ORT1;
ORT1V:
    unsigned short int i = x[n].i;
    if (v[i].last != 0)
        goto ORT1;
    (v[i].rem)--;
    goto ORT1;
ORT1E:
    unsigned int res = TRUE;
    n = n1;
ORT2:
    n++;
    if (n == n2)
        goto ORT2E;
    if (x[n].t <= 3)
        goto ORT2;
ORT2V:
    i = x[n].i;
    if (v[i].last != 0)
        goto ORT2;
    if (v[i].rem == 0)
        goto ORT2;
    res = FALSE;
ORT2E:
    n = n1;
ORT3:
    n++;
    if (n == n2)
        goto ORT3E;
    if (x[n].t <= 3)
        goto ORT3;
ORT3V:
    i = x[n].i;
    if (v[i].last != 0)
        goto ORT3;
    (v[i].rem)++;
    goto ORT3;
ORT3E:
    return res;
}

void gpev(char op1, char op2)
{
    if (not_nil)
        jbyte(op2);
    else
        jbyte(op1);
}
/*--------  end  of  file  CCST2.C  --------*/
