/*-------------- file -- XMO.C -------------*/  
/*                 General MO:              */
/*     p1, m1, numb, symb, first, last,     */
/*     lengr, lengw, multe, crel, delf      */
/*      Last edition date : 02.09.90        */
/*------------------------------------------*/
#include <stdio.h> 
#include "refal.def"
extern REFAL refal;

static void p1_() {
 linkcb *p; long l;
   p = refal.preva->next;
   if ( (p->next != refal.nexta) || (p->tag != TAGN) ) {
NEOT: refal.upshot = 2;
      return;
   }
   l=gcoden(p)+1;
   if (l > 16777215l ) goto NEOT;
   pcoden(p,l);
   rftpl(refal.prevr, p->prev, p->next);
   return;
 }
static char p1_0[] = {Z2   'P','1','\002'};
G_L_B char p1 = '\122';    static void (*p1_1)() = p1_;

static void m1_() {
 linkcb *p; long l;
   p = refal.preva->next;
   if ( (p->next != refal.nexta) || (p->tag != TAGN) ) {
NEOT: refal.upshot = 2;
      return;
   }
   l=gcoden(p)-1;
   if (l<0) goto NEOT;
   pcoden(p,l);
   rftpl(refal.prevr, p->prev, p->next);
   return;
 }
static char m1_0[] = {Z2    'M','1','\002'};
G_L_B char m1 = '\122';     static void (*m1_1)() = m1_;

static int cmpstr(n,s1,s2)  char *s1,*s2;  int n;  {
 /* comparison two string . if s1<s2 then return < 0 */
 /* if s1 = s2 return 0. if s1>s2 then return > 0    */
 int i;
   for (i=0; i<n; i++, s1++, s2++)
      if (*s1 != *s2)  return(*s1 - *s2);
   return(0);
}

long atol();

static void numb_() {
 linkcb *p,*pz,*p1;
 char str[12],zn;
 register i; long l;
   p = refal.preva->next;
   zn = p->info.infoc;
   pz=p;
   if ((p->tag == TAGO) && ((zn=='-')||(zn=='+')) ) {
      p = p->next;
      if(zn=='+')  pz=p; 
   }
   p1=p;
   while ((p->tag==TAGO) && (p->info.infoc=='0')) p=p->next;
   for (i=0; p != refal.nexta; i++) {
      if( (p->tag != TAGO) || (i==11) ) {
NEOT:    refal.upshot = 2;
         return;
      }
      else {
         str[i] = p->info.infoc;
         if (str[i] < '0' || str[i] > '9' ) goto NEOT;
         if ((i==9) && (cmpstr(i+1,str,"2147483647") > 0) )
            goto NEOT;
         p=p->next;
      }
   }
   str[i]='\0';
   if(strlen(str)==0) {
      if( !slins(refal.nexta->prev,1) ) return; 
      p1=refal.nexta->prev; pz=p1;
   }
   l=atol(str);
   p1->tag = TAGN;
   if(l>16777215l) { pcoden(p1,l>>24);
      p1=p1->next; p1->tag = TAGN; l = l & 0xffffffL;
   }
   pcoden(p1,l);
   rftpl(refal.prevr,pz->prev, p1->next);
   return;
}
static char numb_0[] = {Z4   'N','U','M','B','\004'};
G_L_B char numb = '\122';    static void (*numb_1)() = numb_;

