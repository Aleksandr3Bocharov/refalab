//----------   file  CCST1.C  --------------
//      Compiler file
//      Last edition date : 11.07.24
//------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "refal.def"
#include "ccst.h"
#include "cerr.h"
#include "cs.h"
#include "cgop.h"
#include "cj.h"
#include "refal.h"

#define t_sc 1
#define t_lb 2
#define t_rb 3
#define t_s 4
#define t_w 5
#define t_e 6
#define t_k 7
#define t_p 8

typedef enum states
{
    GET_LPE,
    LPE0,
    LPE1,
    LPE2,
    LPE3,
    LPE4,
    LPE5,
    LPE6,
    LPE7,
    LPE8,
    LPE9,
    LPE10,
    NEXT_LPE,
    OSH300,
    RCG,
    RCGL,
    LSW1,
    LTXT,
    LTXT1,
    LTXT2,
    LTXT3,
    LSCO,
    LSW2,
    LBCE,
    LBNIL,
    GEN_LB,
    LB1,
    LSW3,
    LSW4,
    LSMD,
    L1,
    LSW5,
    LED,
    LEMD,
    L2,
    LSW6,
    RCGR,
    RSW1,
    RTXT,
    RTXT1,
    RTXT2,
    RTXT3,
    RSCO,
    RSW2,
    RBCE,
    RBNIL,
    GEN_RB,
    RB1,
    RSW3,
    RSW4,
    RSMD,
    R1,
    RSW5,
    RED,
    REMD,
    R2,
    RSW6,
    NIL,
    CE,
    CE1,
    CE2,
    IMPASSE,
    HSCH,
    NHOLE,
    NHOLE1,
    RIGID,
    OE,
    OE0,
    OERMAX,
    RMAX,
    REM,
    OELMAX,
    LMAX,
    LEM,
    OE1,
    LOE,
    LSG,
    LE_CASE,
    LESW1,
    LESW2,
    LESW3,
    LESW4,
    LESW5,
    LESW6,
    LE,
    ROE,
    RSG,
    RE_CASE,
    RESW1,
    RESW2,
    RESW3,
    RESW4,
    RESW5,
    RESW6,
    RE,
    RCGFIN,
    GET_RPE,
    SW_RPE,
    RPE0,
    RPE1,
    TEXT,
    TEXT1,
    RPE2,
    RPE3,
    RPE4,
    RPE5,
    RPE6,
    RPE7,
    RPE8,
    RPE9,
    RPE10,
    RP_OSH300,
    ERROR
} T_STATES;

static struct
{ // left part buffer elements
    uint32_t p, q, t;
    size_t ind;
    T_LINKTI code;
    size_t next;
    size_t pair;
    T_LINKTI spec;
    bool v;
    bool eoemrk;
    uint32_t e_level;
} x[100];

// ???whole list
static struct
{ // ???variable table elements
    uint32_t _t, _q;
    uint32_t rem;
    size_t last;
    char ci;
    bool _v;
} v[50];

// ???variable table elements
static struct
{
    size_t _next;
    size_t n1, n2;
} h[30];

static T_LINKTI xncode;  // work structure
static T_LINKTI funcptr; // work pointer

static size_t n, n1, n2;     // left part element pointers
static size_t ind, ie;       // element index
static uint32_t nel;         // current element number
static uint32_t e_level;     // counter of the longing levels
static bool not_nil;         // working variables
static size_t nh;            // current whole number
static size_t kol_per;       // subprogram of search in variable table
                             // table pointer
static size_t nh_x, nh_y;    // hole numbers (under enter in brackets)
static uint32_t lrbxy;       // stoped bracket flag
static size_t lastb, lastb1; // variables for brackets linkage
static size_t kol_lit;       // counter of the symbol number
static uint32_t diff_e_level;
static uint32_t kol_skob[100]; // stack for counting of the brackets balance
static size_t ur_skob;
static char buf_lit[80]; // buffer for generating of the "text" statement
static size_t k;
static size_t fh;      // free segment number in the whole  list
static size_t next_nh; // next whole number

static void isk_v(void);
static void gen_bsb(void);
static void pch303(void);
static void pch406(void);
static bool lsg_p(void);
static bool rsg_p(void);
static void gpev(char op1, char op2);
static bool ortgn(size_t on1, size_t on2);

