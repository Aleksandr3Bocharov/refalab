//----------- file RFRUN1.c -------------------
//      Refal-interpretator (part 1)
//      Last edition date: 11.07.2024
//---------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "refal.def"
#include "rfrun1.h"
#include "rfintf.h"
#include "rfrun2.h"

// for unlooping:

#define NMBL sizeof(char)
#define SHB1            \
    b1 = b1->next;      \
    if (b1 == b2)       \
    {                   \
        i_state = FAIL; \
        break;          \
    }
#define SHB2            \
    b2 = b2->prev;      \
    if (b2 == b1)       \
    {                   \
        i_state = FAIL; \
        break;          \
    }
#define SHF             \
    f = f->next;        \
    if (f == flhead)    \
    {                   \
        i_state = LACK; \
        break;          \
    }

typedef enum i_states
{
    START,
    ADVSTEP,
    REF,
    DONE,
    RCGIMP,
    LACK,
    EXIT,
    NEXTOP,
    ADVANCE,
    SJUMP,
    FAIL,
    SB,
    LSC,
    RSC,
    LSCO,
    RSCO,
    LSD,
    RSD,
    LTXT,
    LTXT1,
    RTXT,
    RTXT1,
    LB,
    LBY,
    RB,
    RBY,
    LS,
    RS,
    LW,
    RW,
    LBNIL,
    RBNIL,
    LBCE,
    RBCE,
    NIL,
    CE,
    LED,
    LED1,
    LED2,
    RED,
    RED1,
    RED2,
    NNIL,
    PLE,
    PLV,
    LE,
    PRE,
    PRV,
    RE,
    PLESC,
    PLVSC,
    LESC,
    LESC0,
    LESC1,
    PRESC,
    PRVSC,
    RESC,
    RESC0,
    RESC1,
    LESD,
    RESD,
    PLEB,
    PLVB,
    LEB,
    LEB1,
    PREB,
    PRVB,
    REB,
    REB1,
    EOE,
    EOEI,
    LSRCH,
    LSRCH1,
    RSRCH,
    RSRCH1,
    WSPC,
    ESPC,
    PLESPC,
    LESPC,
    PRESPC,
    RESPC,
    LMAX,
    RMAX,
    EOR,
    NS,
    NSO,
    TEXT,
    BL,
    BR,
    BLR,
    BRACT,
    ACT,
    MULS,
    MULE,
    TPL,
    TPLM,
    TPLE,
    TPLV,
    TPLS,
    EOS,
    SWAP,
    SWAPREF,
    BLF,
    EOSSN,
    SETNOS,
    CFUNC,
    CFDONE,
    CFLUCK
} T_I_STATES;

typedef struct sav_
{ // save area for var-part of REFAL-block
    unsigned int upshot_;
    T_LINKCB *preva_;
    T_LINKCB *nexta_;
    T_LINKCB *prevr_;
    T_LINKCB *nextr_;
    T_ST *currst_;
} T_SAV;

static union
{
    uint8_t c[2];
    unsigned int ii;
} u;

static union
{ // structure for pointer and integer aligning
    uint8_t *ptr;
    uint32_t *inr;
    char chr[2];
} inch;

static T_LINKCB *et[256]; // element table
static unsigned int nel;  // adress of first free string in element table

static T_WJS js[64]; // jump stack and planning translation stack
static T_WJS *jsp;   // jump stack pointer

static T_TS *tsp; // translation stack pointer

static unsigned int tmmod; // timer state
static uint32_t tmstart;   // time at the start
static uint32_t tmstop;    // time at the end

// definition of work variables and pointers
static uint8_t opc;     // current statement code
static uint8_t *vpc;    // virtual program counter
static T_LINKCB *lastk; // last acted sign-k adress
static T_LINKCB *lastb; // last generated left bracket
static T_LINKCB *b0, *b1, *b2;
static T_LINKCB *f0, *f1, *f;
static const uint8_t *vpca; // additional vpc
static unsigned int i, n, m;

static char (*fptr)(T_REFAL *);

