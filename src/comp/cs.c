/*-------------  file  --  CS.C  ---------------*/
/*         Work with identifier table           */
/*        Last edition date : 29.10.86          */
/*----------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include "refal.def"
#include "cs.h"
#include "cerr.h"
#include "cj.h"
#include "cgop.h"
#include "clu.h"

#define N_FAIL '\002'
#define N_NIL '\030'
#define N_SJUMP '\001'
#define N_SWAP '\116'
#define N_SETNOS '\121'

struct refw
{
   struct refw *next;
   int numb[6];
};

struct u
{ /* for comment see file named CLU.C */
   union
   {
      int infon;
      struct u *infop;
   } info;
   char mode;
   char type;
   int l;
   struct u *i;
   struct u *j;
   struct refw *last_ref;
   struct refw ref;
   int def;
   char k;
   char *id;
};

struct i_lbl
{
   union
   {
      int infoln;
      struct i_lbl *infolp;
   } infol;
   char model;
};

struct arr_lbl
{
   struct arr_lbl *nextl;
   struct i_lbl lbl[16];
};
typedef struct arr_lbl T_ARR_LBL;

extern struct
{
   unsigned source : 1;
   unsigned xref : 1;
   unsigned stmnmb : 1;
   unsigned extname : 1;
   unsigned multmod : 1;
   unsigned asmb : 1;
   unsigned names : 1;
} options;

extern struct
{
   int nomkar;
   char modname_var[40];
   int modnmlen;
   int curr_stmnmb;
} scn_;

static struct arr_lbl *first_arr_lbl = NULL;
static int n_lbl = 15;
static struct i_lbl *pfail = NULL;    /* statememt FAIL label */
static struct i_lbl *next_stm = NULL; /* next statement label */
static struct i_lbl *next_nos = NULL; /* next halfword label with  */
                                      /* a number of statements    */
void func_end();
void fnhead(char *idp, int lid);
#define gop(n) jbyte(n)
unsigned jwhere();
void luterm();
void uns_sto();

static void p504(char *idp, int lid)
{
   pchosj("504 label", idp, lid, " is already defined");
   return; /* eg */
}

static void p505(char *idp, int lid)
{
   pchosj("505 label", idp, lid, " is yet not defined");
   return; /* eg */
}

static void p500()
{
   pchosh("500 no statement label");
   return; /* eg */
}

struct i_lbl *alloc_lbl()
{
   struct arr_lbl *q;
   struct i_lbl *p;
   if (n_lbl == 15)
   {
      q = calloc(1, sizeof(T_ARR_LBL));
#ifdef mdebug
      printf("\ncalloc(cs)_lbl: q=%lx", q);
#endif
      if (q == NULL)
         uns_sto();
      q->nextl = first_arr_lbl;
      first_arr_lbl = q;
      n_lbl = -1;
   }
   n_lbl = n_lbl + 1;
   p = &(first_arr_lbl->lbl[n_lbl]);
   p->model = '\000';
   return p;
}

struct i_lbl *genlbl()
{
   struct i_lbl *p;
   p = alloc_lbl();
   return p;
}

void fndef(char *idp, int lid)
{
   struct u *p;
   if (lid != 0)
   { /* new function */
      func_end();
      p = lookup(idp, lid);
      scn_.curr_stmnmb = 0;
      next_stm = alloc_lbl();
      p->type = (p->type) | '\100';
      if ((p->mode) & '\020')
         p504(idp, lid);
      else
      {
         fnhead(idp, lid);
         p->def = scn_.nomkar;
         jlabel(p);
         if (options.stmnmb == 1)
         {
            next_nos = alloc_lbl();
            gopl(N_SETNOS, next_nos);
         };
         gopl(N_SJUMP, next_stm);
      }
   }
   else
   { /*  next statement in function   */
      if (next_stm != NULL)
         jlabel(next_stm);
      else
         p500();
      next_stm = alloc_lbl();
      gopl(N_SJUMP, next_stm);
   };
   return; /*  eg */
}

void func_end()
{
   if (next_stm != NULL)
   {
      if (pfail != NULL)
         jequ(next_stm, pfail);
      else
      {
         pfail = next_stm;
         jlabel(next_stm);
         gop(N_FAIL);
      }
      next_stm = NULL;
      if (options.stmnmb == 1)
      {
         jlabel(next_nos);
         ghw(scn_.curr_stmnmb);
      }
   }
   return;
}

void sempty(char *idp, int lid)
{
   struct u *p;
   p = lookup(idp, lid);
   p->type = (p->type) | '\100';
   if (p->mode & '\020')
      p504(idp, lid);
   else
   {
      fnhead(idp, lid);
      p->def = scn_.nomkar;
      jlabel(p);
      gop(N_FAIL);
   }
   return;
}