// read left part
// and full array X
void cst(bool dir, char *lbl, size_t lblleng)
// dir;     matching feature :left to right or otherwise
// lbl;   sentence label
// lblleng; sentence label length
{
    kol_per = 0;
    n = 0;
    lastb = 0;
    nel = 0;
    T_STATES state = GET_LPE;
    while (true)
        switch (state)
        {
        case GET_LPE:
            // read left part element
            n++;
            scan();
            x[n].t = scn_e.t;
            x[n].code.tag = scn_e.code.tag;
            x[n].code.info.codef = scn_e.code.info.codef;
            x[n].spec.tag = scn_e.spec.tag;
            x[n].spec.info.codef = scn_e.spec.info.codef;
            x[n].v = scn_e.v;
            x[n].next = 0;
            x[n].pair = 0;
            x[n].eoemrk = false;
            x[n].e_level = 0;
            switch (scn_e.t)
            {
            case 0:
                state = LPE0;
                break;
            case 1:
                state = LPE1;
                break;
            case 2:
                state = LPE2;
                break;
            case 3:
                state = LPE3;
                break;
            case 4:
                state = LPE4;
                break;
            case 5:
                state = LPE5;
                break;
            case 6:
                state = LPE6;
                break;
            case 7:
                state = LPE7;
                break;
            case 8:
                state = LPE8;
                break;
            case 9:
                state = LPE9;
                break;
            case 10:
                state = LPE10;
                break;
            default:
                state = OSH300;
            }
            break;
        case LPE0:
            // scanner error
            state = OSH300;
            break;
        case LPE1:
            // constant symbol
            nel++;
            state = NEXT_LPE;
            break;
        case LPE2:
            // left bracket
            nel++;
            x[n].pair = lastb;
            lastb = n;
            state = NEXT_LPE;
            break;
        case LPE3:
            // right bracket
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
            state = NEXT_LPE;
            break;
        case LPE4:
            // s-varyable
            isk_v();
            x[n].ind = ind;
            switch (v[ind]._t)
            {
            case 0:
                v[ind]._t = 1; // yet isn't faced
                break;
            case 1:
                ++v[ind].rem; // next position
                break;
            default: // invalid type pointer
                pch303();
                n--;
            }
            nel++;
            state = NEXT_LPE;
            break;
        case LPE5:
            // w-varyable
            isk_v();
            x[n].ind = ind;
            switch (v[ind]._t)
            {
            case 0:
                v[ind]._t = 2; // yet isn't faced
                break;
            case 2:
                ++v[ind].rem; // next position
                break;
            default: // invalid type pointer
                pch303();
                n--;
            }
            nel += 2;
            state = NEXT_LPE;
            break;
        case LPE6:
            // e- or v-varyable
            isk_v();
            x[n].ind = ind;
            if (v[ind]._t == 0) // yet is't faced
                v[ind]._t = 3;
            else if (v[ind]._t == 3 && v[ind]._v == scn_e.v)
                ++v[ind].rem;
            else // invalid type pointer
            {
                pch303();
                n--;
            };
            nel += 2;
            state = NEXT_LPE;
            break;
        case LPE7:
            // sign 'k'
            pchosh("306 sign 'k' in left part");
            n--;
            state = NEXT_LPE;
            break;
        case LPE8:
            // sign '.'
            pchosh("307 sign '.' in left part");
            n--;
            state = NEXT_LPE;
            break;
        case LPE9:
            // left part end
            if (lastb == 0)
            {
                state = RCG;
                break;
            }
            pchosh("301 too many '(' in left part");
            state = OSH300;
            break;
        case LPE10:
            // sentence end
            pchosh("304 under left part default sign '=' ");
            fndef(lbl, lblleng);
            return;
        case NEXT_LPE:
            // end of element processing
            if (nel <= 252)
            {
                state = GET_LPE;
                break;
            }
            pchosh("305 very large left part");
            state = OSH300;
            break;
        case OSH300:
            fndef(lbl, lblleng);
            state = RP_OSH300;
            break;
        //--------------------------------------------
        //         left part compilation
        //--------------------------------------------
        case RCG:
            fndef(lbl, lblleng);
            n1 = 0;
            n2 = n;
            nel = 4;
            x[n1].q = 3;
            x[n1].p = x[n1].q;
            x[n2].q = 2;
            x[n2].p = x[n2].q;
            h[28]._next = 0;
            nh = 1;
            h[1]._next = 28;
            fh = 2;
            e_level = 0;
            if (dir)
            {
                state = RCGL;
                break;
            }
            state = RCGR;
            break;
        //    hard element projection from left part of current hole
        case RCGL:
            n = n1 + 1;
            if (n == n2)
            {
                state = NIL;
                break;
            }
            switch (x[n].t)
            {
            case 1:
                state = LSW1;
                break;
            case 2:
                state = LSW2;
                break;
            case 3:
                state = LSW3;
                break;
            case 4:
                state = LSW4;
                break;
            case 5:
                state = LSW5;
                break;
            case 6:
                state = LSW6;
            };
            break;
        case LSW1:
            //        constant symbol
            if (x[n].code.tag == TAGO)
            {
                state = LTXT;
                break;
            }
            gops(n_lsc, &x[n].code);
            state = L1;
            break;
        case LTXT:
            kol_lit = 1;
            state = LTXT1;
            break;
        case LTXT1:
            n++;
            if (n == n2 || x[n].t != t_sc || x[n].code.tag != TAGO)
            {
                state = LTXT2;
                break;
            }
            kol_lit++;
            break;
        case LTXT2:
            if (kol_lit == 1)
            {
                state = LSCO;
                break;
            }
            n = n1;
            gopn(n_ltxt, (char)kol_lit);
            state = LTXT3;
            break;
        case LTXT3:
            n++;
            jbyte(x[n].code.info.infoc[0]);
            x[n].q = nel;
            x[n].p = x[n].q;
            nel++;
            kol_lit--;
            if (kol_lit == 0)
            {
                n1 = n;
                state = RCGL;
            }
            break;
        case LSCO:
            n = n1 + 1;
            gopn(n_lsco, x[n].code.info.infoc[0]);
            state = L1;
            break;
        case LSW2:
            //  left bracket
            n1 = n;
            n = x[n1].pair;
            if (n1 + 1 == n)
            {
                state = LBNIL;
                break;
            }
            if (n1 + 2 != n)
            {
                state = GEN_LB;
                break;
            }
            n = n1 + 1;
            if (x[n].t != t_e)
            {
                state = GEN_LB;
                break;
            }
            ind = x[n].ind;
            if (v[ind].last != 0)
            {
                state = GEN_LB;
                break;
            }
            state = LBCE;
            break;
        case LBCE:
            nel += 2;
            jbyte(n_lbce);
            v[ind]._q = nel + 1;
            x[n].next = v[ind].last;
            v[ind].last = n;
            v[ind].rem--;
            if (x[n].v)
                jbyte(n_nnil);
            if (x[n].spec.info.codef != NULL)
                gopl(n_espc, x[n].spec.info.codef);
            x[n].p = nel;
            x[n].q = nel + 1;
            nel += 2;
            x[n1].q = nel - 4;
            x[n1].p = x[n1].q;
            n1 += 2;
            x[n1].q = nel - 3;
            x[n1].p = x[n1].q;
            state = RCGL;
            break;
        case LBNIL:
            jbyte(n_lbnil);
            x[n1].q = nel;
            x[n1].p = x[n1].q;
            n1 = n;
            x[n1].q = nel + 1;
            x[n1].p = x[n1].q;
            nel += 2;
            state = RCGL;
            break;
        case GEN_LB:
            n = n1;
            lrbxy = 1;
            state = LB1;
            break;
        case LB1:
            if (dir)
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
            x[n1].q = nel;
            x[n1].p = x[n1].q;
            x[n2].q = nel + 1;
            x[n2].p = x[n2].q;
            nel += 2;
            state = HSCH;
            break;
        case LSW3:
            state = ERROR;
            break;
        case LSW4:
            //    s-variable
            ind = x[n].ind;
            if (v[ind].last != 0)
                gopn(n_lsd, (char)v[ind]._q);
            else
            {
                jbyte(n_ls);
                v[ind]._q = nel;
            };
            state = LSMD;
            break;
        case LSMD:
            x[n].next = v[ind].last;
            v[ind].last = n;
            (v[ind].rem)--;
            if (x[n].spec.info.codef != NULL)
                gopl(n_wspc, x[n].spec.info.codef);
            state = L1;
            break;
        case L1:
            x[n].q = nel;
            x[n].p = x[n].q;
            nel++;
            n1 = n;
            state = RCGL;
            break;
        case LSW5:
            //   w-variable
            ind = x[n].ind;
            if (v[ind].last != 0)
            {
                state = LED;
                break;
            }
            jbyte(n_lw);
            v[ind]._q = nel + 1;
            x[n].next = v[ind].last;
            v[ind].last = n;
            v[ind].rem--;
            if (x[n].spec.info.codef != NULL)
                gopl(n_wspc, x[n].spec.info.codef);
            state = L2;
            break;
        case LED:
            gopn(n_led, (char)v[ind]._q);
            state = LEMD;
            break;
        case LEMD:
            x[n].next = v[ind].last;
            v[ind].last = n;
            v[ind].rem--;
            if (x[n].spec.info.codef != NULL)
                gopl(n_espc, x[n].spec.info.codef);
            state = L2;
            break;
        case L2:
            x[n].p = nel;
            x[n].q = nel + 1;
            nel += 2;
            n1 = n;
            state = RCGL;
            break;
        case LSW6:
            //   e-variable
            ind = x[n].ind;
            if (v[ind].last != 0)
            {
                state = LED;
                break;
            }
            if (dir)
            {
                state = RCGR;
                break;
            }
            if (n + 1 == n2)
            {
                state = CE;
                break;
            }
            state = IMPASSE;
            break;
        //                 hard element projection
        //                 from  right board of
        //                    current hole
        case RCGR:
            n = n2 - 1;
            if (n == n1)
            {
                state = NIL;
                break;
            }
            switch (x[n].t)
            {
            case 1:
                state = RSW1;
                break;
            case 2:
                state = RSW2;
                break;
            case 3:
                state = RSW3;
                break;
            case 4:
                state = RSW4;
                break;
            case 5:
                state = RSW5;
                break;
            case 6:
                state = RSW6;
            };
            break;
        case RSW1:
            //   constant symbol
            if (x[n].code.tag == TAGO)
            {
                state = RTXT;
                break;
            }
            gops(n_rsc, &x[n].code);
            state = R1;
            break;
        case RTXT:
            kol_lit = 1;
            state = RTXT1;
            break;
        case RTXT1:
            n--;
            if (n == n1 || x[n].t != t_sc || x[n].code.tag != TAGO)
            {
                state = RTXT2;
                break;
            }
            kol_lit++;
            break;
        case RTXT2:
            if (kol_lit == 1)
            {
                state = RSCO;
                break;
            }
            n = n2;
            gopn(n_rtxt, (char)kol_lit);
            state = RTXT3;
            break;
        case RTXT3:
            n--;
            jbyte(x[n].code.info.infoc[0]);
            x[n].q = nel;
            x[n].p = x[n].q;
            nel++;
            kol_lit--;
            if (kol_lit == 0)
            {
                n2 = n;
                state = RCGR;
            }
            break;
        case RSCO:
            n = n2 - 1;
            gopn(n_rsco, x[n].code.info.infoc[0]);
            state = R1;
            break;
        case RSW2:
            state = ERROR;
            break;
        case RSW3:
            //     right bracket
            n2 = n;
            n = x[n2].pair;
            if (n + 1 == n2)
            {
                state = RBNIL;
                break;
            }
            if (n + 2 != n2)
            {
                state = GEN_RB;
                break;
            }
            n = n2 - 1;
            if (x[n].t != t_e)
            {
                state = GEN_RB;
                break;
            }
            ind = x[n].ind;
            if (v[ind].last != 0)
            {
                state = GEN_RB;
                break;
            }
            state = RBCE;
            break;
        case RBCE:
            nel += 2;
            jbyte(n_rbce);
            v[ind]._q = nel + 1;
            x[n].next = v[ind].last;
            v[ind].last = n;
            v[ind].rem--;
            if (x[n].v)
                jbyte(n_nnil);
            if (x[n].spec.info.codef != NULL)
                gopl(n_espc, x[n].spec.info.codef);
            x[n].p = nel;
            x[n].q = nel + 1;
            nel += 2;
            x[n2].q = nel - 3;
            x[n2].p = x[n2].q;
            n2 -= 2;
            x[n2].q = nel - 4;
            x[n2].p = x[n2].q;
            state = RCGR;
            break;
        case RBNIL:
            jbyte(n_rbnil);
            x[n2].q = nel + 1;
            x[n2].p = x[n2].q;
            n2 = n;
            x[n2].q = nel;
            x[n2].p = x[n2].q;
            nel += 2;
            state = RCGR;
            break;
        case GEN_RB:
            n = n2;
            lrbxy = 2;
            state = RB1;
            break;
        case RB1:
            if (!dir)
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
            x[n1].q = nel;
            x[n1].p = x[n1].q;
            x[n2].q = nel + 1;
            x[n2].p = x[n2].q;
            nel += 2;
            state = HSCH;
            break;
        case RSW4:
            //     s_variable
            ind = x[n].ind;
            if (v[ind].last != 0)
                gopn(n_rsd, (char)v[ind]._q);
            else
            {
                jbyte(n_rs);
                v[ind]._q = nel;
            };
            state = RSMD;
            break;
        case RSMD:
            x[n].next = v[ind].last;
            v[ind].last = n;
            v[ind].rem--;
            if (x[n].spec.info.codef != NULL)
                gopl(n_wspc, x[n].spec.info.codef);
            state = R1;
            break;
        case R1:
            x[n].q = nel;
            x[n].p = x[n].q;
            nel++;
            n2 = n;
            state = RCGR;
            break;
        case RSW5:
            //    w_variable
            ind = x[n].ind;
            if (v[ind].last != 0)
            {
                state = RED;
                break;
            }
            jbyte(n_rw);
            v[ind]._q = nel + 1;
            x[n].next = v[ind].last;
            v[ind].last = n;
            v[ind].rem--;
            if (x[n].spec.info.codef != NULL)
                gopl(n_wspc, x[n].spec.info.codef);
            state = R2;
            break;
        case RED:
            gopn(n_red, (char)v[ind]._q);
            state = REMD;
            break;
        case REMD:
            x[n].next = v[ind].last;
            v[ind].last = n;
            v[ind].rem--;
            if (x[n].spec.info.codef != NULL)
                gopl(n_espc, x[n].spec.info.codef);
            state = R2;
            break;
        case R2:
            x[n].p = nel;
            x[n].q = nel + 1;
            nel += 2;
            n2 = n;
            state = RCGR;
            break;
        case RSW6:
            //    e-variable
            ind = x[n].ind;
            if (v[ind].last != 0)
            {
                state = RED;
                break;
            }
            if (!dir)
            {
                state = RCGL;
                break;
            }
            if (n1 + 1 == n)
            {
                state = CE;
                break;
            }
            state = IMPASSE;
            break;
        case NIL:
            //     empty hole
            jbyte(n_nil);
            next_nh = h[nh]._next;
            h[nh]._next = h[next_nh]._next;
            h[nh].n1 = h[next_nh].n1;
            h[nh].n2 = h[next_nh].n2;
            nh = next_nh;
            state = IMPASSE;
            break;
        case CE:
            //   closed including
            if (x[n].eoemrk)
            {
                state = IMPASSE;
                break;
            }
            state = CE1;
            break;
        case CE1:
            if (x[n].spec.info.codef == NULL)
            {
                state = CE2;
                break;
            }
            if (dir)
            {
                state = LMAX;
                break;
            }
            state = RMAX;
            break;
        case CE2:
            ind = x[n].ind;
            v[ind]._q = nel + 1;
            jbyte(n_ce);
            x[n].next = v[ind].last;
            v[ind].last = n;
            v[ind].rem--;
            if (x[n].v)
                jbyte(n_nnil);
            x[n].p = nel;
            x[n].q = nel + 1;
            nel += 2;
            next_nh = h[nh]._next;
            h[nh]._next = h[next_nh]._next;
            h[nh].n1 = h[next_nh].n1;
            h[nh].n2 = h[next_nh].n2;
            nh = next_nh;
            state = IMPASSE;
            break;
        //
        //          It is impossible movement
        //          on hard element here or
        //          hole ended here
        case IMPASSE:
            lrbxy = 0;
            nh_x = nh;
            state = HSCH;
            break;
        //          Search of hole with hard
        //          elements on its boards.
        //          If it not exist than project
        //          e-variable from first hole.
        case HSCH:
            h[nh].n1 = n1;
            h[nh].n2 = n2;
            nh = 1;
            if (h[nh]._next == 0)
            {
                state = RCGFIN;
                break;
            }
            state = NHOLE;
            break;
        case NHOLE:
            n1 = h[nh].n1;
            n2 = h[nh].n2;
            n = n1 + 1;
            if (n == n2)
            {
                gen_bsb();
                state = NIL;
                break;
            }
            if (x[n].t != t_e)
            {
                state = RIGID;
                break;
            }
            ind = x[n].ind;
            if (v[ind].last != 0)
            {
                state = RIGID;
                break;
            }
            if (n + 1 == n2)
            {
                if (v[ind].rem == 1)
                {
                    state = NHOLE1;
                    break;
                }
                gen_bsb();
                state = CE1;
                break;
            }
            n = n2 - 1;
            if (x[n].t != t_e)
            {
                state = RIGID;
                break;
            }
            ind = x[n].ind;
            if (v[ind].last != 0)
            {
                state = RIGID;
                break;
            }
            state = NHOLE1;
            break;
        case NHOLE1:
            nh = h[nh]._next;
            if (h[nh]._next == 0)
            {
                state = OE;
                break;
            }
            state = NHOLE;
            break;
        case RIGID:
            //  hard element on the both hole boards
            gen_bsb();
            if (dir)
            {
                state = RCGL;
                break;
            }
            state = RCGR;
            break;
        //  opened e_variable processing
        case OE:
            nh = 1;
            n1 = h[nh].n1;
            n2 = h[nh].n2;
            gen_bsb();
            if (dir)
                n = n1 + 1;
            else
                n = n2 - 1;
            ind = x[n].ind;
            if (x[n].eoemrk)
            {
                diff_e_level = e_level - x[n].e_level;
                if (diff_e_level == 1)
                    jbyte(n_eoei);
                else
                    gopn(n_eoe, (char)diff_e_level);
                e_level = x[n].e_level;
                x[n].eoemrk = false;
                x[n].e_level = 0;
            };
            if (n1 + 2 == n2)
            {
                state = CE1;
                break;
            }
            if (dir)
                n = n2 - 1;
            else
                n = n1 + 1;
            if (x[n].spec.info.codef == NULL)
            {
                state = OE1;
                break;
            }
            state = OE0;
            break;
        case OE0:
            ind = x[n].ind;
            if (v[ind].last != 0 || v[ind].rem != 1)
            {
                state = OE1;
                break;
            }
            if (dir)
            {
                n--;
                if (n == n1)
                {
                    state = OERMAX;
                    break;
                }
            }
            else
            {
                n++;
                if (n == n2)
                {
                    state = OELMAX;
                    break;
                }
            };
            if (x[n].t != t_e || x[n].v)
            {
                state = OE1;
                break;
            }
            state = OE0;
            break;
        case OERMAX:
            n = n2 - 1;
            ind = x[n].ind;
            state = RMAX;
            break;
        case RMAX:
            gopl(n_rmax, x[n].spec.info.codef);
            if (x[n].v)
                jbyte(n_nnil);
            x[n].spec.info.codef = NULL;
            state = REM;
            break;
        case REM:
            v[ind]._q = nel + 1;
            state = REMD;
            break;
        case OELMAX:
            n = n1 + 1;
            ind = x[n].ind;
            state = LMAX;
            break;
        case LMAX:
            gopl(n_lmax, x[n].spec.info.codef);
            if (x[n].v)
                jbyte(n_nnil);
            x[n].spec.info.codef = NULL;
            state = LEM;
            break;
        case LEM:
            v[ind]._q = nel + 1;
            state = LEMD;
            break;
        case OE1:
            if (dir)
            {
                state = LOE;
                break;
            }
            state = ROE;
            break;
        case LOE:
            n = n1 + 1;
            ie = x[n].ind;
            state = LSG;
            break;
        //         attempt to extract left support group
        case LSG:
            if (lsg_p())
            {
                state = LE_CASE;
                break;
            }
            state = RCGL;
            break;
        case LE_CASE:
            n = n1 + 1;
            switch (x[n].t)
            {
            case 1:
                state = LESW1;
                break;
            case 2:
                state = LESW2;
                break;
            case 3:
                state = LESW3;
                break;
            case 4:
                state = LESW4;
                break;
            case 5:
                state = LESW5;
                break;
            case 6:
                state = LESW6;
            };
            break;
        case LESW1:
            //  ei 'a' . . .
            xncode.tag = x[n].code.tag;
            xncode.info.codef = x[n].code.info.codef;
            n++;
            if (!not_nil && x[n].eoemrk)
            {
                x[n].eoemrk = false;
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
            state = L1;
            break;
        case LESW2:
            //   ei ( . . . ) . . .
            gpev(n_pleb, n_plvb);
            jbyte(n_leb);
            lrbxy = 0;
            state = LB1;
            break;
        case LESW3:
            state = ERROR;
            break;
        case LESW4:
            //  ei sj . . .
            ind = x[n].ind;
            if (v[ind].last == 0)
            {
                state = LE;
                break;
            }
            gpev(n_plesc, n_plvsc);
            gopn(n_lesd, (char)v[ind]._q);
            state = LSMD;
            break;
        case LESW5:
        case LESW6:
            //  ei . . .
            state = LE;
            break;
        case LE:
            gpev(n_ple, n_plv);
            jbyte(n_le);
            state = RCGL;
            break;
        case ROE:
            n = n2 - 1;
            ie = x[n].ind;
            state = RSG;
            break;
        //                 attempt to extract right support group
        case RSG:
            if (rsg_p())
            {
                state = RE_CASE;
                break;
            }
            state = RCGR;
            break;
        case RE_CASE:
            n = n2 - 1;
            switch (x[n].t)
            {
            case 1:
                state = RESW1;
                break;
            case 2:
                state = RESW2;
                break;
            case 3:
                state = RESW3;
                break;
            case 4:
                state = RESW4;
                break;
            case 5:
                state = RESW5;
                break;
            case 6:
                state = RESW6;
            };
            break;
        case RESW1:
            //    . . .  'a' ei
            xncode.tag = x[n].code.tag;
            xncode.info.codef = x[n].code.info.codef;
            n--;
            if (!not_nil && x[n].eoemrk)
            {
                x[n].eoemrk = false;
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
            state = R1;
            break;
        case RESW2:
            state = ERROR;
            break;
        case RESW3:
            // . . .  ( . . .  ) ei
            gpev(n_preb, n_prvb);
            jbyte(n_reb);
            lrbxy = 0;
            state = RB1;
            break;
        case RESW4:
            //  . . . sj ei
            ind = x[n].ind;
            if (v[ind].last == 0)
            {
                state = RE;
                break;
            }
            gpev(n_presc, n_prvsc);
            gopn(n_resd, (char)v[ind]._q);
            state = RSMD;
            break;
        case RESW5:
        case RESW6:
            // . . .  ei
            state = RE;
            break;
        case RE:
            gpev(n_pre, n_prv);
            jbyte(n_re);
            state = RCGR;
            break;
        //                 identification end
        case RCGFIN:
            jbyte(n_eor);
            //--------------------------------------------
            //         right part compilation
            //--------------------------------------------
            ur_skob = 1;
            kol_skob[ur_skob] = 0;
            state = GET_RPE;
            break;
        //  read next element of right part
        case GET_RPE:
            scan();
            state = SW_RPE;
            break;
        case SW_RPE:
            switch (scn_e.t)
            {
            case 0:
                state = RPE0;
                break;
            case 1:
                state = RPE1;
                break;
            case 2:
                state = RPE2;
                break;
            case 3:
                state = RPE3;
                break;
            case 4:
                state = RPE4;
                break;
            case 5:
                state = RPE5;
                break;
            case 6:
                state = RPE6;
                break;
            case 7:
                state = RPE7;
                break;
            case 8:
                state = RPE8;
                break;
            case 9:
                state = RPE9;
                break;
            case 10:
                state = RPE10;
                break;
            default:
                state = RP_OSH300;
            };
            break;
        case RPE0:
            // scanner error
            state = RP_OSH300;
            break;
        case RPE1:
            // symbol-constant
            if (scn_e.code.tag == TAGO)
            {
                state = TEXT;
                break;
            }
            gops(n_ns, &scn_e.code);
            state = GET_RPE;
            break;
        case TEXT:
            kol_lit = 0;
            state = TEXT1;
            break;
        case TEXT1:
            kol_lit++;
            buf_lit[kol_lit] = scn_e.code.info.infoc[0];
            scan();
            if (kol_lit < 80 && scn_e.t == t_sc && scn_e.code.tag == TAGO)
                break;
            if (kol_lit == 1)
                gopn(n_nso, buf_lit[1]);
            else
            {
                gopn(n_text, (char)kol_lit);
                for (k = 1; k <= kol_lit; k++)
                    jbyte(buf_lit[k]);
            };
            state = SW_RPE;
            break;
        case RPE2:
            // left bracket
            scan();
            if (scn_e.t == t_rb)
            {
                jbyte(n_blr);
                state = GET_RPE;
                break;
            }
            kol_skob[ur_skob]++;
            if (scn_e.t == t_sc && scn_e.code.tag == TAGF)
            {
                funcptr.info.codef = scn_e.code.info.codef;
                gopl(n_blf, funcptr.info.codef);
                state = GET_RPE;
                break;
            }
            jbyte(n_bl);
            state = SW_RPE;
            break;
        case RPE3:
            // right bracket
            jbyte(n_br);
            if (kol_skob[ur_skob] == 0)
                pchosh("402 too many ')' in right part");
            else
                kol_skob[ur_skob]--;
            state = GET_RPE;
            break;
        case RPE4:
            // s - varyable
            isk_v();
            switch (v[ind]._t)
            {
            case 0:
                pch406();
                break;
            case 1:
                gopn(n_muls, (char)v[ind]._q);
                break;
            default:
                pch303();
            };
            state = GET_RPE;
            break;
        case RPE5:
            // w - varyable
            isk_v();
            switch (v[ind]._t)
            {
            case 0:
                pch406();
                break;
            case 2:
                n = v[ind].last;
                if (n == 0)
                    gopn(n_mule, (char)v[ind]._q);
                else
                {
                    gopn(n_tplv, (char)x[n].q);
                    v[ind].last = x[n].next;
                };
                break;
            default:
                pch303();
            };
            state = GET_RPE;
            break;
        case RPE6:
            // e- or v-varyable
            isk_v();
            if (v[ind]._t == 0)
                pch406();
            else if (v[ind]._t == 3 && v[ind]._v == scn_e.v)
            {
                n = v[ind].last;
                if (n == 0)
                    gopn(n_mule, (char)v[ind]._q);
                else
                {
                    if (v[ind]._v)
                        gopn(n_tplv, (char)x[n].q);
                    else
                        gopn(n_tple, (char)x[n].q);
                    v[ind].last = x[n].next;
                };
            }
            else
                pch303();
            state = GET_RPE;
            break;
        case RPE7:
            // sign "k"
            if (ur_skob > 511)
            {
                pchosh("407 including of the signs 'k' > 511");
                state = RP_OSH300;
                break;
            }
            kol_skob[++ur_skob] = 0;
            scan();
            if (scn_e.t == t_sc && scn_e.code.tag == TAGF)
            {
                funcptr.info.codef = scn_e.code.info.codef;
                funcptr.tag = TAGO;
                gopl(n_blf, funcptr.info.codef);
                state = GET_RPE;
                break;
            }
            jbyte(n_bl);
            state = SW_RPE;
            break;
        case RPE8:
            // sign '.'
            if (ur_skob == 1)
                pchosh("404 too many sign '.' in right part");
            else
            {
                if (kol_skob[ur_skob] != 0)
                    pchosh("401 too many '(' in right part");
                jbyte(n_bract);
                ur_skob--;
            };
            state = GET_RPE;
            break;
        case RPE9:
            // sign '=' in right part
            pchosh("405 sign '=' in right part");
            state = GET_RPE;
            break;
        case RPE10:
            // sentence end
            // scn_.curr_stmnmb++;
            /*if (options.stmnmb)
            {
                jbyte(n_eossn);
                ghw(scn_.curr_stmnmb);
            }
            else*/
            jbyte(n_eos);
            if (ur_skob != 1)
                pchosh("403 too many signs 'k' in right part");
            if (kol_skob[ur_skob] != 0)
                pchosh("401 too many '(' in right part");
            return;
        case RP_OSH300:
            pchosh("300 sentence is't scanned");
            return;
        //                      place of compiler's error
        case ERROR:
            printf("Compiler's error\n");
            exit(1);
            return;
        }
}

static void isk_v(void)
{
    for (ind = 1; ind <= kol_per; ind++)
        if (v[ind].ci == scn_e.ci)
            return;
    ind = ++kol_per;
    v[ind].ci = scn_e.ci;
    v[ind]._t = 0;
    v[ind].rem = 1;
    v[ind].last = 0;
    v[ind]._v = scn_e.v;
    return;
}

//   generation of stoped brackets and setting boards
static void gen_bsb(void)
{
    switch (lrbxy)
    {
    case 0: // no stoped brackets
        if (nh != nh_x)
            break;
        return;
    case 1: // left stoped brackets
        if (nh == nh_x)
            jbyte(n_lb);
        else if (nh == nh_y)
            jbyte(n_lby);
        else
        {
            jbyte(n_lb);
            break;
        };
        return;
    case 2: // right stoped brackets
        if (nh == nh_x)
            jbyte(n_rb);
        else if (nh == nh_y)
            jbyte(n_rby);
        else
        {
            jbyte(n_rb);
            break;
        };
        return;
    };
    gopnm(n_sb1b2, (char)x[n1].q, (char)x[n2].p);
    return;
}

static void pch303(void)
{
    pchosa("303 differents for variable ", v[ind].ci);
    return;
}

static void pch406(void)
{
    pchosa("406 in left part missing variable ", v[ind].ci);
    return;
}

//    attempt to extract left support group
static bool lsg_p(void)
{
    while (true)
    {
        n++;
        if (n == n2)
            break;
        if (x[n].t != t_lb)
        {
            if (x[n].t != t_e)
                continue;
            ind = x[n].ind;
            if (ind == ie || v[ind].last != 0)
                continue;
            if (x[n].spec.info.codef != NULL || v[ind].rem != 1)
                break;
            if (!ortgn(n1, n))
                break;
            x[n].eoemrk = true;
            x[n].e_level = e_level;
            break;
        }
        n = x[n].pair;
    }
    n1++;
    n = n1;
    ind = x[n].ind;
    v[ind]._q = nel + 1;
    x[n].next = v[ind].last;
    v[ind].last = n;
    v[ind].rem--;
    x[n].p = nel;
    x[n].q = nel + 1;
    nel += 2;
    e_level++;
    not_nil = x[n].v;
    if (x[n].spec.info.codef == NULL)
        return true;
    gpev(n_plespc, n_plv);
    gopl(n_lespc, x[n].spec.info.codef);
    return false;
}

//        attempt to extract right support group
static bool rsg_p(void)
{
    while (true)
    {
        n--;
        if (n == n1)
            break;
        if (x[n].t != t_rb)
        {
            if (x[n].t != t_e)
                continue;
            ind = x[n].ind;
            if (ind == ie || v[ind].last != 0)
                continue;
            if (x[n].spec.info.codef != NULL || v[ind].rem != 1)
                break;
            if (!ortgn(n, n2))
                break;
            x[n].eoemrk = true;
            x[n].e_level = e_level;
            break;
        }
        n = x[n].pair;
    }
    n2--;
    n = n2;
    ind = x[n].ind;
    v[ind]._q = nel + 1;
    x[n].next = v[ind].last;
    v[ind].last = n;
    v[ind].rem--;
    x[n].p = nel;
    x[n].q = nel + 1;
    nel += 2;
    e_level++;
    not_nil = x[n].v;
    if (x[n].spec.info.codef == NULL)
        return true;
    gpev(n_prespc, n_prv);
    gopl(n_respc, x[n].spec.info.codef);
    return false;
}

//    check ortogonality of this sentence against left part
static bool ortgn(size_t on1, size_t on2)
{
    size_t on = on1;
    size_t i;
    while (true)
    {
        on++;
        if (on == on2)
            break;
        if (x[on].t <= 3)
            continue;
        i = x[on].ind;
        if (v[i].last != 0)
            continue;
        v[i].rem--;
    }
    bool res = true;
    on = on1;
    while (true)
    {
        on++;
        if (on == on2)
            break;
        if (x[on].t <= 3)
            continue;
        i = x[on].ind;
        if (v[i].last != 0)
            continue;
        if (v[i].rem == 0)
            continue;
        res = false;
        break;
    };
    on = on1;
    while (true)
    {
        on++;
        if (on == on2)
            break;
        if (x[on].t <= 3)
            continue;
        i = x[on].ind;
        if (v[i].last != 0)
            continue;
        v[i].rem++;
    }
    return res;
}

static void gpev(char op1, char op2)
{
    if (not_nil)
        jbyte(op2);
    else
        jbyte(op1);
}
