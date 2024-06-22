/*------------ file -- D.H--------------------*/
/*       Definitions for REFAL-Debugger       */
/*        Last modification : 19.06.24        */
/*--------------------------------------------*/
#ifndef D_H
#define D_H

#include <stddef.h>
#include "refal.def"

typedef struct DET_TAB
{
    struct DET_TAB *det_next;
    char gt;
    char ge;
    char eq;
    char ne;
    char lt;
    char le;
    char tr;
    char *det_id;
} DET_TAB;
static DET_TAB *last_det = NULL;
static DET_TAB *det_table;
static DET_TAB dummy_det_table = {NULL, 0, 0, 0, 0, 0, 0, 0, NULL};

static unsigned short int trace_cond = 0;
static unsigned short int trap_cond = 0;
static unsigned short int ge_all = 1;
static unsigned short int eq_all = 1;
static unsigned short int e1empty = 0;
static unsigned long int s_from = 0L;
static unsigned long int s_upto = 0L;
static unsigned long int s_stop = 2147483647L;
static int nogcl = 0; /* garbage collection counter */
static int s_arg;
static int l_arg;
static char buff_id[100];
static char buff[100];
static char *arg = buff;
static unsigned long int printed_step;
static unsigned long int curr_step;
static unsigned long int euc_step;
static unsigned long int res_step;
static unsigned short int was_ge;
static unsigned short int was_le;
static unsigned short int was_eq;
static unsigned long int curr_step1;
static unsigned long int curr_step2;
static linkcb *nextk;
static linkcb *res_prevk;
static linkcb *res_nextd;
static linkcb *pk, *prevk, *nextd;
static linkcb *dot1, *prevk1, *nextd1;
static linkcb *dot2, *prevk2, *nextd2;
static int i, c2;

extern void (*dba)();
extern void rfdbg(st *s_st);

#endif
/*---------  end of file DEBUG.DEF -----------*/
