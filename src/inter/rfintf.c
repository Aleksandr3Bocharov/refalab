/*-----------  file  --  RFINTF.C ------------------*/
/*           C-interface functions                  */
/*        Last modification : 17.07.2004 (BLF)      */
/*--------------------------------------------------*/
#include <stdio.h>
#include "refal.def"

REFAL   refal;
static  linkcb  *last_block=NULL;
static  int     rf_init=1;
static  int     curr_size=0;
static  linkcb  hd;

linkcb *malloc();
void   rflist();
void   rfpexm();

void rfabe (amsg) char *amsg;{
  printf ("\n *** refal-abend *** %s",amsg);
  exit(1);
}

lincrm() {
 linkcb *first_free, *p;
 linkcb *new_block; 
 int was_coll, n ;
   if (last_block != NULL) {
      first_free = refal.flhead->next;
      was_coll = lgcl();
      if (was_coll == 1) {
         p = refal.flhead->next;
         n = 0;
         while (( p != first_free) && ( n != 1000)) {
            n++;
            p = p->next;
         }
         if (n == 1000) return (TRUE);
      }
   }
   new_block = malloc(1001 * sizeof(linkcb));  /* kras 06.12.88 */
#ifdef mdebug
printf("\nLincrm: n=%d after new_block=%lx",n,new_block);
#endif
   if (new_block == NULL) return (FALSE);
   new_block->prev = last_block;
   last_block = new_block;
   curr_size = curr_size + 1000;   /* kras 06.12.88 */
   rflist (new_block + 1, 1000);   /* kras 06.12.88 */
   return (TRUE);
}

/*  check a number of items in free items list */
lrqlk (l) int l; {
 linkcb *p;  int n;
   p = refal.flhead;
   for( n=0; n<l; n++ ){
      p = p->next;
      if( p == refal.flhead ) return (FALSE); 
   }
   return (TRUE);
}

lins (p,l) linkcb *p; int l; {
 int n;  linkcb *p1,*q,*q1,*r;
   if (l<1) return (1);
    q1 = refal.flhead;
    for( n=0; n<l; n++ ){
       q1 = q1->next;
       if (q1 == refal.flhead) return (FALSE);
       q1->tag = TAGO;
       q1->info.codep = NULL;
    }
    r = q1->next;
    q = refal.flhead->next;
    refal.flhead->next = r;
    r->prev = refal.flhead;
    p1 = p->next;
    q1->next = p1;
    p1->prev = q1;
    p->next = q;
    q->prev = p;
    return (TRUE);
}

slins (p,k) linkcb *p; int k; {
   while( !lrqlk(k) ){
      if( !lincrm() )
         { refal.upshot=3; return(FALSE); }
   }
   return( lins(p,k) );
}

linskd (ast,f)  st *ast;  char *f; {
 linkcb *p,*q,*r;
   if( !lexist(ast) )
       rfabe ("Linskd: process doesn't exist still");
   if( ast->dot!=NULL )
       rfabe ("Linskd: there are 'k'-signes in view field");
   if( !slins(ast->view,3) ) return (FALSE);
   p= ast->view->next;
   r = p->next;
   q = ast->view->prev;
   p->tag = TAGK;
   q->tag = TAGD;
   q->info.codep = p;
   r->tag = TAGF;
   r->info.codep = (linkcb *) f;  
   ast->dot = q;
   return (TRUE);
}

char rfcnv(cm) char cm; {
 int j;
   j = (int) cm;
   if ((j>96) && (j<123)) return ( cm - '\40' );
   else return ( cm );  /* perewod  a..z w A..Z */
/*#ifdef IBM_PC
   if ((j>-97) && (j<-64))  return ( cm - '\40' );
 #else
   if ((j>-33) && (j<0))    return ( cm - '\40' );
 #endif*/
}

void rfinit () {
 REFAL *p;
 linkcb *phd;
   rf_init = 0;
   p = &refal;
   p->crprev = (st *) &refal;  
   p->crnext = (st *) &refal; 
   p->upshot = 1;
   p->currst = NULL;
   p->svar =  NULL;
   p->dvar = NULL;
   p->flhead = &hd;
   phd = &hd;
   phd->prev = phd;
   phd->next = phd;
   phd->tag = 0;
   phd->info.codep=NULL;
   p->nostm = 0;
   p->stmnmb = 0;
   p->tmmode = 0;
   p->tmintv = 0;
}

