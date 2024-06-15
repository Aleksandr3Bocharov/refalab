/*----------   file  CCST1.C  --------------*/
/*    The first of two compiler files       */
/*      Last edition date : 12.06.24        */
/*------------------------------------------*/
#include <stdio.h>
#include "ccst1.h"
#include "cerr.h"
#include "cs.h"
#include "cgop.h"
#include "cj.h"
#include "ccst2.h"

#define TAG_O 0
#define TAG_F 2
#define TAG_N 4

struct linkti
{
    short tag;
    union
    {
        char *pinf;
        /*      long   intinf;*/
        char chinf[2];
    } info;
};

/* left part buffer elements */
struct
{
    short p, q, t, i;
    struct linkti code;
    short next;
    short pair;
    struct linkti spec;
    short v;
    short eoemrk;
    short e_level;
} x[100];

/* whole list */
struct
{
    short _t, _q;
    short rem;
    short last;
    char ci;
    short _v;
} v[50];

/* variable table elements */
struct
{
    short _next;
    short n1, n2;
} h[30];

/* compiling   options */
extern struct
{
    unsigned int source : 1;
    unsigned int xref : 1;
    unsigned int stmnmb : 1;
    unsigned int extname : 1;
    unsigned int pgtitle : 1;
    unsigned int asmb : 1;  /* kras */
    unsigned int names : 1; /* kras */
} options;

/* the table for corresponding with scanner */
extern struct
{
    int nomkar;
    char modname_var[40];
    int modnmlen;
    int curr_stmnmb;
} scn_;

/* the element of sentence */
extern struct
{
    short t_;
    char ci_;
    int v_;
    struct linkti _code;
    struct linkti _spec;
} scn_e;

short t_sc = 1;
short t_lb = 2;
short t_rb = 3;
short t_s = 4;
short t_w = 5;
short t_e = 6;
short t_k = 7;
short t_p = 8;

struct linkti xncode;  /* work structure */
struct linkti funcptr; /* work pointer */
short n, n1, n2;       /* left part element pointers */
short i, ie;           /* element index */
short nel;             /* current element number */
short lastb, lastb1;   /* variables for brackets linkage  */
short kol_lit;         /* counter of the symbol number */
short e_level;         /* counter of the longing levels */
short diff_e_level;
short not_nil;     /* working variables */
int kol_skob[100]; /* stack for counting of the brackets balance */
int ur_skob;
char buf_lit[80]; /* buffer for generating of the "text" statement */
short k;
short fh;      /* free segment number in the whole  list */
short nh;      /* current whole number */
short next_nh; /* next whole number */
/* subprogram of search in variable table */
/* table pointer */
short kol_per;
short nh_x, nh_y; /* hole numbers (under enter in brackets) */
short lrbxy;      /* stoped bracket flag */
/* read left part   */
/* and full array X */

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
void scan();
void pch303();
void pch406();
void isk_v();
void gen_bsb();
#define gop(n) jbyte(n)
int lsg_p();
int rsg_p();