static void symb_() {
 linkcb *p,*pz,*p1;
 char str[12],zn;
 register i; long l; int j;
   p = refal.preva->next;
   zn = p->info.infoc;
   pz=p;
   if ((p->tag == TAGO) && ((zn=='-')||(zn=='+')) ) {
      p = p->next;
      if(zn=='+') pz=p;
   }
   p1=p;
   while ((p->tag==TAGN) && (gcoden(p)==0l)) p=p->next;
   for (i=0; p != refal.nexta; i++, p=p->next )
      if( (p->tag != TAGN) || (i==2) ) goto NEOT;
   p=p->prev;
   if( (i==2) && (gcoden(p1) >= 128) )  {
NEOT:    refal.upshot = 2;
         return;
   }
   l=gcoden(p);
   if(i==2) l=l+ 16777216l * gcoden(p1);
   if((i==0)||(l==0l)) { pz=p1; l=0l; }
   sprintf(str,"%ld",l);
   j=strlen(str);
   if( !lrqlk(j) )
      if( !lincrm() ){  refal.upshot = 3;  return; }
   if( pz != refal.nexta )  lins(p1,j);
   else { pz=pz->prev; lins(pz,j); pz=pz->next; p1=pz; } 
   for(i=0, p=p1; i<j; i++, p=p->next) {
      p->tag=TAGO; p->info.codep=NULL; p->info.infoc = str[i];
   }
   rftpl(refal.prevr,pz->prev, p);
   return;
}
static char symb_0[] = {Z4   'S','Y','M','B','\004'};
G_L_B char symb = '\122';    static void (*symb_1)() = symb_;

static void first_() {
 linkcb *p,*pn;
 int k;  long n;  /*eg*/
   pn = refal.preva->next;
   if ((pn == refal.nexta) || (pn->tag != TAGN))
                         { refal.upshot = 2; return; }; /* FAIL */
   n = gcoden(pn);
   p = pn;
   for ( k=1; k<=n; k++) {
      p = p->next;
      if ( p == refal.nexta) {
         pn->info.codep = NULL; pn->info.infoc = '*';
         pn->tag = TAGO;
         rftpl(refal.prevr, refal.preva, refal.nexta);
         return;
      }
      if (p->tag == TAGLB) p = p->info.codep;
   }
   p = p->next;
   refal.preva->tag = TAGLB;  refal.preva->info.codep = pn; 
   pn->tag = TAGRB;   pn->info.codep = refal.preva; 
   rftpl(refal.preva,pn,p);
   rftpl(refal.prevr,refal.nextr,refal.nexta); return;
}
static char first_0[] = {Z5  'F','I','R','S','T','\005'};
G_L_B char first = '\122';   static void (*first_1)() = first_;

static void last_() {
 linkcb *pn,*p;
 int k; long n;  /*eg*/
   pn = refal.preva->next;
   if((pn == refal.nexta) || (pn->tag != TAGN))
                        { refal.upshot = 2; return; }; /* FAIL */
   n = gcoden(pn);
   p = refal.nexta;
   for(k=1; k<=n; k++) {
      p = p->prev;
      if (p == pn) {
         pn->tag = TAGO;  pn->info.codep = NULL; pn->info.infoc = '*';
         rftpl(refal.prevr,pn,refal.nexta);
         p = refal.nextr->prev;
         rftpl(p,refal.preva,refal.nexta);
         return;
      }
      if(p->tag == TAGRB) p = p->info.codep;
   }
   p = p->prev;
   refal.preva->tag = TAGLB;   refal.preva->info.codep = pn; 
   pn->tag = TAGRB;   pn->info.codep = refal.preva;
   rftpl(refal.preva,p,refal.nexta);
   rftpl(refal.prevr,pn,refal.nexta);
   p = refal.nextr->prev;
   rftpl(p,refal.nextr,refal.nexta); return; 
}
static char last_0[] = {Z4   'L','A','S','T','\004'};
G_L_B char last = '\122';    static void (*last_1)() = last_;

static void lengr_() {
 linkcb *p;
 long n; /* kras */
   n = 0l;
   p = refal.preva->next;
   while( p != refal.nexta) {
      n++;
      p = p->next;
   }
   refal.preva->tag = TAGN;
   pcoden(refal.preva, n);
   rftpl(refal.prevr,refal.nextr,refal.nexta); return; 
}
static char lengr_0[] = {Z5  'L','E','N','G','R','\005'};
G_L_B char lengr = '\122';   static void (*lengr_1)() = lengr_;