void rfcanc (ast)  st *ast; {
 linkcb *flhead1,*view1,*store1;
  if( rf_init ) rfinit ();
  if( !lexist(ast) )
     rfabe ("Rfcanc: process doesn't exist");
  if( ast->state == 4 )
     rfabe ("Rfcanc: process is in job yet");
  ast->stprev->stnext = ast->stnext;
  ast->stnext->stprev = ast->stprev;
  flhead1 = refal.flhead->prev;
  view1 = ast->view->prev;
  store1 = ast->store->prev;
  flhead1->next = ast->view;
  ast->view->prev = flhead1;
  view1->next = ast->store;
  ast->store->prev = view1;
  store1->next = refal.flhead;
  refal.flhead->prev = store1;
}

/*    delete part of list and add it to free memory list */
void rfdel (p,q)  linkcb *p,*q;  {
 linkcb *p1,*q1,*r;
  p1 = p->next;
  if (p1 == q) return;
  q1 = q->prev;
  r = refal.flhead->prev;
  p->next = q;
  q->prev = p;
  q1->next = refal.flhead;
  refal.flhead->prev = q1;
  r->next = p1;
  p1->prev = r;
}

void rftermm () {
 linkcb *new_block;
   while( last_block != NULL ){
      new_block = last_block;
      last_block = new_block->prev;
      free(new_block);
#ifdef mdebug
printf("\nLincrm: free new_block=%lx",new_block);
#endif
   }
}

void rfexec (func)  char *func;  {
  
/* BLF 17.07.2004 */
  linkcb *prevk,*nextd,*pk;
	
  st s_st;
  if( rf_init == 1 ) rfinit();
  if( !lincrm()           ) goto LACK;
  if( !lcre(&s_st)        ) goto LACK;
  if( !linskd(&s_st,func) ) goto LACK;
  s_st.stop = 0x7FFFFFFFL;
AGAIN:

/* BLF 17.07.2004 */
#ifdef DBLF  
	/*	step by step execution with full debug trace information 
		see refal-2 user manual (3.14 - example of processing control) */
	while (s_st.state == 1 && s_st.dot != NULL) {
		s_st.stop = s_st.step + 1;
        pk = s_st.dot->info.codep;
        prevk = pk->prev;
        nextd = s_st.dot->next;
        printf ("\n step: %d", s_st.stop);
		rfpexm (" Term: ", prevk, nextd);

        rfrun (&s_st);
        
		if (s_st.state == 1)  rfpexm (" Result: ", prevk, nextd);
    }  
#else
	/* no debug info */ 
	rfrun (&s_st);
	if( s_st.state==3 )
	if( lincrm() ) goto AGAIN;
	if( s_st.dot != NULL ) goto ABEND;
#endif
/* BLF -- end correction 17.07.2004 */
  
DONE:
  printf("\nConcretization is executed");
  goto EOJ;
ABEND:
  switch( s_st.state ){
   case 1:
     printf("\nStop on step number"); break;
   case 2:
     printf("\nRecognition impossible"); break;
   case 3:
     printf("\nFree memory exhausted");
  }
EOJ:
  printf("\nTotal steps number = %ld",s_st.step);
  if (s_st.view->next != s_st.view) {
     printf ("\nView field:");
     rfpexm (  "            ",s_st.view,s_st.view);
  }
  if (s_st.store->next !=s_st.store){
     printf ("\nBurried:");
     rfpexm (  "         ",s_st.store,s_st.store);
  }
  rfcanc (&s_st);
  rftermm();
 
  /* BLF */
  printf ("\n");

  return;
LACK:
  printf ("\nNo ehough memory for initialization");
  rftermm();
}

