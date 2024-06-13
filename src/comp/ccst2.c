/*-------------  file  CCST2.C  ------------*/
/*     The second of two compiler files     */
/*        Last edition date : 14.06.24      */
/*------------------------------------------*/
#include <stdio.h>
#include "ccst2.h"
#include "cj.h"
#include "cgop.h"
#include "cerr.h"

#define TAG_O 0
#define TAG_F 2
#define TAG_N 4
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

struct linkti
{ /*   bylo opisanie w ccst1 */
  short tag;
  union
  {
    char *pinf;
    /*     long   intinf;*/
    char chinf[2];
  } info;
};

extern struct
{ /* left part buffer elements */
  short p, q, t, i;
  struct linkti code;
  short next;
  short pair;
  struct linkti spec;
  short v;
  short eoemrk;
  short e_level;
} x[100];

extern struct
{ /* variable table elements */
  short _t, _q;
  short rem;
  short last;
  char ci;
  short _v;
} v[30];

extern struct
{ /* whole list */
  short _next;
  short n1;
  short n2;
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
  short t_;
  char ci_;
  int v_;
  struct linkti _code;
  struct linkti _spec;
} scn_e;

extern short t_sc;
extern short t_lb;
extern short t_rb;
extern short t_s;
extern short t_w;
extern short t_e;
extern short t_k;
extern short t_p;

extern struct linkti xncode;  /* work structure */
extern struct linkti funcptr; /* work pointer */

extern short n, n1, n2;     /* left part element pointers */
extern short i, ie;         /* element index */
extern short nel;           /* current element number */
extern short lastb, lastb1; /* variables for brackets linkage  */
extern short e_level;       /* counter of the longing levels */
extern short not_nil;       /* working variables */
extern short k;
extern short nh;         /* current whole number */
extern short next_nh;    /* next whole number */
extern short kol_per;    /* subprogram of search in variable table */
                         /* table pointer */
extern short nh_x, nh_y; /* hole numbers (under enter in brackets)  */
extern short lrbxy;      /* stoped bracket flag */
#define gop(n) jbyte(n)
int ortgn(short n1, short n2);

void isk_v()
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
    gop(n_lb);
  else if (nh == nh_y)
    gop(n_lby);
  else
  {
    gop(n_lb);
    goto GEN_SB;
  };
  return;
SW2: /* right stoped brackets */
  if (nh == nh_x)
    gop(n_rb);
  else if (nh == nh_y)
    gop(n_rby);
  else
  {
    gop(n_rb);
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
int lsg_p()
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
int rsg_p()
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

int ortgn(short n1, short n2)
{
  short n;
  short i;
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

void gpev(char op1, char op2)
{
  if (not_nil)
    gop(op2);
  else
    gop(op1);
}
/*--------  end  of  file  CCST2.C  --------*/
