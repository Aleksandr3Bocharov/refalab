#ifndef CCST1_H
#define CCST1_H

#include "cgop.h"

/* the table of assembly language statement codes */
#define n_sjump '\001'
#define n_fail '\002'
#define n_sb1b2 '\003'
#define n_lsc '\004'
#define n_rsc '\005'
#define n_lsco '\006'
#define n_rsco '\007'
#define n_lsd '\010'
#define n_rsd '\011'
#define n_ltxt '\012'
#define n_rtxt '\013'
#define n_lb '\014'
#define n_lby '\015'
#define n_rb '\016'
#define n_rby '\017'
#define n_ls '\020'
#define n_rs '\021'
#define n_lw '\022'
#define n_rw '\023'
#define n_lbnil '\024'
#define n_rbnil '\025'
#define n_lbce '\026'
#define n_rbce '\027'
#define n_nil '\030'
#define n_ce '\031'
#define n_led '\032'
#define n_red '\033'
#define n_nnil '\034'
#define n_ple '\035'
#define n_plv '\036'
#define n_le '\037'
#define n_pre '\040'
#define n_prv '\041'
#define n_re '\042'
#define n_plesc '\043'
#define n_plvsc '\044'
#define n_lesc '\045'
#define n_presc '\046'
#define n_prvsc '\047'
#define n_resc '\050'
#define n_lesd '\051'
#define n_resd '\052'
#define n_pleb '\053'
#define n_plvb '\054'
#define n_leb '\055'
#define n_preb '\056'
#define n_prvb '\057'
#define n_reb '\060'
#define n_eoe '\061'
#define n_eoei '\062'
#define n_lsrch '\063'
#define n_rsrch '\064'
#define n_wspc '\065'
#define n_espc '\066'
#define n_plespc '\067'
#define n_lespc '\070'
#define n_prespc '\071'
#define n_respc '\072'
#define n_lmax '\073'
#define n_rmax '\074'
#define n_eor '\075'
#define n_ns '\076'
#define n_nso '\077'
#define n_text '\100'
#define n_bl '\101'
#define n_br '\102'
#define n_blr '\103'
#define n_bract '\104'
#define n_act '\105'
#define n_muls '\106'
#define n_mule '\107'
#define n_tpl '\110'
#define n_tplm '\111'
#define n_tple '\112'
#define n_tplv '\113'
#define n_tpls '\114'
#define n_eos '\115'
#define n_swap '\116'
#define n_blf '\117'
#define n_eossn '\120'
#define n_setnos '\121'

struct x_
{ /* left part buffer elements */
    unsigned short int p, q, t, i;
    struct linkti code;
    unsigned short int next;
    unsigned short int pair;
    struct linkti spec;
    unsigned short int v;
    unsigned short int eoemrk;
    unsigned short int e_level;
};
typedef struct x_ T_X;

extern T_X x[];

struct v_
{ /* variable table elements */
    unsigned short int _t, _q;
    unsigned short int rem;
    unsigned short int last;
    char ci;
    unsigned short int _v;
};
typedef struct v_ T_V;

extern T_V v[];

extern const unsigned short int t_lb;
extern const unsigned short int t_rb;
extern const unsigned short int t_e;

extern unsigned short int n, n1, n2;  /* left part element pointers */
extern unsigned short int i, ie;      /* element index */
extern unsigned short int nel;        /* current element number */
extern unsigned short int e_level;    /* counter of the longing levels */
extern unsigned short int not_nil;    /* working variables */
extern unsigned short int nh;         /* current whole number */
extern unsigned short int kol_per;    /* subprogram of search in variable table */
                                      /* table pointer */
extern unsigned short int nh_x, nh_y; /* hole numbers (under enter in brackets)  */
extern unsigned short int lrbxy;      /* stoped bracket flag */

extern void cst(unsigned int dir, unsigned char *lbl, unsigned int lblleng);

#endif