void rfpexm(pt,pr,pn) char *pt; linkcb *pr,*pn; {
  char *f;
  unsigned char c;
  int l,k,fr;
  linkcb *pr1;

  fr = 0;
  printf ("\n%s",pt);
  while (pr != pn->prev) {
     pr1 = pr;
     pr = pr->next;
     if(pr->prev != pr1) rfabe("rfpexm: list structure violation");
     if (pr->tag == TAGO) {
        if (fr == 0) {
           fr = 1;
           putchar ('\'');
        };
        putchar (pr->info.infoc);
     }
     else {
        if (fr == 1) {
           fr = 0;
           putchar ('\'');
        };
        if (pr->tag == TAGK) putchar ('k');
        else if (pr->tag == TAGD) putchar ('.');
        else if (pr->tag == TAGLB) putchar('(');
        else if (pr->tag == TAGRB) putchar(')');
        else if (pr->tag == TAGN) printf ("/%ld/",gcoden(pr));
        else if (pr->tag == TAGF) {
           putchar ('/');
           f = pr->info.codef - 1;
           c = *f;
           l = (int)c;
           f -= l;
           for (k=1; k<=l; k++,f++) putchar (rfcnv(*f));
           putchar ('/');
        }
        else if (pr->tag == TAGR) printf ("/%%%lx/",pr->info.codep); 
        else if ((pr->tag & 0001 ) != 0) rfabe("rfpexm: unknown bracket type ");
        else printf ("/<%x>,%lx/",pr->tag,pr->info.codep);
     }  
  }
  if (fr == 1) putchar('\''); 
  return; 
}

void rftpl(r,p,q)  linkcb *p, *r, *q; {
 linkcb *r1, *q1, *p1;
   p1 = p->next;
   if ( p1 == q ) return;
   r1 = r->next;    q1 = q->prev;
   p->next = q;     q->prev = p;
   q1->next = r1;   r1->prev = q1;
   r->next = p1;    p1->prev = r; 
}

/*  copy expression and add it to nessecary place  */
lcopy (r,p,q)  linkcb *r,*p,*q; {
 linkcb *r1,*f,*f0,*f1,*lastb = NULL;
   f = refal.flhead;
   f0 =  p->next;
   while (f0 != q) {
      f = f->next;
      if (f == refal.flhead) return(FALSE);
      switch( f0->tag ){
       case TAGLB:
          f->info.codep = lastb;
          lastb = f;
          break;
       case TAGRB:
          f->info.codep = lastb;
          f->tag = TAGRB;
          f1 = lastb->info.codep;
          lastb->info.codep = f;
          lastb->tag = TAGLB;
          lastb = f1;
          break;
       default:
          f->tag = f0->tag;
       /*   pcoden(f,gcoden(f0)); */ 
      }
      f0 = f0->next;
   }
   if (refal.flhead == f) return(1);
   f0 = refal.flhead->next;
   f1 = f->next;
   refal.flhead->next = f1;
   f1->prev = refal.flhead;
   r1 = r->next;
   f->next = r1;
   r1->prev = f;
   r->next = f0;
   f0->prev = r;
   return(1);
}

lexist (ast) st *ast; {
 REFAL *p;
   p = &refal;
   do {
      p =(REFAL *) (p->crnext); 
      if (p == (REFAL *) ast) return (TRUE);
   } while (p != &refal);
   return(FALSE);
}
 
lcre (ast) st *ast; {
 st *q;   linkcb *flhead1;
   if( rf_init ==1) rfinit();
   if( lexist(ast) )
      rfabe ("Lcre: process already exists");
   ast->view = refal.flhead->next;
   if (ast->view == refal.flhead) return(FALSE);
   ast->store = ast->view->next;
   if (ast->store == refal.flhead) return (FALSE);
   flhead1 = ast->store->next;
   refal.flhead->next = flhead1;
   flhead1->prev = refal.flhead;
   (ast->view->next) = (ast->view);
   (ast->view->prev) = (ast->view);
   (ast->store->next) =(ast->store);
   (ast->store->prev) = (ast->store);
   q = refal.crprev;
   ast->stnext = (st *) &refal;  
   refal.crprev = ast;
   q->stnext = ast;
   ast->stprev = q;
   ast->state = 1;
   ast->dot = NULL;
   ast->step = 0L;
   ast->stop = -1L;
   return (TRUE);
}