void rfrun(T_ST *ast) // adress of current state table
{
    // dynamic area DSA
    T_SAV *savecr = malloc(sizeof(T_SAV));
    u.ii = 0;
    if (!lexist(ast))
        rfabe("rfrun: attempt to run unexisting process");
    if (ast->state == 4)
        rfabe("rfrun: attampt to run process in state 4");

    // saving part of refal-block

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
    T_LINKCB quasik; // quasi-sign k
    quasik.info.codep = ast->dot;
    // adress of free memory list head
    T_LINKCB *flhead = refal.flhead;
    if (tmmod == 1)
        tmstart = time(0l);
    T_I_STATES i_state = START;
    while (true)
        switch (i_state)
        {
            // start of next step
        case START:
            if (ast->step >= ast->stop)
            {
                i_state = DONE;
                break;
            }
            b2 = quasik.info.codep;
            if (b2 == NULL)
            {
                i_state = DONE;
                break;
            }
            b0 = b2->info.codep;
            b1 = b0->next;
            vpc = (uint8_t *)b1->info.codef;
            if (b1->tag != TAGF)
            {
                i_state = REF;
                break;
            }
            // here must be check on c-function
            // if (c) goto CFUNC;
            jsp = js;
            et[1] = b0;
            et[2] = b2;
            et[3] = b1;
            nel = 4;
            i_state = NEXTOP;
            break;
            // increase step number
        case ADVSTEP:
            (ast->step)++;
            i_state = START;
            break;
            // symbol - reference execution
        case REF:
            if (b1->tag != TAGR)
            {
                i_state = RCGIMP;
                break;
            }
            et[1] = b0;
            et[2] = b2;
            et[3] = b1;
            f = (T_LINKCB *)vpc;
            i_state = SWAPREF;
            break;
            // interpreter exits
            // store state
        case DONE:
            ast->state = 1; // normal end
            i_state = EXIT;
            break;
        case RCGIMP:
            ast->state = 2; // recognition impossible
            i_state = EXIT;
            break;
        case LACK:
            ast->state = 3; // end of free memory
            quasik.info.codep = et[2];
            i_state = EXIT;
            break;
            // state remove from
        case EXIT:
            if (tmmod == 1)
                tmstop = time(0l);
            ast->dot = quasik.info.codep;
            // restore REFAL-block
            refal.upshot = savecr->upshot_;
            refal.preva = savecr->preva_;
            refal.nexta = savecr->nexta_;
            refal.prevr = savecr->prevr_;
            refal.nextr = savecr->nextr_;
            refal.currst = savecr->currst_;
            refal.tmmode = tmmod;
            free(savecr);
            if (tmmod == 1)
                //{
                // printf("\nn=%ld k=%ld",tmstart,tmstop);
                refal.tmintv = (tmstop - tmstart) * 1000000l;
            //}
            return;
        case NEXTOP:
            opc = *vpc;
#ifdef mdebug
            printf("\n switcher");
            // BLF 03.07.2004
            printf(" code8=%o\t(D=%d,H=%x)", opc, opc, opc);
#endif
            // operation switcher
            //       OPSWITCH
            //  printf("\n  ego opc=%o NMBL=%o LBLL=%o SMBL=%o",opc,NMBL,LBLL,SMBL);
            switch (opc)
            {
            case 0000:
                i_state = ADVANCE;
                break;
            case 0001:
                i_state = SJUMP;
                break;
            case 0002:
                i_state = FAIL;
                break;
            case 0003:
                i_state = SB;
                break;
            case 0004:
                i_state = LSC;
                break;
            case 0005:
                i_state = RSC;
                break;
            case 0006:
                i_state = LSCO;
                break;
            case 0007:
                i_state = RSCO;
                break;
            case 0010:
                i_state = LSD;
                break;
            case 0011:
                i_state = RSD;
                break;
            case 0012:
                i_state = LTXT;
                break;
            case 0013:
                i_state = RTXT;
                break;
            case 0014:
                i_state = LB;
                break;
            case 0015:
                i_state = LBY;
                break;
            case 0016:
                i_state = RB;
                break;
            case 0017:
                i_state = RBY;
                break;
            case 0020:
                i_state = LS;
                break;
            case 0021:
                i_state = RS;
                break;
            case 0022:
                i_state = LW;
                break;
            case 0023:
                i_state = RW;
                break;
            case 0024:
                i_state = LBNIL;
                break;
            case 0025:
                i_state = RBNIL;
                break;
            case 0026:
                i_state = LBCE;
                break;
            case 0027:
                i_state = RBCE;
                break;
            case 0030:
                i_state = NIL;
                break;
            case 0031:
                i_state = CE;
                break;
            case 0032:
                i_state = LED;
                break;
            case 0033:
                i_state = RED;
                break;
            case 0034:
                i_state = NNIL;
                break;
            case 0035:
                i_state = PLE;
                break;
            case 0036:
                i_state = PLV;
                break;
            case 0037:
                i_state = LE;
                break;
            case 0040:
                i_state = PRE;
                break;
            case 0041:
                i_state = PRV;
                break;
            case 0042:
                i_state = RE;
                break;
            case 0043:
                i_state = PLESC;
                break;
            case 0044:
                i_state = PLVSC;
                break;
            case 0045:
                i_state = LESC;
                break;
            case 0046:
                i_state = PRESC;
                break;
            case 0047:
                i_state = PRVSC;
                break;
            case 0050:
                i_state = RESC;
                break;
            case 0051:
                i_state = LESD;
                break;
            case 0052:
                i_state = RESD;
                break;
            case 0053:
                i_state = PLEB;
                break;
            case 0054:
                i_state = PLVB;
                break;
            case 0055:
                i_state = LEB;
                break;
            case 0056:
                i_state = PREB;
                break;
            case 0057:
                i_state = PRVB;
                break;
            case 0060:
                i_state = REB;
                break;
            case 0061:
                i_state = EOE;
                break;
            case 0062:
                i_state = EOEI;
                break;
            case 0063:
                i_state = LSRCH;
                break;
            case 0064:
                i_state = RSRCH;
                break;
            case 0065:
                i_state = WSPC;
                break;
            case 0066:
                i_state = ESPC;
                break;
            case 0067:
                i_state = PLESPC;
                break;
            case 0070:
                i_state = LESPC;
                break;
            case 0071:
                i_state = PRESPC;
                break;
            case 0072:
                i_state = RESPC;
                break;
            case 0073:
                i_state = LMAX;
                break;
            case 0074:
                i_state = RMAX;
                break;
            case 0075:
                i_state = EOR;
                break;
            case 0076:
                i_state = NS;
                break;
            case 0077:
                i_state = NSO;
                break;
            case 0100:
                i_state = TEXT;
                break;
            case 0101:
                i_state = BL;
                break;
            case 0102:
                i_state = BR;
                break;
            case 0103:
                i_state = BLR;
                break;
            case 0104:
                i_state = BRACT;
                break;
            case 0105:
                i_state = ACT;
                break;
            case 0106:
                i_state = MULS;
                break;
            case 0107:
                i_state = MULE;
                break;
            case 0110:
                i_state = TPL;
                break;
            case 0111:
                i_state = TPLM;
                break;
            case 0112:
                i_state = TPLE;
                break;
            case 0113:
                i_state = TPLV;
                break;
            case 0114:
                i_state = TPLS;
                break;
            case 0115:
                i_state = EOS;
                break;
            case 0116:
                i_state = SWAP;
                break;
            case 0117:
                i_state = BLF;
                break;
            case 0120:
                i_state = EOSSN;
                break;
            case 0121:
                i_state = SETNOS;
                break;
            case 0122:
                i_state = CFUNC;
            };
            break;
        }

    // select next statement assembly language
ADVANCE:
    vpc = vpc + NMBL;
    goto NEXTOP;
    // SJUMP(L);
SJUMP:
    move(LBLL, vpc + NMBL, (uint8_t *)&(inch.ptr));
    putjs(jsp, &b1, &b2, &nel, &(inch.ptr));
    jsp++;
    vpc = vpc + NMBL + LBLL;
    goto NEXTOP;
    // FAIL;
FAIL:
    if (jsp == js)
        goto RCGIMP;
    jsp--;
    getjs(jsp, &b1, &b2, &nel, &vpc);
    goto NEXTOP;
    // SB(N,M);
SB:
    n = (unsigned int)*(vpc + NMBL);
    m = (unsigned int)*(vpc + NMBL + NMBL);
    b1 = et[n];
    b2 = et[m];
    vpc = vpc + 3 * NMBL;
    goto NEXTOP;
    // LSC(S);
LSC:
    SHB1 if (!cmpr(SMBL, vpc + NMBL, (uint8_t *)&(b1->tag))) goto FAIL;
    vpc = vpc + NMBL + SMBL;
    et[nel] = b1;
    nel++;
    goto NEXTOP;
    // RSC(S);
RSC:
    SHB2 if (!cmpr(SMBL, vpc + NMBL, (uint8_t *)&(b2->tag))) goto FAIL;
    vpc = vpc + NMBL + SMBL;
    et[nel] = b2;
    nel++;
    goto NEXTOP;
    // LSCO(N);
LSCO:
    SHB1 if (b1->tag != TAGO) goto FAIL;
    if (b1->info.infoc != *(vpc + NMBL))
        goto FAIL;
    et[nel] = b1;
    nel++;
    vpc = vpc + NMBL + NMBL;
    goto NEXTOP;
    // RSCO(N);
RSCO:
    SHB2 if (b2->tag != TAGO) goto FAIL;
    if (b2->info.infoc != *(vpc + NMBL))
        goto FAIL;
    et[nel] = b2;
    nel++;
    vpc = vpc + NMBL + NMBL;
    goto NEXTOP;
    // LSD(N);
LSD:
    SHB1
        n = (unsigned int)*(vpc + NMBL);
    if (b1->tag != et[n]->tag)
        goto FAIL;
    if (b1->info.codef != et[n]->info.codef)
        goto FAIL;
    et[nel] = b1;
    nel++;
    vpc = vpc + NMBL + NMBL;
    goto NEXTOP;
    // RSD(N);
RSD:
    SHB2
        n = (unsigned int)*(vpc + NMBL);
    if (b2->tag != et[n]->tag)
        goto FAIL;
    if (b2->info.codef != et[n]->info.codef)
        goto FAIL;
    et[nel] = b2;
    nel++;
    vpc = vpc + NMBL + NMBL;
    goto NEXTOP;
    // LTXT(N,S1,...,SN);
LTXT:
    n = (unsigned int)*(vpc + NMBL);
    vpc = vpc + NMBL + NMBL;
    goto LTXT1;
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
    // RTXT(N,S1,...,SN);
RTXT:
    n = (unsigned int)*(vpc + NMBL);
    vpc = vpc + NMBL + NMBL;
    goto RTXT1;
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
    // LB;
LB:
    SHB1 if ((b1->tag & 0001) == 0) goto FAIL;
    b2 = b1->info.codep;
    et[nel] = b1;
    et[nel + 1] = b2;
    nel = nel + 2;
    goto ADVANCE;
    // LBY;
LBY:
    SHB1 if ((b1->tag & 0001) == 0) goto FAIL;
    et[nel] = b1;
    b1 = b1->info.codep;
    et[nel + 1] = b1;
    nel = nel + 2;
    goto ADVANCE;
    // RB;
RB:
    SHB2 if ((b2->tag & 0001) == 0) goto FAIL;
    b1 = b2->info.codep;
    et[nel] = b1;
    et[nel + 1] = b2;
    nel = nel + 2;
    goto ADVANCE;
    // RBY;
RBY:
    SHB2 if ((b2->tag & 0001) == 0) goto FAIL;
    et[nel + 1] = b2;
    b2 = b2->info.codep;
    et[nel] = b2;
    nel = nel + 2;
    goto ADVANCE;
    // LS;
LS:
    SHB1 if ((b1->tag & 0001) != 0) goto FAIL;
    et[nel] = b1;
    nel++;
    goto ADVANCE;
    // RS;
RS:
    SHB2 if ((b2->tag & 0001) != 0) goto FAIL;
    et[nel] = b2;
    nel++;
    goto ADVANCE;
    // LW;
LW:
    SHB1
        et[nel] = b1;
    if ((b1->tag & 0001) != 0)
        b1 = b1->info.codep;
    et[nel + 1] = b1;
    nel = nel + 2;
    goto ADVANCE;
    // RW;
RW:
    SHB2
        et[nel + 1] = b2;
    if ((b2->tag & 0001) != 0)
        b2 = b2->info.codep;
    et[nel] = b2;
    nel = nel + 2;
    goto ADVANCE;
    // LBNIL
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
    // RBNIL;
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
    // LBCE;
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
    // RBCE;
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
    // NIL;
NIL:
    if (b1->next != b2)
        goto FAIL;
    goto ADVANCE;
    // CE;
CE:
    et[nel] = b1->next;
    et[nel + 1] = b2->prev;
    nel = nel + 2;
    goto ADVANCE;
    // LED(N);
LED:
    n = (unsigned int)*(vpc + NMBL);
    et[nel] = b1->next;
    b0 = et[n - 1]->prev;
    goto LED1;
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
    // RED(N);
RED:
    n = (unsigned int)*(vpc + NMBL);
    et[nel + 1] = b2->prev;
    b0 = et[n]->next;
    goto RED1;
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
    // NNIL;
NNIL:
    if (et[nel - 1]->next == et[nel - 2])
        goto FAIL;
    goto ADVANCE;
    // PLE;
PLE:
    vpc = vpc + NMBL;
    putjs(jsp, &b1, &b2, &nel, &vpc);
    jsp++;
    et[nel] = b1->next;
    et[nel + 1] = b1;
    nel = nel + 2;
    goto ADVANCE;
    // PLV;
PLV:
    vpc = vpc + NMBL;
    putjs(jsp, &b1, &b2, &nel, &vpc);
    et[nel] = b1->next;
    et[nel + 1] = b1;
    goto NEXTOP;
    // LE;
LE:
    b1 = et[nel + 1];
    SHB1 if ((b1->tag & 0001) != 0) b1 = b1->info.codep;
    jsp++;
    et[nel + 1] = b1;
    nel = nel + 2;
    goto ADVANCE;
    // PRE;
PRE:
    vpc = vpc + NMBL;
    putjs(jsp, &b1, &b2, &nel, &vpc);
    jsp++;
    et[nel] = b2;
    et[nel + 1] = b2->prev;
    nel = nel + 2;
    goto ADVANCE;
    // PRV;
PRV:
    vpc = vpc + NMBL;
    putjs(jsp, &b1, &b2, &nel, &vpc);
    et[nel] = b2;
    et[nel + 1] = b2->prev;
    goto NEXTOP;
    // RE;
RE:
    b2 = et[nel];
    SHB2 if ((b2->tag & 0001) != 0) b2 = b2->info.codep;
    jsp++;
    et[nel] = b2;
    nel = nel + 2;
    goto ADVANCE;
    // PLESC;
PLESC:
    vpc = vpc + NMBL;
    putjs(jsp, &b1, &b2, &nel, &vpc);
    et[nel] = b1->next;
    et[nel + 2] = b1;
    goto NEXTOP;
    // PLVSC;
PLVSC:
    vpc = vpc + NMBL;
    putjs(jsp, &b1, &b2, &nel, &vpc);
    et[nel] = b1->next;
    SHB1 if ((b1->tag & 0001) != 0) b1 = b1->info.codep;
    et[nel + 2] = b1;
    goto NEXTOP;
    // LESC(S);
LESC:
    vpc = vpc + NMBL + SMBL;
    vpca = vpc - SMBL;
    goto LESC0;
LESC0:
    b1 = et[nel + 2];
    goto LESC1;
LESC1:
    SHB1 if ((b1->tag & 0001) != 0)
    { // if(BRA(B1))
        b1 = b1->info.codep;
        goto LESC1;
    }
    if (!cmpr(SMBL, vpca, (uint8_t *)&(b1->tag)))
        goto LESC1;
    jsp++;
    et[nel + 1] = b1->prev;
    et[nel + 2] = b1;
    nel = nel + 3;
    goto NEXTOP;
    // PRESC;
PRESC:
    vpc = vpc + NMBL;
    putjs(jsp, &b1, &b2, &nel, &vpc);
    et[nel + 1] = b2->prev;
    et[nel + 2] = b2;
    goto NEXTOP;
    // PRVSC;
PRVSC:
    vpc = vpc + NMBL;
    putjs(jsp, &b1, &b2, &nel, &vpc);
    et[nel + 1] = b2->prev;
    SHB2 if ((b2->tag & 0001) != 0) b2 = b2->info.codep;
    et[nel + 2] = b2;
    goto NEXTOP;
    // RESC(S);
RESC:
    vpc = vpc + NMBL + SMBL;
    vpca = vpc - SMBL;
    goto RESC0;
RESC0:
    b2 = et[nel + 2];
    goto RESC1;
RESC1:
    SHB2 if ((b2->tag & 0001) != 0)
    { // if(BRA(B2))
        b2 = b2->info.codep;
        goto RESC1;
    }
    if (!cmpr(SMBL, vpca, (uint8_t *)&(b2->tag)))
        goto RESC1;
    jsp++;
    et[nel + 2] = b2;
    et[nel] = b2->next;
    nel = nel + 3;
    goto NEXTOP;
    // LESD(N);
LESD:
    n = (unsigned int)*(vpc + NMBL);
    vpca = (uint8_t *)&(et[n]->tag);
    vpc = vpc + NMBL * 2;
    goto LESC0;
    // RESD(N);
RESD:
    n = (unsigned int)*(vpc + NMBL);
    vpca = (uint8_t *)&(et[n]->tag);
    vpc = vpc + NMBL * 2;
    goto RESC0;
    // PLEB;
PLEB:
    vpc = vpc + NMBL;
    putjs(jsp, &b1, &b2, &nel, &vpc);
    et[nel] = b1->next;
    et[nel + 3] = b1;
    goto NEXTOP;
    // PLVB;
PLVB:
    vpc = vpc + NMBL;
    putjs(jsp, &b1, &b2, &nel, &vpc);
    et[nel] = b1->next;
    SHB1 if ((b1->tag & 0001) != 0) b1 = b1->info.codep;
    et[nel + 3] = b1;
    goto NEXTOP;
    // LEB;
LEB:
    b1 = et[nel + 3];
    goto LEB1;
LEB1:
    SHB1 if ((b1->tag & 0001) == 0) goto LEB1;
    jsp++;
    et[nel + 1] = b1->prev;
    et[nel + 2] = b1;
    b2 = b1->info.codep;
    et[nel + 3] = b2;
    nel = nel + 4;
    goto ADVANCE;
    // PREB;
PREB:
    vpc = vpc + NMBL;
    putjs(jsp, &b1, &b2, &nel, &vpc);
    et[nel + 1] = b2->prev;
    et[nel + 2] = b2;
    goto NEXTOP;
    // PRVB;
PRVB:
    vpc = vpc + NMBL;
    putjs(jsp, &b1, &b2, &nel, &vpc);
    et[nel + 1] = b2->prev;
    SHB2 if ((b2->tag & 0001) != 0) b2 = b2->info.codep;
    et[nel + 2] = b2;
    goto NEXTOP;
    //  REB;
REB:
    b2 = et[nel + 2];
    goto REB1;
REB1:
    SHB2 if ((b2->tag & 0001) == 0) goto REB1;
    jsp++;
    et[nel] = b2->next;
    et[nel + 3] = b2;
    b1 = b2->info.codep;
    et[nel + 2] = b1;
    nel = nel + 4;
    goto ADVANCE;
    // EOE(N);
EOE:
    n = (unsigned int)*(vpc + NMBL);
    jsp = jsp - n;
    vpc = vpc + 2 * NMBL;
    goto NEXTOP;
    // EOEI;
EOEI:
    jsp--;
    goto ADVANCE;
    // LSRCH(S);
LSRCH:
    et[nel] = b1->next;
    goto LSRCH1;
LSRCH1:
    SHB1 if ((b1->tag & 0001) != 0)
    {
        b1 = b1->info.codep;
        goto LSRCH1;
    };
    if (!cmpr(SMBL, vpc + NMBL, (uint8_t *)&(b1->tag)))
        goto LSRCH1;
    et[nel + 1] = b1->prev;
    et[nel + 2] = b1;
    nel = nel + 3;
    vpc = vpc + NMBL + SMBL;
    goto NEXTOP;
    // RSRCH(S);
RSRCH:
    et[nel + 1] = b2->prev;
    goto RSRCH;
RSRCH1:
    SHB2 if ((b2->tag & 0001) != 0)
    {
        b2 = b2->info.codep;
        goto RSRCH1;
    };
    if (!cmpr(SMBL, vpc + NMBL, (uint8_t *)&(b2->tag)))
        goto RSRCH1;
    et[nel] = b2->next;
    et[nel + 2] = b2;
    nel = nel + 3;
    vpc = vpc + NMBL + SMBL;
    goto NEXTOP;
    // WSPC(L);
WSPC:
    if (!spc((T_SPCS *)(jsp + 1), vpc, et[nel - 1]))
        goto FAIL;
    vpc = vpc + NMBL + LBLL;
    goto NEXTOP;
    // ESPC(L);
ESPC:
    b0 = et[nel - 2]->prev;
    while (b0 != et[nel - 1])
    {
        b0 = b0->next;
        if ((b0->tag & 0001) != 0)
            b0 = b0->info.codep;
        if (!spc((T_SPCS *)(jsp + 1), vpc, b0))
            goto FAIL;
    };
    vpc = vpc + NMBL + LBLL;
    goto NEXTOP;
    // PLESPC;
PLESPC:
    vpc = vpc + NMBL;
    putjs(jsp, &b1, &b2, &nel, &vpc);
    jsp++;
    et[nel] = b1->next;
    et[nel + 1] = b1;
    nel = nel + 2;
    vpc = vpc + NMBL + LBLL;
    goto NEXTOP;
    // LESPC(L);
LESPC:
    b1 = et[nel + 1];
    SHB1 if ((b1->tag & 0001) != 0) b1 = b1->info.codep;
    if (!spc((T_SPCS *)(jsp + 1), vpc, b1))
        goto FAIL;
    jsp++;
    et[nel + 1] = b1;
    nel = nel + 2;
    vpc = vpc + NMBL + LBLL;
    goto NEXTOP;
    // PRESPC;
PRESPC:
    vpc = vpc + NMBL;
    putjs(jsp, &b1, &b2, &nel, &vpc);
    jsp++;
    et[nel + 1] = b2->prev;
    et[nel] = b2;
    nel = nel + 2;
    vpc = vpc + NMBL + LBLL;
    goto NEXTOP;
    // RESPC(L);
RESPC:
    b2 = et[nel];
    SHB2 if ((b2->tag & 0001) != 0) b2 = b2->info.codep;
    if (!spc((T_SPCS *)(jsp + 1), vpc, b2))
        goto FAIL;
    jsp++;
    et[nel] = b2;
    nel = nel + 2;
    vpc = vpc + NMBL + LBLL;
    goto NEXTOP;
    // LMAX(L);
LMAX:
    et[nel] = b1->next;
    while ((b1 = b1->next) != b2)
    {
        if (!spc((T_SPCS *)(jsp + 1), vpc, b1))
            break;
        if ((b1->tag & 0001) != 0)
            b1 = b1->info.codep;
    };
    b1 = b1->prev;
    et[nel + 1] = b1;
    nel = nel + 2;
    vpc = vpc + NMBL + LBLL;
    goto NEXTOP;
    // RMAX(L);
RMAX:
    et[nel + 1] = b2->prev;
    while ((b2 = b2->prev) != b1)
    {
        if (!spc((T_SPCS *)(jsp + 1), vpc, b2))
            break;
        if ((b2->tag & 0001) != 0)
            b2 = b2->info.codep;
    };
    b2 = b2->next;
    et[nel] = b2;
    nel = nel + 2;
    vpc = vpc + NMBL + LBLL;
    goto NEXTOP;
    // EOR;
EOR:
    f = flhead;
    lastk = &quasik;
    lastb = NULL;
    tsp = (T_TS *)js;
    goto ADVANCE;
    // NS(S);
NS:
    SHF
        move(SMBL, vpc + NMBL, (uint8_t *)&(f->tag));
    vpc = vpc + NMBL + SMBL;
    goto NEXTOP;
    // NSO(N);
NSO:
    SHF
        f->tag = TAGO;
    f->info.codep = NULL;
    f->info.infoc = *(vpc + 1);
    vpc = vpc + NMBL + NMBL;
    goto NEXTOP;
    // TEXT(N,S1,S2,...,SN);
TEXT:
    n = (unsigned int)*(vpc + NMBL);
    // printf("\n TEXT uc0=%x uii=%x  %d",u.c[0],u.ii,u.ii);
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
    // BL;
BL:
    SHF
        f->info.codep = lastb;
    lastb = f;
    goto ADVANCE;
    // BR;
BR:
    SHF
        f->info.codep = lastb;
    f1 = lastb->info.codep;
    lastb->tag = TAGLB;
    lastb->info.codep = f;
    f->tag = TAGRB;
    lastb = f1;
    goto ADVANCE;
    // BLR;
BLR:
    SHF
        f1 = f;
    SHF
        f1->info.codep = f;
    f->info.codep = f1;
    f1->tag = TAGLB;
    f->tag = TAGRB;
    goto ADVANCE;
    // BRACT;
BRACT:
    SHF
        f->info.codep = lastb;
    f->tag = TAGD;
    lastk->info.codep = f;
    lastk->tag = TAGK;
    lastk = lastb;
    lastb = lastb->info.codep;
    goto ADVANCE;
    // ACT(N);
ACT:
    n = (unsigned int)*(vpc + NMBL);
    lastk->info.codep = et[n];
    lastk->tag = TAGK;
    lastk = et[n]->info.codep;
    et[n]->tag = TAGD;
    vpc = vpc + NMBL + NMBL;
    goto NEXTOP;
    // MULS;
MULS:
    n = (unsigned int)*(vpc + NMBL);
    SHF
        move(SMBL, (uint8_t *)&(et[n]->tag), (uint8_t *)&(f->tag));
    vpc = vpc + NMBL + NMBL;
    goto NEXTOP;
    // MULE(N);
MULE:
    //                    if(ferr == 1){
    //                        lastb = lastb1;
    //                        ferr = 0;}
    //                    else
    //                        lastb1 = lastb;
    n = (unsigned int)*(vpc + NMBL);
    vpc = vpc + NMBL + NMBL;
    f0 = et[n - 1]->prev;
    while (f0 != et[n])
    {
        f0 = f0->next;
        SHF //              f = f->next;       instead of SHF
            //             if (f == flhead){
            //               ferr = 1;
            //               goto LACK;}
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
        else move(SMBL, (uint8_t *)&(f0->tag), (uint8_t *)&(f->tag));
    };
    goto NEXTOP;
    // TPL(N,M);
    // TPLM(N,M);
TPL:
TPLM:
    n = (unsigned int)*(vpc + NMBL);
    m = (unsigned int)*(vpc + NMBL + NMBL);
    vpc = vpc + NMBL * 3;
    if (et[m]->next == et[n])
        goto NEXTOP;
    putts(tsp, &f, &et[n], &et[m]);
    tsp++;
    goto NEXTOP;
    // TPLE(N); (= TPL(N-1,N);)
    // TPLV(N);
TPLE:
TPLV:
    n = (unsigned int)*(vpc + NMBL);
    vpc = vpc + NMBL + NMBL;
    if (et[n]->next == et[n - 1])
        goto NEXTOP;
    putts(tsp, &f, &et[n - 1], &et[n]);
    tsp++;
    goto NEXTOP;
    // TPLS(N); (= TPLM(N,N);)
TPLS:
    n = (unsigned int)*(vpc + NMBL);
    vpc = vpc + NMBL + NMBL;
    putts(tsp, &f, &et[n], &et[n]);
    tsp++;
    goto NEXTOP;
    // EOS;
EOS:
    lastk->info.codep = et[1]->info.codep;
    lastk->tag = TAGK;
    // item adress followed by result
    T_LINKCB *nextr = f->next;
    // execute planned transplantation
    // EOS1:
    while (tsp != (T_TS *)js)
    {
        tsp = tsp - 1;
        getts(tsp, &f, &f0, &f1);
        link(f0->prev, f1->next);
        link(f1, f->next);
        link(f, f0);
    }
    // include replace result
    // INSRES:
    if (flhead->next == nextr)
        link(et[1]->prev, et[2]->next);
    else
    {
        link(nextr->prev, et[2]->next);
        link(et[1]->prev, flhead->next);
    };
    //  delete k and  .
    // DELKD
    link(et[2], nextr);
    link(flhead, et[1]);
    goto ADVSTEP;
    // SWAP;
    //  static box head is after operator code
SWAP:
    vpc = vpc + NMBL;
    f = (T_LINKCB *)vpc;
    if (f->prev != NULL)
        goto SWAPREF;
    link(f, f);
    f->info.codep = refal.svar;
    f->tag = 0;
    refal.svar = f;
    goto SWAPREF;
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
    // BLF(L);
BLF:
    SHF
        f->info.codep = lastb;
    lastb = f;
    SHF
        move(LBLL, vpc + NMBL, (uint8_t *)&(f->info.codef));
    f->tag = TAGF;
    vpc = vpc + NMBL + LBLL;
    goto NEXTOP;
    // EOSSN (NN);
EOSSN:
    move(NMBL + NMBL, vpc + NMBL, (uint8_t *)&(refal.stmnmb));
    goto EOS;
    // SETNOS(L);
SETNOS:
    move(LBLL, vpc + NMBL, (uint8_t *)&(inch.inr));
    refal.nostm = (int)*(inch.inr);
    vpc = vpc + NMBL + LBLL;
    goto NEXTOP;
    // C-refal-function execution
CFUNC:
    move(LBLL, vpc + NMBL + Z_0, (uint8_t *)&fptr);
    refal.upshot = 1;
    refal.prevr = b0->prev;
    refal.nextr = b0;
    refal.preva = b1;
    refal.nexta = b2;
    //        call  C - function
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
    //        return from C - function
    //          step is done
CFDONE:
    quasik.info.codep = refal.nextr->info.codep;
    link(refal.nextr->prev, refal.nexta->next);
    link(refal.nexta, flhead->next);
    link(flhead, refal.nextr);
    goto ADVSTEP;
    //        return from C - function
    //     free memory exhausted
CFLACK:
    if (refal.prevr->next != refal.nextr)
    {
        link(refal.nextr->prev, flhead->next);
        link(flhead, refal.prevr->next);
        link(refal.prevr, refal.nextr);
    }
    ast->state = 3;
    goto EXIT;
}
//------------ end of file RFRUN1.C ----------
