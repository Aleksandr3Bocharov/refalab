/*-------------- file -- XTIME.C ------------*/
/*        MO: rftime (date and time)         */
/*            rftm (current process time)    */
/*         Last edition date : 09.09.90  BLF */
/*-------------------------------------------*/
#include <stdio.h>
#include "refal.def"

/*
#ifdef IBM_PC
#include "\refal\time.h"
#include "\refal\dos.h"

#ifdef QC
#define gettime _dos_gettime
#define ti_hund hsecond
#define ti_sec second
#define ti_min minute
#define ti_hour hour
  typedef struct dostime_t tm;
#else
  typedef struct time tm;
#endif
  static tm t0,t1;
#endif

unsigned long int time();
char *ctime();

static void rftime_() {
 linkcb *p;
 unsigned long int   tim; char *c;
   tim=time(0l);
   c=ctime(&tim);
   p = refal.prevr;
   while( *c >= ' ' ){
      if( !slins(p,1) ) { refal.upshot=2; return; }
      p = p->next;
      p->tag = TAGO; p->info.codep=NULL;  p->info.infoc = *c;
      c++;
   }
}
static char rftime_0[] = {Z6      'R','F','T','I','M','E','\006'};
G_L_B char rftime = '\122';   static void (*rftime_1)() = rftime_;

static void rftm_() {
 linkcb *p; int ih=0,im=0,is=0,ik,i;
 char c,s[12];
   p = refal.preva->next;
   if( p != refal.nexta ){
      if( p->tag != TAGO ) goto NEOT;
      c = p->info.infoc;
      switch (c) {
         case 's':
         case 'S':
#ifdef IBM_PC
            gettime(&t0);
#endif
            return;
         case 'g':
         case 'G':
#ifdef IBM_PC

         /* BLF            gettime(&t1);
            ik = t1.ti_hund - t0.ti_hund;
            if( ik<0 ) {
               ik += 100;  is--; }
            is += (t1.ti_sec - t0.ti_sec);
            if( is<0 ) {
               is += 60;  im--; }
            im += (t1.ti_min - t0.ti_min);
            if( im<0 ) {
               im += 60;  ih--; }
            ih += (t1.ti_hour - t0.ti_hour);
#endif
            if( ih<0 ) ih += 24;
            sprintf(s,"%2d:%2d:%2d.%2d",
                       ih, im, is, ik);
            for( i=0; i<10; i+=3 )
               if(s[i]==' ') s[i]='0';
            p = refal.prevr;
            if ( !slins(p,strlen(s)) ) return;
            for( i=0; s[i] != 0; i++ ){
               p = p->next;
               p->tag = TAGO; p->info.codep=NULL; p->info.infoc = s[i];
            }
            return;
         default : goto NEOT;
      }
   }
NEOT: refal.upshot = 2; return;
}
static char rftm_0[] = {Z4          'R','F','T','M','\004'};
G_L_B char rftm = '\122';   static void (*rftm_1)() = rftm_;
*/
/*------------------ end of file  XTIME.C ----------------*/