static void mark(root)  linkcb *root; {
 linkcb *h,*p,*q,*r;
   h = p = root;
MRK:
   if(p->next == h) goto UP;
      p = p->next;
      if (p->tag != TAGR) goto MRK;
      q = p->info.codep;
      if (q->tag != 0) goto MRK;
      q->tag = 0xFFFF;
      p->info.codep = h;
      q->prev = p;
      h = p = q;
      goto MRK;
UP:   if (h == root) return;
      q = h->prev;
      h->prev = p;
      r = h;
      h = q->info.codep;
      q->info.codep = r;
      q->tag = TAGR;
      p = q;
      goto MRK;
}

lgcl() {
 st *p;  int was_coll;
 linkcb *pzero, *q, *r, *flhead1, *p1, hdvar, *hd;
   hd = &hdvar;
   if(refal.dvar == NULL) return(0);
   /* mark boxes achieved from view field & burriage */
   was_coll = 0;
   pzero = NULL;
   p = (st *) refal.crnext; 
   while (p != (st *) &refal) {   
      mark(p->view);
      mark(p->store);
      p = p->stnext;
   }
   /* mark boxes achieved from static boxes    */
   if (refal.svar != NULL) {
      r = refal.svar;
      do {
         mark(r);
         r = r->info.codep;
      } while(r != pzero);
      /*   remove garbage   */
      hd->info.codep = refal.dvar;
      p1 = hd;
      q = refal.dvar;
      do {
         if(q->tag != 0) {   /* box isn't removed */
            q->tag = 0;
            p1 = q;
         }
         else {              /* remove box     */
            was_coll = 1;
            p1->info.codep = q->info.codep;
            p1->tag = q->tag;
            r = q->prev;
            flhead1 = refal.flhead->next;
            r->next = flhead1;
            flhead1->prev = r;
            refal.flhead->next = q;
            q->prev = refal.flhead;
         }
         q = p1->info.codep;
      } while (q != pzero);
      if (hd->info.codep == pzero)
         refal.dvar = NULL;
      else
         refal.dvar = hd->info.codep;
   }
   return (was_coll);  
}

void rflist(par,n) linkcb *par; int n; {
 linkcb *p, *q;
 int k;
  if (rf_init != 0) rfinit();
  q = par;
  p = refal.flhead->prev;
  for( k=0; k<n; k++ ){ 
     p->next = q;
     q->prev = p;
     q->tag = 0; 
     q->info.codep = NULL;          	 
	 p = q;
     q++;
  }
  p->next = refal.flhead;
  refal.flhead->prev = p; return; 
}

void rfpex(pt,pr,pn) char *pt; linkcb *pr,*pn; {
 char *f;
 unsigned char c;
 int l,k;
 linkcb *pr1;
  printf ("\n%s",pt);
  while (pr != pn->prev) {
     pr1 = pr;
     pr = pr->next;
     if (pr1 != pr->prev) rfabe("rfpex: list structure violation");
     if (pr->tag == TAGO) putchar (pr->info.infoc);
     else if (pr->tag == TAGK) putchar ('k');
     else if (pr->tag == TAGD) putchar ('.');
     else if (pr->tag == TAGLB) putchar('(');
     else if (pr->tag == TAGRB) putchar(')');
     else if (pr->tag == TAGN) printf ("'%ld'",gcoden(pr));
     else if (pr->tag == TAGF) {
        putchar ('\'');
        f = pr->info.codef - 1;
        c = *f;
        l = (int)c;
        f = f - l;
        for (k=1; k<=l; k++,f++) putchar (rfcnv(*f));
        putchar ('\'');
     }
     else if (pr->tag == TAGR) printf ("'%%%lx'",pr->info.codep); 
     else if ((pr->tag & 0001) != 0)  rfabe("rfpex: unknown bracket type ");
     else printf ("'%x,%lx'",pr->tag,pr->info.codep);
  }  return; 
}

linkcb *lldupl(p,q,u) linkcb *p,*q,*u; {
 linkcb *x,*y;
   x = p->next;
   y = u->next;
   while ( x != q) {
      if (x->tag != y->tag) return(0);
      if (x->info.codef != y->info.codef)
         if ((x->tag != TAGLB) && (x->tag != TAGRB)) return(0);
      x = x->next;
      y = y->next;
   }
   return (y);
}
/*----------- end of file  RFINTF.C ------------*/
