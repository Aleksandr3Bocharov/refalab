/*------------------file----CGOP.C--------------------*/
/*    generation of the assembly language operators   */
/*          Last edition date : 15.05.2024            */
/*----------------------------------------------------*/
/* #define PRCODE */ /* BLF */
#include "refal.def"
#include "cgop.h"

struct linkti {
   short tagg;
   union {
      char infoc;
/*      long coden;*/
      char *codef;
   } infoo;
};
struct _TAG {
   char b1;
   char b2;
};

extern void j3addr();
extern void gsymbol();

/*void gop (k)  char k; {
#ifdef PRCODE
 prcode(k);
#endif
   jbyte(k);
}*/

void gopn(char  k, char n) {
#ifdef PRCODE
 prcode(k); printf ("  %d", (int)n );
#endif
   jbyte(k);
   jbyte(n);
}
void gopnm(char k , char n , char m) {
#ifdef PRCODE
 prcode(k); printf ("  %d %d", (int) n, (int) m );
#endif
   jbyte(k);
   jbyte(n);
   jbyte(m);
}
void gopl(char k, char *l) {
#ifdef PRCODE
 prcode(k); printf (" %lx", l );
#endif
#ifdef PDP
   jvir();  /* vyravnivanie */
#endif
   jbyte(k);
   j3addr(l);
}
void gsymbol(struct linkti *code) {
 struct _TAG *q;
 char *r;
 register int i;
   q = (struct _TAG *) &(code->tagg);
   jbyte( q->b1 );
   jbyte( q->b2 );
   if (code->tagg == 2) {
      j3addr(code->infoo.codef);
      return;
/*      jbyte( '\0' ); jbyte( '\0' );  return; - for no LARGE IBM_PC */
   };
   r = (char *) &(code->infoo.codef);
   if (code->tagg == 0) {
      jbyte(*r);  jbyte(*(r+1));
#ifndef PDP
      jbyte( '\0' );    jbyte( '\0' );
#endif
   }
   else  for (i=0; i<LBLL;i++)  jbyte(*(r+i));
}

void gops(char k, struct linkti *code) {
#ifdef PRCODE
 prcode(k); prstruct(code);
#endif
#ifdef PDP
   jvir();  /* vyravnivanie */
#endif
   jbyte(k);
   gsymbol(code);
}

void ghw(short h) {
struct _TAG *po;
  po = (struct _TAG *) &h;   /*  eg */
  jbyte(po->b1);
  jbyte(po->b2);
}