static void lengw_() {
 linkcb *p;
 long n;  /* kras  */
   n = 0l;
   p = refal.preva->next;
   while (p != refal.nexta){
      n++;
      if(p->tag == TAGLB) p = p->info.codep;
      p = p->next;
   }
   refal.preva->tag = TAGN;
   pcoden(refal.preva, n);
   rftpl(refal.prevr,refal.nextr,refal.nexta); return; 
}
static char lengw_0[] = {Z5  'L','E','N','G','W','\005'};
G_L_B char lengw = '\122';   static void (*lengw_1)() = lengw_;

static void multe_() {
 linkcb *p,*pn,*q;
 long n,k;
   pn = refal.preva->next;
   if((pn == refal.nexta) || (pn->tag != TAGN))
                     { refal.upshot = 2; return;};  /* FAIL */
   n = gcoden(pn);
   if(n == 0) return;
   p  = pn->next;
   if (p == refal.nexta) return;
   if(p->next != refal.nexta) {
      do {
         p = refal.nextr->prev;
         if (!lcopy(p,pn,refal.nexta))
            { refal.upshot = 3; return;};  /* LACK */
         n--;
      } while (n >= 1);
   }
   else {
      if( !slins(refal.prevr,n) ) return;  /*  LACK */
      q = refal.prevr;
      for (k=0; k<n; k++) {
         q = q->next;
         q->tag = p->tag;
         pcoden(q, gcoden(p));
      }
   }
   return; 
}
static char multe_0[] = {Z5  'M','U','L','T','E','\005'};
G_L_B char multe = '\122';   static void (*multe_1)() = multe_;

static void delf_() {
 linkcb *dot,*dot1,*sk,*nd;
  if(refal.preva->next != refal.nexta){refal.upshot = 2;return;} /*FAIL*/
  dot = refal.nexta;
  dot1 = refal.nextr->info.codep;
REPEAT:
  sk = dot->info.codep;
  if (sk == NULL) rfabe("delf: sign '#' missing ");
  dot = sk->info.codep;
  nd = dot->next;
  if (nd->info.infoc != '#' ) goto REPEAT;
  while (dot1 != dot ) {
   sk = dot1->info.codep;
   nd = sk->info.codep;
   rfdel( sk->prev, dot1->next);
   dot1 = nd;
  }
  sk = dot1->info.codep;
  nd = sk->info.codep;
  dot1 = dot1->next;
  rfdel (sk->prev, dot1->next);
  refal.nextr->info.codep = nd; return; 
}
static char delf_0[] = {Z4  'D','E','L','F','\004' };
G_L_B char delf = '\122';   static void (*delf_1)() = delf_;

static void crel_() {
 char c;
 linkcb *p,*p1,*q,*q1;
   p = refal.preva->next;
   if ( p->tag != TAGLB ) goto FAIL;
   p1 = p->info.codep;
   p = p->next;
   q = p1->next;
   q1 = refal.nexta;
   c = '=';
   for(; c=='=' && p!=p1 && q!=q1; p=p->next, q=q->next) {
      if( p->tag==TAGLB ) {
         if( q->tag!=TAGLB ) goto FAIL;
         else break;      }
      if( p->tag==TAGRB ) {
         if( q->tag!=TAGRB ) goto FAIL;
         else break;      }
      if( p->info.coden < q->info.coden ) c = '<';
      if( p->info.coden > q->info.coden ) c = '>';
   }
   if( p==p1 && q!=q1 ) c = '<';
   if( q==q1 && p!=p1 ) c = '>';
   for(; p!=p1; p=p->next) if( p->tag==TAGLB ) goto FAIL;
   for(; q!=q1; q=q->next) if( q->tag==TAGLB ) goto FAIL;
   p = refal.preva->next;
   p->tag = TAGO;   p->info.codep = NULL;   p->info.infoc = c;
   q = p->next;
   rftpl(refal.prevr, refal.preva, q);
   return;
FAIL:
   refal.upshot = 2; return;  
}
static char crel_0[] = {Z4  'C','R','E','L','\004'};
G_L_B char crel = '\122';   static void (*crel_1)() = crel_;

/*-------------------- end of file  XMO.C ----------------*/
