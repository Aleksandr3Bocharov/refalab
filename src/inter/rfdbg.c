/*------------ file -- RFDBG.C -------------*/
/*          Standard REFAL-Debugger         */
/*        Last modification : 14.09.91      */
/*------------------------------------------*/
#include <stdio.h>
#include "refal.def"

static st s_st;             
extern char go();

void main () { 
  rfinit();
  if( !lincrm()         ) goto LACK;
  if( !lcre(&s_st)      ) goto LACK;
  if( !linskd(&s_st,go) ) goto LACK;  
  rfdbg(&s_st);  /* there is exit */
LACK:
  printf("\nREFAL-debugger: no memory for initialization");
  fclose(stdin);
  rftermm();
  exit(1);
}
/*---------  end of file RFDBG.C -----------*/
