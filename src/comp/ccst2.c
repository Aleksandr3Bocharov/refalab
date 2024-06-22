/*-------------  file  CCST2.C  ------------*/
/*     The second of two compiler files     */
/*        Last edition date : 14.06.24      */
/*------------------------------------------*/
#include <stdio.h>
#include "refal.def"
#include "ccst2.h"
#include "cj.h"
#include "cgop.h"
#include "cerr.h"

/* the table of assembly language statement codes */
#define n_sb1b2 '\003'
#define n_lb '\014'
#define n_lby '\015'
#define n_rb '\016'
#define n_rby '\017'
#define n_plv '\036'
#define n_prv '\041'
#define n_plespc '\067'
#define n_lespc '\070'
#define n_prespc '\071'
#define n_respc '\072'

struct linkt
{ /*   bylo opisanie w ccst1 */
    unsigned short int tag;
    union
    {
        char *pinf;
        /*     unsigned long int   intinf;*/
        char chinf[2];
    } info;
};

extern struct
{ /* left part buffer elements */
    unsigned short int p, q, t, i;
    struct linkt code;
    unsigned short int next;
    unsigned short int pair;
    struct linkt spec;
    unsigned short int v;
    unsigned short int eoemrk;
    unsigned short int e_level;
} x[100];

extern struct
{ /* variable table elements */
    unsigned short int _t, _q;
    unsigned short int rem;
    unsigned short int last;
    char ci;
    unsigned short int _v;
} v[30];

extern struct
{ /* whole list */
    unsigned short int _next;
    unsigned short int n1;
    unsigned short int n2;
} h[50];

extern struct
{ /* compiling   options */
    unsigned source : 1;
    unsigned xref : 1;
    unsigned stmnmb : 1;
    unsigned extname : 1;
    unsigned pgtitle : 1;
    unsigned asmb : 1;
    unsigned names : 1;
} options;

extern struct
{ /* the table for corresponding with scanner */
    int nomkar;
    char modname_var[40];
    int modnmlen;
    int curr_stmnmb;
} scn_;

extern struct
{ /* the element of sentence */
    unsigned short int t_;
    char ci_;
    int v_;
    struct linkt _code;
    struct linkt _spec;
} scn_e;

extern unsigned short int t_sc;
extern unsigned short int t_lb;
extern unsigned short int t_rb;
extern unsigned short int t_s;
extern unsigned short int t_w;
extern unsigned short int t_e;
extern unsigned short int t_k;
extern unsigned short int t_p;

extern struct linkt xncode;  /* work structure */
extern struct linkt funcptr; /* work pointer */

extern unsigned short int n, n1, n2;     /* left part element pointers */
extern unsigned short int i, ie;         /* element index */
extern unsigned short int nel;           /* current element number */
extern unsigned short int lastb, lastb1; /* variables for brackets linkage  */
extern unsigned short int e_level;       /* counter of the longing levels */
extern unsigned short int not_nil;       /* working variables */
extern unsigned short int k;
extern unsigned short int nh;         /* current whole number */
extern unsigned short int next_nh;    /* next whole number */
extern unsigned short int kol_per;    /* subprogram of search in variable table */
                                      /* table pointer */
extern unsigned short int nh_x, nh_y; /* hole numbers (under enter in brackets)  */
extern unsigned short int lrbxy;      /* stoped bracket flag */

static int ortgn(unsigned short int n1, unsigned short int n2);

extern void isk_v()
{
    for (i = 1; i <= kol_per; i++)
        if (v[i].ci == scn_e.ci_)
            return;
    i = ++kol_per;
    v[i].ci = scn_e.ci_;
    v[i]._t = 0;
    v[i].rem = 1;
    v[i].last = 0;
    v[i]._v = scn_e.v_;
}

/*   generation of stoped brackets and setting boards   */
extern void gen_bsb()
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

extern void pch303()
{
    pchosa("303 differents for variable ", v[i].ci);
}

extern void pch406()
{
    pchosa("406 in left part missing variable ", v[i].ci);
}

/*    attempt to extract left support group     */
extern int lsg_p()
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
    if ((x[n].spec.info.pinf != NULL) || (v[i].rem != 1))
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
    if (x[n].spec.info.pinf == NULL)
        return 1;
    gpev(n_plespc, n_plv);
    gopl(n_lespc, x[n].spec.info.pinf);
    return 0;
}

/*        attempt to extract right support group     */
extern int rsg_p()
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
    if ((x[n].spec.info.pinf != NULL) ||
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
    if (x[n].spec.info.pinf == NULL)
        return 1;
    gpev(n_prespc, n_prv);
    gopl(n_respc, x[n].spec.info.pinf);
    return 0;
}

/*    check ortogonality of this sentence against left part */

static int ortgn(unsigned short int n1, unsigned short int n2)
{
    unsigned short int n;
    unsigned short int i;
    int res;
    n = n1;
ORT1:
    n++;
    if (n == n2)
        goto ORT1E;
    if (x[n].t <= 3)
        goto ORT1;
ORT1V:
    i = x[n].i;
    if (v[i].last != 0)
        goto ORT1;
    (v[i].rem)--;
    goto ORT1;
ORT1E:
    res = 1;
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
    res = 0;
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

extern void gpev(char op1, char op2)
{
    if (not_nil)
        jbyte(op2);
    else
        jbyte(op1);
}
/*--------  end  of  file  CCST2.C  --------*/