void sswap(char *idp, int lid)
{
   struct u *p;
   int l0, j0, k0, kk;
   p = lookup(idp, lid);
   p->type = (p->type) | '\100';
   if (p->mode & '\020')
      p504(idp, lid);
   else
   { /*  align box head on the word board */
      j0 = jwhere();
      if (options.extname == 1)
         l0 = 255 > (scn_.modnmlen + lid + 1) ? (scn_.modnmlen + lid + 1) : 255;
      else
         l0 = lid;
      j0 = (j0 + l0 + 2) % 4;
      if (j0 != 0)
         j0 = 4 - j0;
      for (k0 = 1; k0 <= j0; k0++)
         jbyte(' ');
      fnhead(idp, lid);
      p->def = scn_.nomkar;
      jlabel(p);
      gop(N_SWAP);
      /*   kk = sizeof(int)+sizeof(long)+sizeof(POINTER) * 2;  */
      kk = SMBL + LBLL * 2;
      for (k0 = 1; k0 <= kk; k0++)
         jbyte('\000');
   }
   return;
}

void sentry(char *idp, int lidp, char *ide, int lide)
{
   struct u *p;
   p = lookup(idp, lidp);
   jentry(p, ide, lide);
   return; /* eg */
}

void sextrn(char *idp, int lidp, char *ide, int lide)
/* idp internal name */
/* ide external name */
{
   /*  int ind; */ /* eg */
   struct u *p;
   p = lookup(idp, lidp);
   if ((p->mode) & '\020')
      p504(idp, lidp);
   else
   {
      p->def = scn_.nomkar;
      jextrn(p, ide, lide);
   }
   return; /*  eg */
}

struct u *fnref(char *idp, int lid)
{
   struct u *p;
   p = lookup(idp, lid);
   p->type = (p->type) | '\100';
   return (p);
}

struct u *spref(char *idp, int lid, char d)
{
   struct u *p;
   p = lookup(idp, lid);
   p->type = (p->type) | '\200';
   if ((d != ')') && (((p->mode) & '\020') != '\020'))
      p505(idp, lid);
   return (p);
}

void spdef(char *idp, int lid)
{
   struct u *p;
   if (lid == 0)
      p500();
   else
   { /* label exist */
      p = lookup(idp, lid);
      p->type = (p->type) | '\200';
      if ((p->mode) & '\020')
         p504(idp, lid);
      else
      {
         p->def = scn_.nomkar;
         jlabel(p);
      }
   }
   return; /*  eg */
}

void sequ(char *id1, int lid1, char *id0, int lid0)
{
   struct u *p0, *p1;
   p0 = lookup(id0, lid0);
   if (lid1 == 0)
   {
      p500();
      return;
   }
   p1 = lookup(id1, lid1);
   if (p0 == p1)
      return;
   if (((p1->mode) & '\300') == '\000')
   {
      p0->type = (p0->type) | (p1->type);
      p1->def = scn_.nomkar;
      jequ(p1, p0);
   }
   else if (((p0->mode) & '\300') == '\000')
   {
      p1->type = (p1->type) | (p0->type);
      p0->def = scn_.nomkar;
      jequ(p0, p1);
   }
   else
      pchosh("501 both labels already defined ");
   return; /* eg */
}

void fnhead(char *idp, int lid)
{
   char *idpm;
   int k0, l0, ll;
   if (options.names == 1)
   {
      if (options.extname == 1)
      {
         idpm = scn_.modname_var; /* eg */
         l0 = scn_.modnmlen;
         for (k0 = 0; k0 < l0 && k0 < 8; k0++)
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
   return; /* eg */
}

void check_id(struct u *pp) /* check identifier attributes on confirmness */
{
   struct u *q;
   q = pp;
   /* printf("\nCHECK: pp=%lx q=%lx mode=%o$$$",pp,q,q->mode); */
   while (((q->mode) & '\300') == '\300')
      q = q->info.infop;
   if (((pp->mode) & '\300') == '\000')
   {
      pchosx("512 label", pp->id, pp->l, " not defined");
   }
   if ((((pp->mode) & '\040') == '\040') &&
       (((pp->mode) & '\300') == '\200'))
      pchosx("511 label", pp->id, pp->l, " both extern and entry");
   if (((q->mode) & '\300') == '\300')
      pchosx("502 label", pp->id, pp->l, " boht specifier and function");
}

void s_end()
{
   func_end();
   through(check_id);

}
void s_init()
{ /* module initiation  */
   first_arr_lbl = NULL;
   n_lbl = 15;
   pfail = NULL;
   next_stm = NULL;
   return; /* eg */
}

void s_term()
{ /* module termination */
   struct arr_lbl *p, *p1;
   p = first_arr_lbl;
   while (p != NULL)
   {
      p1 = p->nextl;
#ifdef mdebug
      printf("\nfree(cs): p=%lx", p);
#endif
      free(p);
      p = p1;
   }
   luterm();
   return; /* eg */
}
