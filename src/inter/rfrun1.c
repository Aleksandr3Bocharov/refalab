/*----------- file RFRUN1.c -------------------*/
/*      Refal-interpretator (part 1)           */
/*      Last edition date: 19.06.2024          */
/*---------------------------------------------*/
#include <stdio.h>
#include "refal.def"

extern REFAL refal;

/* for unlooping: */

#define NMBL sizeof(char)
#define SHB1      \
   b1 = b1->next; \
   if (b1 == b2)  \
      goto FAIL;
#define SHB2      \
   b2 = b2->prev; \
   if (b2 == b1)  \
      goto FAIL;
#define SHF         \
   f = f->next;     \
   if (f == flhead) \
      goto LACK;

static linkcb *et[256]; /* element table */
static int nel;         /* adress of first free string in element table */

struct wjs
{ /* jump stack structure */
   linkcb *jsb1;
   linkcb *jsb2;
   int jsnel;
   char *jsvpc;
};
static struct wjs js[64]; /* jump stack and planning translation stack*/
static struct wjs *jsp;   /* jump stack pointer*/

struct ts
{ /* translation stack structure*/
   linkcb *ts0;
   linkcb *ts1;
   linkcb *ts2;
};
static struct ts *tsp; /*translation stack pointer*/

static int tmmod;      /* timer state */
static long tmstart;   /* time at the start */
static long tmstop;    /* time at the end    */

struct sav_
{ /* save area for var-part of REFAL-block */
   int upshot_;
   linkcb *preva_;
   linkcb *nexta_;
   linkcb *prevr_;
   linkcb *nextr_;
   st *currst_;
};
typedef struct sav_ sav;

static union
{
   char c[2];
   int ii;
} u;

static union
{ /* structure for pointer and integer aligning */
   char *ptr;
   long *inr;
   char chr[2];
} inch;

/* definition of work variables and pointers*/
static char opc;           /* current statement code */
static unsigned char *vpc; /* virtual program counter */
static linkcb *lastk;      /* last acted sign-k adress */
static linkcb *lastb;      /* last generated left bracket*/
static linkcb *b0, *b1, *b2;
static linkcb *f0, *f1, *f;
static char *vpca; /* additional vpc  */
static int i, n, m;

static char (*fptr)();

sav *malloc();
long time();