void cst(int dir, char *lbl, int lblleng)
/* int dir;     matching feature :left to right or otherwise */
/* char *lbl;   sentence label */
/* int lblleng; sentence label length */
{
    kol_per = 0;
    n = 0;
    lastb = 0;
    nel = 0;
GET_LPE: /* read left part element */
    n++;
    scan();
    x[n].t = scn_e.t_;
    x[n].code.tag = scn_e._code.tag;
    x[n].code.info.pinf = scn_e._code.info.pinf;
    x[n].spec.tag = scn_e._spec.tag;
    x[n].spec.info.pinf = scn_e._spec.info.pinf;
    x[n].v = scn_e.v_;
    x[n].next = 0;
    x[n].pair = 0;
    x[n].eoemrk = 0;
    x[n].e_level = 0;
    switch (scn_e.t_)
    {
    case 0:
        goto LPE0;
    case 1:
        goto LPE1;
    case 2:
        goto LPE2;
    case 3:
        goto LPE3;
    case 4:
        goto LPE4;
    case 5:
        goto LPE5;
    case 6:
        goto LPE6;
    case 7:
        goto LPE7;
    case 8:
        goto LPE8;
    case 9:
        goto LPE9;
    case 10:
        goto LPE10;
    default:;
    }
LPE0: /*scanner error */
    goto OSH300;
LPE1: /*constant symbol*/
    nel++;
    goto NEXT_LPE;
LPE2: /*left bracket*/
    nel++;
    x[n].pair = lastb;
    lastb = n;
    goto NEXT_LPE;
LPE3: /*right bracket*/
    if (lastb == 0)
    {
        pchosh("302 too many ')' in left part");
        n--;
    }
    else
    {
        nel++;
        lastb1 = x[lastb].pair;
        x[lastb].pair = n;
        x[n].pair = lastb;
        lastb = lastb1;
    }
    goto NEXT_LPE;
LPE4: /*s-varyable */
    isk_v();
    x[n].i = i;
    switch (v[i]._t)
    {
    case 0:
        v[i]._t = 1; /*yet isn't faced*/
        break;
    case 1:
        ++(v[i].rem); /*next position*/
        break;
    default: /*invalid type pointer*/
        pch303();
        n--;
    }
    nel++;
    goto NEXT_LPE;
LPE5: /* w-varyable*/
    isk_v();
    x[n].i = i;
    switch (v[i]._t)
    {
    case 0:
        v[i]._t = 2; /*yet isn't faced*/
        break;
    case 2:
        ++(v[i].rem); /*next position*/
        break;
    default: /*invalid type pointer*/
        pch303();
        n--;
    }
    nel += 2;
    goto NEXT_LPE;
LPE6: /*e- or v-varyable*/
    isk_v();
    x[n].i = i;
    if (v[i]._t == 0) /*yet is't faced*/
        v[i]._t = 3;
    else if ((v[i]._t == 3) && (v[i]._v == scn_e.v_))
        ++(v[i].rem);
    else /*invalid type pointer*/
    {
        pch303();
        n--;
    };
    nel += 2;
    goto NEXT_LPE;
LPE7: /*sign 'k'*/
    pchosh("306 sign 'k' in left part");
    n--;
    goto NEXT_LPE;
LPE8: /*sign '.'*/
    pchosh("307 sign '.' in left part");
    n--;
    goto NEXT_LPE;
LPE9: /*left part end*/
    if (lastb == 0)
        goto RCG;
    pchosh("301 too many '(' in left part");
    goto OSH300;
LPE10: /*sentence end*/
    pchosh("304 under left part default sign '=' ");
    fndef(lbl, lblleng);
    return;
NEXT_LPE: /* end of element processing */
    if (nel <= 252)
        goto GET_LPE;
    pchosh("305 very large left part");
    goto OSH300;
OSH300:
    fndef(lbl, lblleng);
RP_OSH300:
    pchosh("300 sentence is't scanned");
    return;
    /*--------------------------------------------*/
    /*         left part compilation              */
    /*--------------------------------------------*/

RCG:
    fndef(lbl, lblleng);
    n1 = 0;
    n2 = n;
    nel = 4;
    x[n1].p = x[n1].q = 3;
    x[n2].p = x[n2].q = 2;
    h[28]._next = 0;
    nh = 1;
    h[1]._next = 28;
    fh = 2;
    e_level = 0;
    if (dir == 1)
        goto RCGL;
    else
        goto RCGR;

    /*    hard element projection from left part of current hole   */

RCGL:
    n = n1 + 1;
    if (n == n2)
        goto NIL;
    switch (x[n].t)
    {
    case 1:
        goto LSW1;
    case 2:
        goto LSW2;
    case 3:
        goto LSW3;
    case 4:
        goto LSW4;
    case 5:
        goto LSW5;
    case 6:
        goto LSW6;
    };
LSW1: /*        constant symbol        */
    if (x[n].code.tag == TAG_O)
        goto LTXT;
    gops(n_lsc, &(x[n].code));
    goto L1;
LTXT:
    kol_lit = 1;
LTXT1:
    n++;
    if ((n == n2) || (x[n].t != t_sc) || (x[n].code.tag != TAG_O))
        goto LTXT2;
    kol_lit++;
    goto LTXT1;
LTXT2:
    if (kol_lit == 1)
        goto LSCO;
    n = n1;
    gopn(n_ltxt, (char)kol_lit);
LTXT3:
    n++;
    jbyte(x[n].code.info.chinf[0]);
    x[n].p = x[n].q = nel;
    nel++;
    kol_lit--;
    if (kol_lit != 0)
        goto LTXT3;
    n1 = n;
    goto RCGL;
LSCO:
    n = n1 + 1;
    gopn(n_lsco, (char)x[n].code.info.chinf[0]);
    goto L1;
LSW4: /*    s-variable    */
    i = x[n].i;
    if (v[i].last != 0)
        gopn(n_lsd, (char)v[i]._q);
    else
    {
        gop(n_ls);
        v[i]._q = nel;
    };
LSMD:
    x[n].next = v[i].last;
    v[i].last = n;
    (v[i].rem)--;
    if (x[n].spec.info.pinf != NULL)
        gopl(n_wspc, x[n].spec.info.pinf);
L1:
    x[n].p = x[n].q = nel;
    nel++;
    n1 = n;
    goto RCGL;
LSW5: /*   w-variable  */
    i = x[n].i;
    if (v[i].last != 0)
        goto LED;
    gop(n_lw);
    v[i]._q = nel + 1;
    x[n].next = v[i].last;
    v[i].last = n;
    (v[i].rem)--;
    if (x[n].spec.info.pinf != NULL)
        gopl(n_wspc, x[n].spec.info.pinf);
    goto L2;
LEM:
    v[i]._q = nel + 1;
    goto LEMD;
LSW6: /*   e-variable   */
    i = x[n].i;
    if (v[i].last != 0)
        goto LED;
    if (dir == 1)
        goto RCGR;
    if (n + 1 == n2)
        goto CE;
    else
        goto IMPASSE;
LED:
    gopn(n_led, (char)v[i]._q);
LEMD:
    x[n].next = v[i].last;
    v[i].last = n;
    (v[i].rem)--;
    if (x[n].spec.info.pinf != NULL)
        gopl(n_espc, x[n].spec.info.pinf);
L2:
    x[n].p = nel;
    x[n].q = nel + 1;
    nel += 2;
    n1 = n;
    goto RCGL;
LSW2: /*  left bracket  */
    n1 = n;
    n = x[n1].pair;
    if (n1 + 1 == n)
        goto LBNIL;
    if (n1 + 2 != n)
        goto GEN_LB;
    n = n1 + 1;
    if (x[n].t != t_e)
        goto GEN_LB;
    i = x[n].i;
    if (v[i].last != 0)
        goto GEN_LB;
LBCE:
    nel += 2;
    gop(n_lbce);
    v[i]._q = nel + 1;
    x[n].next = v[i].last;
    v[i].last = n;
    (v[i].rem)--;
    if (x[n].v != 0)
        gop(n_nnil);
    if (x[n].spec.info.pinf != NULL)
        gopl(n_espc, x[n].spec.info.pinf);
    x[n].p = nel;
    x[n].q = nel + 1;
    nel += 2;
    x[n1].p = x[n1].q = nel - 4;
    n1 += 2;
    x[n1].p = x[n1].q = nel - 3;
    goto RCGL;
LBNIL:
    gop(n_lbnil);
    x[n1].p = x[n1].q = nel;
    n1 = n;
    x[n1].p = x[n1].q = nel + 1;
    nel += 2;
    goto RCGL;
GEN_LB:
    n = n1;
    lrbxy = 1;
LB1:
    if (dir == 1)
    {
        nh_x = nh;
        nh_y = fh;
    }
    else
    {
        nh_x = fh;
        nh_y = nh;
    };
    h[fh]._next = h[nh]._next;
    h[nh]._next = fh;
    fh++;
    n1 = n;
    n = n2;
    n2 = x[n1].pair;
    nh = nh_x;
    h[nh_y].n1 = n2;
    h[nh_y].n2 = n;
    x[n1].p = x[n1].q = nel;
    x[n2].p = x[n2].q = nel + 1;
    nel += 2;
    goto HSCH;

    /*                 hard element projection                */
    /*                 from  right board of                   */
    /*                    current hole                        */

RCGR:
    n = n2 - 1;
    if (n == n1)
        goto NIL;
    switch (x[n].t)
    {
    case 1:
        goto RSW1;
    case 2:
        goto RSW2;
    case 3:
        goto RSW3;
    case 4:
        goto RSW4;
    case 5:
        goto RSW5;
    case 6:
        goto RSW6;
    };
RSW1: /*   constant symbol   */
    if (x[n].code.tag == TAG_O)
        goto RTXT;
    gops(n_rsc, &(x[n].code));
    goto R1;
RTXT:
    kol_lit = 1;
RTXT1:
    n--;
    if ((n == n1) || (x[n].t != t_sc) || (x[n].code.tag != TAG_O))
        goto RTXT2;
    kol_lit++;
    goto RTXT1;
RTXT2:
    if (kol_lit == 1)
        goto RSCO;
    n = n2;
    gopn(n_rtxt, (char)kol_lit);
RTXT3:
    n--;
    jbyte(x[n].code.info.chinf[0]);
    x[n].p = x[n].q = nel;
    nel++;
    kol_lit--;
    if (kol_lit != 0)
        goto RTXT3;
    n2 = n;
    goto RCGR;
RSCO:
    n = n2 - 1;
    gopn(n_rsco, (char)x[n].code.info.chinf[0]);
    goto R1;
RSW4: /*     s_variable       */
    i = x[n].i;
    if (v[i].last != 0)
        gopn(n_rsd, (char)v[i]._q);
    else
    {
        gop(n_rs);
        v[i]._q = nel;
    };
RSMD:
    x[n].next = v[i].last;
    v[i].last = n;
    (v[i].rem)--;
    if (x[n].spec.info.pinf != NULL)
        gopl(n_wspc, x[n].spec.info.pinf);
R1:
    x[n].p = x[n].q = nel;
    nel++;
    n2 = n;
    goto RCGR;
RSW5: /*    w_variable   */
    i = x[n].i;
    if (v[i].last != 0)
        goto RED;
    gop(n_rw);
    v[i]._q = nel + 1;
    x[n].next = v[i].last;
    v[i].last = n;
    (v[i].rem)--;
    if (x[n].spec.info.pinf != NULL)
        gopl(n_wspc, x[n].spec.info.pinf);
    goto R2;
REM:
    v[i]._q = nel + 1;
    goto REMD;
RSW6: /*    e-variable     */
    i = x[n].i;
    if (v[i].last != 0)
        goto RED;
    if (dir == 0)
        goto RCGL;
    if (n1 + 1 == n)
        goto CE;
    else
        goto IMPASSE;
RED:
    gopn(n_red, (char)v[i]._q);
REMD:
    x[n].next = v[i].last;
    v[i].last = n;
    (v[i].rem)--;
    if (x[n].spec.info.pinf != NULL)
        gopl(n_espc, x[n].spec.info.pinf);
R2:
    x[n].p = nel;
    x[n].q = nel + 1;
    nel += 2;
    n2 = n;
    goto RCGR;
RSW3: /*     right bracket     */
    n2 = n;
    n = x[n2].pair;
    if (n + 1 == n2)
        goto RBNIL;
    if (n + 2 != n2)
        goto GEN_RB;
    n = n2 - 1;
    if (x[n].t != t_e)
        goto GEN_RB;
    i = x[n].i;
    if (v[i].last != 0)
        goto GEN_RB;
RBCE:
    nel += 2;
    gop(n_rbce);
    v[i]._q = nel + 1;
    x[n].next = v[i].last;
    v[i].last = n;
    (v[i].rem)--;
    if (x[n].v != 0)
        gop(n_nnil);
    if (x[n].spec.info.pinf != NULL)
        gopl(n_espc, x[n].spec.info.pinf);
    x[n].p = nel;
    x[n].q = nel + 1;
    nel += 2;
    x[n2].p = x[n2].q = nel - 3;
    n2 -= 2;
    x[n2].p = x[n2].q = nel - 4;
    goto RCGR;
RBNIL:
    gop(n_rbnil);
    x[n2].p = x[n2].q = nel + 1;
    n2 = n;
    x[n2].p = x[n2].q = nel;
    nel += 2;
    goto RCGR;
GEN_RB:
    n = n2;
    lrbxy = 2;
RB1:
    if (dir == 0)
    {
        nh_x = nh;
        nh_y = fh;
    }
    else
    {
        nh_x = fh;
        nh_y = nh;
    };
    h[fh]._next = h[nh]._next;
    h[nh]._next = fh;
    fh++;
    n2 = n;
    n = n1;
    n1 = x[n2].pair;
    nh = nh_x;
    h[nh_y].n1 = n;
    h[nh_y].n2 = n1;
    x[n1].p = x[n1].q = nel;
    x[n2].p = x[n2].q = nel + 1;
    nel += 2;
    goto HSCH;
NIL: /*     empty hole    */
    gop(n_nil);
    next_nh = h[nh]._next;
    h[nh]._next = h[next_nh]._next;
    h[nh].n1 = h[next_nh].n1;
    h[nh].n2 = h[next_nh].n2;
    nh = next_nh;
    goto IMPASSE;
CE: /*   closed including   */
    if (x[n].eoemrk != 0)
        goto IMPASSE;
CE1:
    if (x[n].spec.info.pinf == NULL)
        goto CE2;
    if (dir == 1)
        goto LMAX;
    else
        goto RMAX;
CE2:
    i = x[n].i;
    v[i]._q = nel + 1;
    gop(n_ce);
    x[n].next = v[i].last;
    v[i].last = n;
    (v[i].rem)--;
    if (x[n].v == 1)
        gop(n_nnil);
    x[n].p = nel;
    x[n].q = nel + 1;
    nel += 2;
    next_nh = h[nh]._next;
    h[nh]._next = h[next_nh]._next;
    h[nh].n1 = h[next_nh].n1;
    h[nh].n2 = h[next_nh].n2;
    nh = next_nh;
    goto IMPASSE;
    /*                      place of compiler's error          */
LSW3:
RSW2:
    printf("Compiler's error\n");
    exit(1);
/*                                           */
/*          It is impossible movement        */
/*          on hard element here or          */
/*          hole ended here                  */
IMPASSE:
    lrbxy = 0;
    nh_x = nh;
/*          Search of hole with hard          */
/*          elements on its boards.           */
/*          If it not exist than project      */
/*          e-variable from first hole.       */
HSCH:
    h[nh].n1 = n1;
    h[nh].n2 = n2;
    nh = 1;
    if (h[nh]._next == 0)
        goto RCGFIN;
NHOLE:
    n1 = h[nh].n1;
    n2 = h[nh].n2;
    n = n1 + 1;
    if (n == n2)
    {
        gen_bsb();
        goto NIL;
    };
    if (x[n].t != t_e)
        goto RIGID;
    i = x[n].i;
    if (v[i].last != 0)
        goto RIGID;
    if (n + 1 == n2)
    {
        if (v[i].rem == 1)
            goto NHOLE1;
        gen_bsb();
        goto CE1;
    };
    n = n2 - 1;
    if (x[n].t != t_e)
        goto RIGID;
    i = x[n].i;
    if (v[i].last != 0)
        goto RIGID;
NHOLE1:
    nh = h[nh]._next;
    if (h[nh]._next == 0)
        goto OE;
    else
        goto NHOLE;
RIGID: /*  hard element on the both hole boards   */
    gen_bsb();
    if (dir == 1)
        goto RCGL;
    else
        goto RCGR;
    /*  opened e_variable processing  */
OE:
    nh = 1;
    n1 = h[nh].n1;
    n2 = h[nh].n2;
    gen_bsb();
    if (dir == 1)
        n = n1 + 1;
    else
        n = n2 - 1;
    i = x[n].i;
    if (x[n].eoemrk)
    {
        diff_e_level = e_level - x[n].e_level;
        if (diff_e_level == 1)
            gop(n_eoei);
        else
            gopn(n_eoe, (char)diff_e_level);
        e_level = x[n].e_level;
        x[n].eoemrk = 0;
        x[n].e_level = 0;
    };
    if (n1 + 2 == n2)
        goto CE1;
    if (dir == 1)
        n = n2 - 1;
    else
        n = n1 + 1;
    if (x[n].spec.info.pinf == NULL)
        goto OE1;
OE0:
    i = x[n].i;
    if ((v[i].last != 0) || (v[i].rem != 1))
        goto OE1;
    if (dir == 1)
    {
        n--;
        if (n == n1)
            goto OERMAX;
    }
    else
    {
        n++;
        if (n == n2)
            goto OELMAX;
    };
    if ((x[n].t != t_e) || (x[n].v == 1))
        goto OE1;
    goto OE0;
OERMAX:
    n = n2 - 1;
    i = x[n].i;
RMAX:
    gopl(n_rmax, x[n].spec.info.pinf);
    if (x[n].v == 1)
        gop(n_nnil);
    x[n].spec.info.pinf = NULL;
    goto REM;
OELMAX:
    n = n1 + 1;
    i = x[n].i;
LMAX:
    gopl(n_lmax, x[n].spec.info.pinf);
    if (x[n].v == 1)
        gop(n_nnil);
    x[n].spec.info.pinf = NULL;
    goto LEM;
OE1:
    if (dir == 1)
        goto LOE;
    else
        goto ROE;
LOE:
    n = n1 + 1;
    ie = x[n].i;
    /*         attempt to extract left support group     */
LSG:
    if (lsg_p() == 1)
        goto LE_CASE;
    else
        goto RCGL;
LE_CASE:
    n = n1 + 1;
    switch (x[n].t)
    {
    case 1:
        goto LESW1;
    case 2:
        goto LESW2;
    case 3:
        goto LESW3;
    case 4:
        goto LESW4;
    case 5:
        goto LESW5;
    case 6:
        goto LESW6;
    };
LESW1: /*  ei 'a' . . .  */
    xncode.tag = x[n].code.tag;
    xncode.info.pinf = x[n].code.info.pinf;
    n++;
    if ((not_nil == 0) && (x[n].eoemrk == 1))
    {
        x[n].eoemrk = 0;
        x[n].e_level = 0;
        e_level--;
        gops(n_lsrch, &xncode);
    }
    else
    {
        gpev(n_plesc, n_plvsc);
        gops(n_lesc, &xncode);
    };
    n--;
    goto L1;
LESW2: /*   ei ( . . . ) . . . */
    gpev(n_pleb, n_plvb);
    gop(n_leb);
    lrbxy = 0;
    goto LB1;
LESW4: /*  ei sj . . . */
    i = x[n].i;
    if (v[i].last == 0)
        goto LE;
    gpev(n_plesc, n_plvsc);
    gopn(n_lesd, (char)v[i]._q);
    goto LSMD;
LESW5:
LESW6: /*  ei . . .    */
LE:
    gpev(n_ple, n_plv);
    gop(n_le);
    goto RCGL;
ROE:
    n = n2 - 1;
    ie = x[n].i;
/*                 attempt to extract right support group     */
RSG:
    if (rsg_p() == 1)
        goto RE_CASE;
    else
        goto RCGR;
RE_CASE:
    n = n2 - 1;
    switch (x[n].t)
    {
    case 1:
        goto RESW1;
    case 2:
        goto RESW2;
    case 3:
        goto RESW3;
    case 4:
        goto RESW4;
    case 5:
        goto RESW5;
    case 6:
        goto RESW6;
    };
RESW1: /*    . . .  'a' ei  */
    xncode.tag = x[n].code.tag;
    xncode.info.pinf = x[n].code.info.pinf;
    n--;
    if ((not_nil == 0) && (x[n].eoemrk == 1))
    {
        x[n].eoemrk = 0;
        x[n].e_level = 0;
        e_level--;
        gops(n_rsrch, &xncode);
    }
    else
    {
        gpev(n_presc, n_prvsc);
        gops(n_resc, &xncode);
    };
    n++;
    goto R1;
RESW3: /* . . .  ( . . .  ) ei  */
    gpev(n_preb, n_prvb);
    gop(n_reb);
    lrbxy = 0;
    goto RB1;
RESW4: /*  . . . sj ei  */
    i = x[n].i;
    if (v[i].last == 0)
        goto RE;
    gpev(n_presc, n_prvsc);
    gopn(n_resd, (char)v[i]._q);
    goto RSMD;
RESW5:
RESW6: /* . . .  ei   */
RE:
    gpev(n_pre, n_prv);
    gop(n_re);
    goto RCGR;
    /*                 place compiler error          */
LESW3:
RESW2:
    printf("Compiler error\n");
    exit(1);
    /*                 identification end            */
RCGFIN:
    gop(n_eor);

    /*--------------------------------------------*/
    /*         right part compilation             */
    /*--------------------------------------------*/

    ur_skob = 1;
    kol_skob[ur_skob] = 0;

    /*  read next element of right part  */

GET_RPE:
    scan();
SW_RPE:
    switch (scn_e.t_)
    {
    case 0:
        goto RPE0;
    case 1:
        goto RPE1;
    case 2:
        goto RPE2;
    case 3:
        goto RPE3;
    case 4:
        goto RPE4;
    case 5:
        goto RPE5;
    case 6:
        goto RPE6;
    case 7:
        goto RPE7;
    case 8:
        goto RPE8;
    case 9:
        goto RPE9;
    case 10:
        goto RPE10;
    default:;
    };
RPE0: /* scanner error */
    goto RP_OSH300;
RPE1: /* symbol-constant */
    if (scn_e._code.tag == TAG_O)
        goto TEXT;
    gops(n_ns, &(scn_e._code));
    goto GET_RPE;
TEXT:
    kol_lit = 0;
TEXT1:
    kol_lit++;
    buf_lit[kol_lit] = scn_e._code.info.chinf[0];
    scan();
    if ((kol_lit < 80) && (scn_e.t_ == t_sc) && (scn_e._code.tag == TAG_O))
        goto TEXT1;
    if (kol_lit == 1)
        gopn(n_nso, buf_lit[1]);
    else
    {
        gopn(n_text, kol_lit);
        for (k = 1; k <= kol_lit; k++)
            jbyte(buf_lit[k]);
    };
    goto SW_RPE;
RPE2: /* left bracket */
    scan();
    if (scn_e.t_ == t_rb)
    {
        gop(n_blr);
        goto GET_RPE;
    };
    kol_skob[ur_skob]++;
    if ((scn_e.t_ == t_sc) && (scn_e._code.tag == TAG_F))
    {
        funcptr.info.pinf = scn_e._code.info.pinf;
        gopl(n_blf, funcptr.info.pinf);
        goto GET_RPE;
    }
    else
    {
        gop(n_bl);
        goto SW_RPE;
    };
RPE3: /* right bracket */
    gop(n_br);
    if (kol_skob[ur_skob] == 0)
        pchosh("402 too many ')' in right part");
    else
        kol_skob[ur_skob]--;
    goto GET_RPE;
RPE4: /* s - varyable */
    isk_v();
    switch (v[i]._t)
    {
    case 0:
        pch406();
        break;
    case 1:
        gopn(n_muls, v[i]._q);
        break;
    default:
        pch303();
    };
    goto GET_RPE;
RPE5: /* w - varyable */
    isk_v();
    switch (v[i]._t)
    {
    case 0:
        pch406();
        break;
    case 2:
        n = v[i].last;
        if (n == 0)
            gopn(n_mule, v[i]._q);
        else
        {
            gopn(n_tplv, x[n].q);
            v[i].last = x[n].next;
        };
        break;
    default:
        pch303();
    };
    goto GET_RPE;
RPE6: /* e- or v-varyable */
    isk_v();
    if (v[i]._t == 0)
        pch406();
    else if ((v[i]._t == 3) && (v[i]._v == scn_e.v_))
    {
        n = v[i].last;
        if (n == 0)
            gopn(n_mule, v[i]._q);
        else
        {
            if (v[i]._v == 1)
                gopn(n_tplv, x[n].q);
            else
            {
                gopn(n_tple, x[n].q);
            }
            v[i].last = x[n].next;
        };
    }
    else
        pch303();
    goto GET_RPE;
RPE7: /* sign "k" */
    if (ur_skob > 511)
    {
        pchosh("407 including of the signs 'k' > 511");
        goto RP_OSH300;
    };
    kol_skob[++ur_skob] = 0;
    scan();
    if ((scn_e.t_ == t_sc) && (scn_e._code.tag == TAG_F))
    {
        funcptr.info.pinf = scn_e._code.info.pinf;
        funcptr.tag = 0;
        gopl(n_blf, funcptr.info.pinf);
        goto GET_RPE;
    }
    else
    {
        gop(n_bl);
        goto SW_RPE;
    };
RPE8: /* sign '.' */
    if (ur_skob == 1)
        pchosh("404 too many sign '.' in right part");
    else
    {
        if (kol_skob[ur_skob] != 0)
            pchosh("401 too many '(' in right part");
        gop(n_bract);
        ur_skob--;
    };
    goto GET_RPE;
RPE9: /* sign '=' in right part */
    pchosh("405 sign '=' in right part");
    goto GET_RPE;
RPE10: /* sentence end */
    scn_.curr_stmnmb++;
    if (options.stmnmb == 1)
    {
        gop(n_eossn);
        ghw(scn_.curr_stmnmb);
    }
    else
        gop(n_eos);
    if (ur_skob != 1)
        pchosh("403 too many signs 'k' in right part");
    if (kol_skob[ur_skob] != 0)
        pchosh("401 too many '(' in right part");
}
/*-----------  end of file CCST1.C  -----------*/