void rfrun(st *ast) /* adress of current state table */
{
   /* dynamic area DSA */
   sav *savecr;
   linkcb quasik;  /* quasi-sign k */
   linkcb *flhead; /* adress of free memory list head */
   linkcb *nextr;  /* item adress followed by result */
   savecr = malloc(sizeof(sav));
   u.ii = 0;
   if (!lexist(ast))
      rfabe("rfrun: attempt to run unexisting process");
   if (ast->state == 4)
      rfabe("rfrun: attampt to run process in state 4");

   /* saving part of refal-block */

   savecr->upshot_ = refal.upshot;
   savecr->preva_ = refal.preva;
   savecr->nexta_ = refal.nexta;
   savecr->prevr_ = refal.prevr;
   savecr->nextr_ = refal.nextr;
   savecr->currst_ = refal.currst;
   tmmod = refal.tmmode;
   refal.tmmode = 0;
   refal.currst = ast;
   ast->state = 4;
   quasik.info.codep = ast->dot;
   flhead = refal.flhead;
   if (tmmod == 1)
      tmstart = time(0l);
   goto START;
   /* interpreter exits */
   /* store state       */
DONE:
   ast->state = 1; /* normal end*/
   goto EXIT;
RCGIMP:
   ast->state = 2; /* recognition impossible */
   goto EXIT;
LACK:
   ast->state = 3; /* end of free memory*/
   quasik.info.codep = et[2];
   goto EXIT;
   /* state remove from */
EXIT:
   if (tmmod == 1)
      tmstop = time(0l);
EXIT0:
   ast->dot = quasik.info.codep;
   /* restore REFAL-block */
   refal.upshot = savecr->upshot_;
   refal.preva = savecr->preva_;
   refal.nexta = savecr->nexta_;
   refal.prevr = savecr->prevr_;
   refal.nextr = savecr->nextr_;
   refal.currst = savecr->currst_;
   refal.tmmode = tmmod;
   free(savecr);
   if (tmmod == 1)
   {
      /*printf("\nn=%ld k=%ld",tmstart,tmstop);*/
      refal.tmintv = (tmstop - tmstart) * 1000000l;
   }
   return;

   /* select next statement assembly language */
ADVANCE:
   vpc = vpc + NMBL;
NEXTOP:
   opc = *vpc;
#ifdef PRINT_CODE
   printf("\n switcher");

   /* BLF 03.07.2004 */
   printf(" code8=%o\t(D=%d,H=%x)", opc, opc, opc);
#endif

   /* operation switcher */
   /*       OPSWITCH     */
   /*  printf("\n  ego opc=%o NMBL=%o LBLL=%o SMBL=%o",opc,NMBL,LBLL,SMBL);*/

   switch (opc)
   {
   case 0000:
      goto ADVANCE;
   case 0001:
      goto SJUMP;
   case 0002:
      goto FAIL;
   case 0003:
      goto SB;
   case 0004:
      goto LSC;
   case 0005:
      goto RSC;
   case 0006:
      goto LSCO;
   case 0007:
      goto RSCO;
   case 0010:
      goto LSD;
   case 0011:
      goto RSD;
   case 0012:
      goto LTXT;
   case 0013:
      goto RTXT;
   case 0014:
      goto LB;
   case 0015:
      goto LBY;
   case 0016:
      goto RB;
   case 0017:
      goto RBY;
   case 0020:
      goto LS;
   case 0021:
      goto RS;
   case 0022:
      goto LW;
   case 0023:
      goto RW;
   case 0024:
      goto LBNIL;
   case 0025:
      goto RBNIL;
   case 0026:
      goto LBCE;
   case 0027:
      goto RBCE;
   case 0030:
      goto NIL;
   case 0031:
      goto CE;
   case 0032:
      goto LED;
   case 0033:
      goto RED;
   case 0034:
      goto NNIL;
   case 0035:
      goto PLE;
   case 0036:
      goto PLV;
   case 0037:
      goto LE;
   case 0040:
      goto PRE;
   case 0041:
      goto PRV;
   case 0042:
      goto RE;
   case 0043:
      goto PLESC;
   case 0044:
      goto PLVSC;
   case 0045:
      goto LESC;
   case 0046:
      goto PRESC;
   case 0047:
      goto PRVSC;
   case 0050:
      goto RESC;
   case 0051:
      goto LESD;
   case 0052:
      goto RESD;
   case 0053:
      goto PLEB;
   case 0054:
      goto PLVB;
   case 0055:
      goto LEB;
   case 0056:
      goto PREB;
   case 0057:
      goto PRVB;
   case 0060:
      goto REB;
   case 0061:
      goto EOE;
   case 0062:
      goto EOEI;
   case 0063:
      goto LSRCH;
   case 0064:
      goto RSRCH;
   case 0065:
      goto WSPC;
   case 0066:
      goto ESPC;
   case 0067:
      goto PLESPC;
   case 0070:
      goto LESPC;
   case 0071:
      goto PRESPC;
   case 0072:
      goto RESPC;
   case 0073:
      goto LMAX;
   case 0074:
      goto RMAX;
   case 0075:
      goto EOR;
   case 0076:
      goto NS;
   case 0077:
      goto NSO;
   case 0100:
      goto TEXT;
   case 0101:
      goto BL;
   case 0102:
      goto BR;
   case 0103:
      goto BLR;
   case 0104:
      goto BRACT;
   case 0105:
      goto ACT;
   case 0106:
      goto MULS;
   case 0107:
      goto MULE;
   case 0110:
      goto TPL;
   case 0111:
      goto TPLM;
   case 0112:
      goto TPLE;
   case 0113:
      goto TPLV;
   case 0114:
      goto TPLS;
   case 0115:
      goto EOS;
   case 0116:
      goto SWAP;
   case 0117:
      goto BLF;
   case 0120:
      goto EOSSN;
   case 0121:
      goto SETNOS;
   case 0122:
      goto CFUNC;
   };
   /* SETNOS(L);    */
SETNOS:
   move(LBLL, vpc + NMBL, &(inch.inr));
   refal.nostm = (int)*(inch.inr);
   vpc = vpc + NMBL + LBLL;
   goto NEXTOP;
   /* EOSSN (NN); */
EOSSN:
   move(NMBL + NMBL, vpc + NMBL, &(refal.stmnmb));
   /* EOS;       */
EOS:
   lastk->info.codep = et[1]->info.codep;
   lastk->tag = TAGK;
   nextr = f->next;
   /* execute planned transplantation */
   /* EOS1:    */
   while (tsp != (struct ts *)js)
   {
      tsp = tsp - 1;
      getts(tsp, &f, &f0, &f1);
      link(f0->prev, f1->next);
      link(f1, f->next);
      link(f, f0);
   }
   /* include replace result */
   /* INSRES:                */
   if (flhead->next == nextr)
      link(et[1]->prev, et[2]->next);
   else
   {
      link(nextr->prev, et[2]->next);
      link(et[1]->prev, flhead->next);
   };
   /*  delete k and  . */
   /* DELKD            */
   link(et[2], nextr);
   link(flhead, et[1]);
   /* increase step number */
ADVSTEP:
   (ast->step)++;

   /* start of next step */
START:
   if (ast->step >= ast->stop)
      goto DONE;
   b2 = quasik.info.codep;
   if (b2 == NULL)
      goto DONE;
   b0 = b2->info.codep;
   b1 = b0->next;
   vpc = b1->info.codef;
   if (b1->tag != TAGF)
      goto REF;
   /* here must be check on c-function */
   /* if (c) goto CFUNC;               */
   jsp = js;
   et[1] = b0;
   et[2] = b2;
   et[3] = b1;
   nel = 4;
   goto NEXTOP;
   /* C-refal-function execution */
CFUNC:;
   move(LBLL, vpc + NMBL + Z_0, &fptr);
   refal.upshot = 1;
   refal.prevr = b0->prev;
   refal.nextr = b0;
   refal.preva = b1;
   refal.nexta = b2;
   /*        call  C - function    */
   (*fptr)(&refal);
   switch (refal.upshot)
   {
   case 1:
      goto CFDONE;
   case 2:
      goto RCGIMP;
   case 3:
      goto CFLACK;
   default:
      goto CFDONE;
   }
   /*        return from C - function   */
   /*          step is done             */
CFDONE:
   quasik.info.codep = refal.nextr->info.codep;
   link(refal.nextr->prev, refal.nexta->next);
   link(refal.nexta, flhead->next);
   link(flhead, refal.nextr);
   goto ADVSTEP;
   /*        return from C - function      */
   /*     free memory exhausted            */
CFLACK:
   if (refal.prevr->next != refal.nextr)
   {
      link(refal.nextr->prev, flhead->next);
      link(flhead, refal.prevr->next);
      link(refal.prevr, refal.nextr);
   }
   ast->state = 3;
   goto EXIT;

   /* symbol - reference execution */
REF:
   if (b1->tag != TAGR)
      goto RCGIMP;
   et[1] = b0;
   et[2] = b2;
   et[3] = b1;
   f = (linkcb *)vpc;
   goto SWAPREF;
   /* SWAP;   */
   /*  static box head is after operator code */
SWAP:
   vpc = vpc + NMBL;
   f = (linkcb *)vpc;
   if (f->prev != NULL)
      goto SWAPREF;
   link(f, f);
   f->info.codep = refal.svar;
   f->tag = 0;
   refal.svar = f;
SWAPREF:
   quasik.info.codep = et[1]->info.codep;
   if (f->next != f)
   {
      link(f->prev, et[2]->next);
      link(et[1]->prev, f->next);
   }
   else
      link(et[1]->prev, et[2]->next);
   if (et[3]->next != et[2])
   {
      link(et[2]->prev, f);
      link(f, et[3]->next);
      link(et[3], et[2]);
   }
   else
      link(f, f);
   link(et[2], flhead->next);
   link(flhead, et[1]);
   goto ADVSTEP;
   /* LSC(S);       */
LSC:
   SHB1 if (cmpr(SMBL, vpc + NMBL, &(b1->tag)) == 0) goto FAIL;
   vpc = vpc + NMBL + SMBL;
   et[nel] = b1;
   nel++;
   goto NEXTOP;
   /* RSC(S);     */
RSC:
   SHB2 if (cmpr(SMBL, vpc + NMBL, &(b2->tag)) == 0) goto FAIL;
   vpc = vpc + NMBL + SMBL;
   et[nel] = b2;
   nel++;
   goto NEXTOP;
   /* NIL;      */
NIL:
   if (b1->next != b2)
      goto FAIL;
   goto ADVANCE;
   /* LB;       */
LB:
   SHB1 if ((b1->tag & 0001) == 0) goto FAIL;
   b2 = b1->info.codep;
   et[nel] = b1;
   et[nel + 1] = b2;
   nel = nel + 2;
   goto ADVANCE;
   /* RB;      */
RB:
   SHB2 if ((b2->tag & 0001) == 0) goto FAIL;
   b1 = b2->info.codep;
   et[nel] = b1;
   et[nel + 1] = b2;
   nel = nel + 2;
   goto ADVANCE;
   /* SB(N,M);  */
SB:
   n = (unsigned)*(vpc + NMBL);
   m = (unsigned)*(vpc + NMBL + NMBL);
   b1 = et[n];
   b2 = et[m];
   vpc = vpc + 3 * NMBL;
   goto NEXTOP;
   /* LBY;    */
LBY:
   SHB1 if ((b1->tag & 0001) == 0) goto FAIL;
   et[nel] = b1;
   b1 = b1->info.codep;
   et[nel + 1] = b1;
   nel = nel + 2;
   goto ADVANCE;
   /* RBY;    */
RBY:
   SHB2 if ((b2->tag & 0001) == 0) goto FAIL;
   et[nel + 1] = b2;
   b2 = b2->info.codep;
   et[nel] = b2;
   nel = nel + 2;
   goto ADVANCE;
   /* LBNIL  */
LBNIL:
   SHB1 if ((b1->tag & 0001) == 0) goto FAIL;
   b0 = b1;
   b1 = b1->info.codep;
   if (b0->next != b1)
      goto FAIL;
   et[nel] = b0;
   et[nel + 1] = b1;
   nel = nel + 2;
   goto ADVANCE;
   /* RBNIL; */
RBNIL:
   SHB2 if ((b2->tag & 0001) == 0) goto FAIL;
   b0 = b2;
   b2 = b2->info.codep;
   if (b2->next != b0)
      goto FAIL;
   et[nel] = b2;
   et[nel + 1] = b0;
   nel = nel + 2;
   goto ADVANCE;
   /* LSCO(N); */
LSCO:
   SHB1 if (b1->tag != TAGO) goto FAIL;
   if (b1->info.infoc != *(vpc + NMBL))
      goto FAIL;
   et[nel] = b1;
   nel++;
   vpc = vpc + NMBL + NMBL;
   goto NEXTOP;
   /* RSCO(N);  */
RSCO:
   SHB2 if (b2->tag != TAGO) goto FAIL;
   if (b2->info.infoc != *(vpc + NMBL))
      goto FAIL;
   et[nel] = b2;
   nel++;
   vpc = vpc + NMBL + NMBL;
   goto NEXTOP;
   /* LTXT(N,S1,...,SN); */
LTXT:
   n = (unsigned)*(vpc + NMBL);
   vpc = vpc + NMBL + NMBL;
LTXT1:
   SHB1 if (b1->tag != TAGO) goto FAIL;
   if (b1->info.infoc != *vpc)
      goto FAIL;
   et[nel] = b1;
   nel++;
   vpc = vpc + NMBL;
   n--;
   if (n != 0)
      goto LTXT1;
   goto NEXTOP;
   /*RTXT(N,S1,...,SN);*/
RTXT:
   n = (unsigned)*(vpc + NMBL);
   vpc = vpc + NMBL + NMBL;
RTXT1:
   SHB2 if (b2->tag != TAGO) goto FAIL;
   if (b2->info.infoc != *vpc)
      goto FAIL;
   et[nel] = b2;
   nel++;
   vpc = vpc + NMBL;
   n--;
   if (n != 0)
      goto RTXT1;
   goto NEXTOP;
   /*LS;           */
LS:
   SHB1 if ((b1->tag & 0001) != 0) goto FAIL;
   et[nel] = b1;
   nel++;
   goto ADVANCE;
   /* RS;         */
RS:
   SHB2 if ((b2->tag & 0001) != 0) goto FAIL;
   et[nel] = b2;
   nel++;
   goto ADVANCE;
   /* LSD(N);     */
LSD:
   SHB1
       n = (unsigned)*(vpc + NMBL);
   if (b1->tag != et[n]->tag)
      goto FAIL;
   if (b1->info.codef != et[n]->info.codef)
      goto FAIL;
   et[nel] = b1;
   nel++;
   vpc = vpc + NMBL + NMBL;
   goto NEXTOP;
   /* RSD(N);         */
RSD:
   SHB2
       n = (unsigned)*(vpc + NMBL);
   if (b2->tag != et[n]->tag)
      goto FAIL;
   if (b2->info.codef != et[n]->info.codef)
      goto FAIL;
   et[nel] = b2;
   nel++;
   vpc = vpc + NMBL + NMBL;
   goto NEXTOP;
   /* CE;       */
CE:
   et[nel] = b1->next;
   et[nel + 1] = b2->prev;
   nel = nel + 2;
   goto ADVANCE;
   /*LBCE;     */
LBCE:
   SHB1 if ((b1->tag & 0001) == 0) goto FAIL;
   b0 = b1;
   b1 = b1->info.codep;
   et[nel] = b0;
   et[nel + 1] = b1;
   et[nel + 2] = b0->next;
   et[nel + 3] = b1->prev;
   nel = nel + 4;
   goto ADVANCE;
   /*RBCE;    */
RBCE:
   SHB2 if ((b2->tag & 0001) == 0) goto FAIL;
   b0 = b2;
   b2 = b2->info.codep;
   et[nel] = b2;
   et[nel + 1] = b0;
   et[nel + 2] = b2->next;
   et[nel + 3] = b0->prev;
   nel = nel + 4;
   goto ADVANCE;
   /*LED(N);  */
LED:
   n = (unsigned)*(vpc + NMBL);
   et[nel] = b1->next;
   b0 = et[n - 1]->prev;
LED1:
   if (b0 == et[n])
      goto LED2;
   b0 = b0->next;
   SHB1 if (b1->tag != b0->tag) goto FAIL;
   if (b1->info.codef == b0->info.codef)
      goto LED1;
   if ((b1->tag & 0001) != 0)
      goto LED1;
   goto FAIL;
LED2:
   et[nel + 1] = b1;
   nel = nel + 2;
   vpc = vpc + NMBL * 2;
   goto NEXTOP;
   /*RED(N);   */
RED:
   n = (unsigned)*(vpc + NMBL);
   et[nel + 1] = b2->prev;
   b0 = et[n]->next;
RED1:
   if (b0 == et[n - 1])
      goto RED2;
   b0 = b0->prev;
   SHB2 if (b2->tag != b0->tag) goto FAIL;
   if (b2->info.codef == b0->info.codef)
      goto RED1;
   if ((b2->tag & 0001) != 0)
      goto RED1;
   goto FAIL;
RED2:
   et[nel] = b2;
   nel = nel + 2;
   vpc = vpc + NMBL * 2;
   goto NEXTOP;

   /*LW;    */
LW:
   SHB1
       et[nel] = b1;
   if ((b1->tag & 0001) != 0)
      b1 = b1->info.codep;
   et[nel + 1] = b1;
   nel = nel + 2;
   goto ADVANCE;
   /*RW;    */
RW:
   SHB2
       et[nel + 1] = b2;
   if ((b2->tag & 0001) != 0)
      b2 = b2->info.codep;
   et[nel] = b2;
   nel = nel + 2;
   goto ADVANCE;
   /*NNIL;   */
NNIL:
   if (et[nel - 1]->next == et[nel - 2])
      goto FAIL;
   goto ADVANCE;
   /*PLE;    */
PLE:
   vpc = vpc + NMBL;
   putjs(jsp, &b1, &b2, &nel, &vpc);
   jsp++;
   et[nel] = b1->next;
   et[nel + 1] = b1;
   nel = nel + 2;
   goto ADVANCE;
   /*PLV;     */
PLV:
   vpc = vpc + NMBL;
   putjs(jsp, &b1, &b2, &nel, &vpc);
   et[nel] = b1->next;
   et[nel + 1] = b1;
   goto NEXTOP;
   /*LE;      */
LE:
   b1 = et[nel + 1];
   SHB1 if ((b1->tag & 0001) != 0) b1 = b1->info.codep;
   jsp++;
   et[nel + 1] = b1;
   nel = nel + 2;
   goto ADVANCE;
   /*PRE;      */
PRE:
   vpc = vpc + NMBL;
   putjs(jsp, &b1, &b2, &nel, &vpc);
   jsp++;
   et[nel] = b2;
   et[nel + 1] = b2->prev;
   nel = nel + 2;
   goto ADVANCE;
   /*PRV;      */
PRV:
   vpc = vpc + NMBL;
   putjs(jsp, &b1, &b2, &nel, &vpc);
   et[nel] = b2;
   et[nel + 1] = b2->prev;
   goto NEXTOP;
   /*RE;           */
RE:
   b2 = et[nel];
   SHB2 if ((b2->tag & 0001) != 0) b2 = b2->info.codep;
   jsp++;
   et[nel] = b2;
   nel = nel + 2;
   goto ADVANCE;
   /*SJUMP(L);     */
SJUMP:
   move(LBLL, vpc + NMBL, &(inch.ptr));
   putjs(jsp, &b1, &b2, &nel, &(inch.ptr));
   jsp++;
   vpc = vpc + NMBL + LBLL;
   goto NEXTOP;
   /*FAIL;         */
FAIL:
   if (jsp == js)
      goto RCGIMP;
   jsp--;
   getjs(jsp, &b1, &b2, &nel, &vpc);
   goto NEXTOP;
   /*PLESC;         */
PLESC:
   vpc = vpc + NMBL;
   putjs(jsp, &b1, &b2, &nel, &vpc);
   et[nel] = b1->next;
   et[nel + 2] = b1;
   goto NEXTOP;
   /*PLVSC;        */
PLVSC:
   vpc = vpc + NMBL;
   putjs(jsp, &b1, &b2, &nel, &vpc);
   et[nel] = b1->next;
   SHB1 if ((b1->tag & 0001) != 0) b1 = b1->info.codep;
   et[nel + 2] = b1;
   goto NEXTOP;
   /* LESC(S);    */
LESC:
   vpc = vpc + NMBL + SMBL;
   vpca = vpc - SMBL;
LESC0:
   b1 = et[nel + 2];
LESC1:
   SHB1 if ((b1->tag & 0001) != 0)
   { /* if(BRA(B1)) */
      b1 = b1->info.codep;
      goto LESC1;
   }
   if (cmpr(SMBL, vpca, &(b1->tag)) == 0)
      goto LESC1;
   jsp++;
   et[nel + 1] = b1->prev;
   et[nel + 2] = b1;
   nel = nel + 3;
   goto NEXTOP;
   /* PRESC; */
PRESC:
   vpc = vpc + NMBL;
   putjs(jsp, &b1, &b2, &nel, &vpc);
   et[nel + 1] = b2->prev;
   et[nel + 2] = b2;
   goto NEXTOP;
   /* PRVSC; */
PRVSC:
   vpc = vpc + NMBL;
   putjs(jsp, &b1, &b2, &nel, &vpc);
   et[nel + 1] = b2->prev;
   SHB2 if ((b2->tag & 0001) != 0) b2 = b2->info.codep;
   et[nel + 2] = b2;
   goto NEXTOP;
   /* RESC(S); */
RESC:
   vpc = vpc + NMBL + SMBL;
   vpca = vpc - SMBL;
RESC0:
   b2 = et[nel + 2];
RESC1:
   SHB2 if ((b2->tag & 0001) != 0)
   { /* if(BRA(B2)) */
      b2 = b2->info.codep;
      goto RESC1;
   }
   if (cmpr(SMBL, vpca, &(b2->tag)) == 0)
      goto RESC1;
   jsp++;
   et[nel + 2] = b2;
   et[nel] = b2->next;
   nel = nel + 3;
   goto NEXTOP;
   /* LESD(N); */
LESD:
   n = (unsigned)*(vpc + NMBL);
   vpca = (char *)&(et[n]->tag);
   vpc = vpc + NMBL * 2;
   goto LESC0;
   /* RESD(N); */
RESD:
   n = (unsigned)*(vpc + NMBL);
   vpca = (char *)&(et[n]->tag);
   vpc = vpc + NMBL * 2;
   goto RESC0;
   /* PLEB; */
PLEB:
   vpc = vpc + NMBL;
   putjs(jsp, &b1, &b2, &nel, &vpc);
   et[nel] = b1->next;
   et[nel + 3] = b1;
   goto NEXTOP;
   /* PLVB; */
PLVB:
   vpc = vpc + NMBL;
   putjs(jsp, &b1, &b2, &nel, &vpc);
   et[nel] = b1->next;
   SHB1 if ((b1->tag & 0001) != 0) b1 = b1->info.codep;
   et[nel + 3] = b1;
   goto NEXTOP;
   /* LEB;  */
LEB:
   b1 = et[nel + 3];
LEB1:
   SHB1 if ((b1->tag & 0001) == 0) goto LEB1;
   jsp++;
   et[nel + 1] = b1->prev;
   et[nel + 2] = b1;
   b2 = b1->info.codep;
   et[nel + 3] = b2;
   nel = nel + 4;
   goto ADVANCE;
   /* PREB; */
PREB:
   vpc = vpc + NMBL;
   putjs(jsp, &b1, &b2, &nel, &vpc);
   et[nel + 1] = b2->prev;
   et[nel + 2] = b2;
   goto NEXTOP;
   /* PRVB; */
PRVB:
   vpc = vpc + NMBL;
   putjs(jsp, &b1, &b2, &nel, &vpc);
   et[nel + 1] = b2->prev;
   SHB2 if ((b2->tag & 0001) != 0) b2 = b2->info.codep;
   et[nel + 2] = b2;
   goto NEXTOP;
   /*  REB;    */
REB:
   b2 = et[nel + 2];
REB1:
   SHB2 if ((b2->tag & 0001) == 0) goto REB1;
   jsp++;
   et[nel] = b2->next;
   et[nel + 3] = b2;
   b1 = b2->info.codep;
   et[nel + 2] = b1;
   nel = nel + 4;
   goto ADVANCE;
   /* EOE(N);  */
EOE:
   n = (unsigned)*(vpc + NMBL);
   jsp = jsp - n;
   vpc = vpc + 2 * NMBL;
   goto NEXTOP;
   /* EOEI;   */
EOEI:
   jsp--;
   goto ADVANCE;
   /*LSRCH(S);  */
LSRCH:
   et[nel] = b1->next;
LSRCH1:
   SHB1 if ((b1->tag & 0001) != 0)
   {
      b1 = b1->info.codep;
      goto LSRCH1;
   };
   if (cmpr(SMBL, vpc + NMBL, &(b1->tag)) == 0)
      goto LSRCH1;
   et[nel + 1] = b1->prev;
   et[nel + 2] = b1;
   nel = nel + 3;
   vpc = vpc + NMBL + SMBL;
   goto NEXTOP;
   /*RSRCH(S);  */
RSRCH:
   et[nel + 1] = b2->prev;
RSRCH1:
   SHB2 if ((b2->tag & 0001) != 0)
   {
      b2 = b2->info.codep;
      goto RSRCH1;
   };
   if (cmpr(SMBL, vpc + NMBL, &(b2->tag)) == 0)
      goto RSRCH1;
   et[nel] = b2->next;
   et[nel + 2] = b2;
   nel = nel + 3;
   vpc = vpc + NMBL + SMBL;
   goto NEXTOP;
   /* WSPC(L);  */
WSPC:
   if (spc(jsp + 1, vpc, et[nel - 1]) == 0)
      goto FAIL;
   vpc = vpc + NMBL + LBLL;
   goto NEXTOP;
   /* ESPC(L); */
ESPC:
   b0 = et[nel - 2]->prev;
   while (b0 != et[nel - 1])
   {
      b0 = b0->next;
      if ((b0->tag & 0001) != 0)
         b0 = b0->info.codep;
      if (spc(jsp + 1, vpc, b0) == 0)
         goto FAIL;
   };
   vpc = vpc + NMBL + LBLL;
   goto NEXTOP;
   /*PLESPC;  */
PLESPC:
   vpc = vpc + NMBL;
   putjs(jsp, &b1, &b2, &nel, &vpc);
   jsp++;
   et[nel] = b1->next;
   et[nel + 1] = b1;
   nel = nel + 2;
   vpc = vpc + NMBL + LBLL;
   goto NEXTOP;
   /* LESPC(L); */
LESPC:
   b1 = et[nel + 1];
   SHB1 if ((b1->tag & 0001) != 0) b1 = b1->info.codep;
   if (spc(jsp + 1, vpc, b1) == 0)
      goto FAIL;
   jsp++;
   et[nel + 1] = b1;
   nel = nel + 2;
   vpc = vpc + NMBL + LBLL;
   goto NEXTOP;
   /* PRESPC; */
PRESPC:
   vpc = vpc + NMBL;
   putjs(jsp, &b1, &b2, &nel, &vpc);
   jsp++;
   et[nel + 1] = b2->prev;
   et[nel] = b2;
   nel = nel + 2;
   vpc = vpc + NMBL + LBLL;
   goto NEXTOP;
   /* RESPC(L); */
RESPC:
   b2 = et[nel];
   SHB2 if ((b2->tag & 0001) != 0) b2 = b2->info.codep;
   if (spc(jsp + 1, vpc, b2) == 0)
      goto FAIL;
   jsp++;
   et[nel] = b2;
   nel = nel + 2;
   vpc = vpc + NMBL + LBLL;
   goto NEXTOP;
   /* LMAX(L); */
LMAX:
   et[nel] = b1->next;
   while ((b1 = b1->next) != b2)
   {
      if (spc(jsp + 1, vpc, b1) == 0)
         break;
      if ((b1->tag & 0001) != 0)
         b1 = b1->info.codep;
   };
   b1 = b1->prev;
   et[nel + 1] = b1;
   nel = nel + 2;
   vpc = vpc + NMBL + LBLL;
   goto NEXTOP;
   /* RMAX(L);  */
RMAX:
   et[nel + 1] = b2->prev;
   while ((b2 = b2->prev) != b1)
   {
      if (spc(jsp + 1, vpc, b2) == 0)
         break;
      if ((b2->tag & 0001) != 0)
         b2 = b2->info.codep;
   };
   b2 = b2->next;
   et[nel] = b2;
   nel = nel + 2;
   vpc = vpc + NMBL + LBLL;
   goto NEXTOP;
   /* EOR;     */
EOR:
   f = flhead;
   lastk = &quasik;
   lastb = NULL;
   tsp = (struct ts *)js;
   goto ADVANCE;
   /* NS(S);  */
NS:
   SHF
       move(SMBL, vpc + NMBL, &(f->tag));
   vpc = vpc + NMBL + SMBL;
   goto NEXTOP;
   /* NSO(N);  */
NSO:
   SHF
       f->tag = TAGO;
   f->info.codep = NULL;
   f->info.infoc = *(vpc + 1);
   vpc = vpc + NMBL + NMBL;
   goto NEXTOP;
   /* TEXT(N,S1,S2,...,SN); */
TEXT:
   n = (unsigned)*(vpc + NMBL);
   /* printf("\n TEXT uc0=%x uii=%x  %d",u.c[0],u.ii,u.ii);*/
   vpc = vpc + NMBL + NMBL;
   for (i = 1; i <= n; i++)
   {
      SHF
          f->tag = TAGO;
      f->info.codep = NULL;
      f->info.infoc = *vpc;
      vpc = vpc + NMBL;
   };
   goto NEXTOP;
   /* BL;       */
BL:
   SHF
       f->info.codep = lastb;
   lastb = f;
   goto ADVANCE;
   /* BR;      */
BR:
   SHF
       f->info.codep = lastb;
   f1 = lastb->info.codep;
   lastb->tag = TAGLB;
   lastb->info.codep = f;
   f->tag = TAGRB;
   lastb = f1;
   goto ADVANCE;
   /* BLR;  */
BLR:
   SHF
       f1 = f;
   SHF
       f1->info.codep = f;
   f->info.codep = f1;
   f1->tag = TAGLB;
   f->tag = TAGRB;
   goto ADVANCE;
   /* BLF(L);  */
BLF:
   SHF
       f->info.codep = lastb;
   lastb = f;
   SHF
       move(LBLL, vpc + NMBL, &(f->info.codef));
   f->tag = TAGF;
   vpc = vpc + NMBL + LBLL;
   goto NEXTOP;
   /* BRACT;    */
BRACT:
   SHF
       f->info.codep = lastb;
   f->tag = TAGD;
   lastk->info.codep = f;
   lastk->tag = TAGK;
   lastk = lastb;
   lastb = lastb->info.codep;
   goto ADVANCE;
   /* ACT(N);  */
ACT:
   n = (unsigned)*(vpc + NMBL);
   lastk->info.codep = et[n];
   lastk->tag = TAGK;
   lastk = et[n]->info.codep;
   et[n]->tag = TAGD;
   vpc = vpc + NMBL + NMBL;
   goto NEXTOP;
   /* MULS;   */
MULS:
   n = (unsigned)*(vpc + NMBL);
   SHF
       move(SMBL, &(et[n]->tag), &(f->tag));
   vpc = vpc + NMBL + NMBL;
   goto NEXTOP;
   /*MULE(N); */
MULE:
   /*                    if(ferr == 1){       */
   /*                        lastb = lastb1;  */
   /*                        ferr = 0;}       */
   /*                    else                 */
   /*                        lastb1 = lastb;  */
   n = (unsigned)*(vpc + NMBL);
   vpc = vpc + NMBL + NMBL;
   f0 = et[n - 1]->prev;
   while (f0 != et[n])
   {
      f0 = f0->next;
      SHF /*              f = f->next;       instead of SHF */
          /*             if (f == flhead){                  */
          /*               ferr = 1;                        */
          /*               goto LACK;}                      */
          if ((f0->tag & 0001) != 0)
      {
         if (f0->tag != TAGRB)
         {
            f->info.codep = lastb;
            lastb = f;
         }
         else
         {
            f->info.codep = lastb;
            f->tag = TAGRB;
            f1 = lastb->info.codep;
            lastb->info.codep = f;
            lastb->tag = TAGLB;
            lastb = f1;
         };
      }
      else
      {
         move(SMBL, &(f0->tag), &(f->tag));
      }
   };
   goto NEXTOP;
   /*TPL(N,M); */
   /*TPLM(N,M);*/
TPL:;
TPLM:
   n = (unsigned)*(vpc + NMBL);
   m = (unsigned)*(vpc + NMBL + NMBL);
   vpc = vpc + NMBL * 3;
   if (et[m]->next == et[n])
      goto NEXTOP;
   putts(tsp, &f, &et[n], &et[m]);
   tsp++;
   goto NEXTOP;
   /* TPLE(N); (= TPL(N-1,N);)*/
   /* TPLV(N);                */
TPLE:;
TPLV:
   n = (unsigned)*(vpc + NMBL);
   vpc = vpc + NMBL + NMBL;
   if (et[n]->next == et[n - 1])
      goto NEXTOP;
   putts(tsp, &f, &et[n - 1], &et[n]);
   tsp++;
   goto NEXTOP;
   /*TPLS(N); (= TPLM(N,N);) */
TPLS:
   n = (unsigned)*(vpc + NMBL);
   vpc = vpc + NMBL + NMBL;
   putts(tsp, &f, &et[n], &et[n]);
   tsp++;
   goto NEXTOP;
}
/*------------ end of file RFRUN1.C ----------*/
