// Copyright 2024 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2024-09-27
// https://github.com/Aleksandr3Bocharov/RefalAB

//----------- file RFRUN1.C -------------------
//      Refal-interpretator (part 1)
//---------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include "refal.def"
#include "rfrun1.h"
#include "rfintf.h"
#include "rfrun2.h"

// for unlooping:

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
    //    EOSSN,
    //    SETNOS,
    CFUNC,
    CFDONE,
    CFLACK
} T_I_STATES;

typedef struct sav_
{ // save area for var-part of REFAL-block
    uint32_t upshot_;
    T_LINKCB *preva_;
    T_LINKCB *nexta_;
    T_LINKCB *prevr_;
    T_LINKCB *nextr_;
    T_ST *currst_;
} T_SAV;

/*static union
{
    char c[2];
    uint16_t ii;
} u; */

static union
{ // structure for pointer and integer aligning
    uint8_t *ptr;
    //    uint16_t *inr;
    //    char chr[2];
} inch;

static T_LINKCB *et[256]; // element table
static size_t nel;        // adress of first free string in element table

static T_WJS js[64]; // jump stack and planning translation stack
static T_WJS *jsp;   // jump stack pointer

static T_TS *tsp; // translation stack pointer

static bool tmmod;     // timer state
static time_t tmstart; // time at the start
static time_t tmstop;  // time at the end

// definition of work variables and pointers
static uint8_t opc;     // current statement code
static uint8_t *vpc;    // virtual program counter
static T_LINKCB *lastk; // last acted sign-k adress
static T_LINKCB *lastb; // last generated left bracket
static T_LINKCB *b0, *b1, *b2;
static T_LINKCB *f0, *f1, *f;
static const uint8_t *vpca; // additional vpc
static size_t i, n, m;

static void (*fptr)(void);

void rfrun(T_ST *ast) // adress of current state table
{
    // dynamic area DSA
    T_SAV *savecr = malloc(sizeof(T_SAV));
    // u.ii = 0;
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
    refal.tmmode = false;
    refal.currst = ast;
    ast->state = 4;
    T_LINKCB quasik; // quasi-sign k
    quasik.info.codep = ast->dot;
    // adress of free memory list head
    T_LINKCB *flhead = refal.flhead;
    if (tmmod)
        tmstart = time(NULL);
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
            vpc = b1->info.codef;
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
            ast->step++;
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
            if (tmmod)
                tmstop = time(0);
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
            if (tmmod)
                //{
                // printf("\nn=%ld k=%ld",tmstart,tmstop);
                refal.tmintv = 0;
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
            // case 0120:
            //     i_state = EOSSN;
            //     break;
            // case 0121:
            //     i_state = SETNOS;
            //     break;
            case 0122:
                i_state = CFUNC;
            };
            break;
            // select next statement assembly language
        case ADVANCE:
            vpc = vpc + NMBL;
            i_state = NEXTOP;
            break;
            // SJUMP(L);
        case SJUMP:
            memcpy(&inch.ptr, vpc + NMBL, LBLL);
            putjs(jsp, &b1, &b2, &nel, &inch.ptr);
            jsp++;
            vpc = vpc + NMBL + LBLL;
            i_state = NEXTOP;
            break;
            // FAIL;
        case FAIL:
            if (jsp == js)
            {
                i_state = RCGIMP;
                break;
            }
            jsp--;
            getjs(jsp, &b1, &b2, &nel, &vpc);
            i_state = NEXTOP;
            break;
            // SB(N,M);
        case SB:
            n = *(vpc + NMBL);
            m = *(vpc + NMBL + NMBL);
            b1 = et[n];
            b2 = et[m];
            vpc = vpc + 3 * NMBL;
            i_state = NEXTOP;
            break;
            // LSC(S);
        case LSC:
            SHB1 if (memcmp(vpc + NMBL, &b1->tag, SMBL) != 0)
            {
                i_state = FAIL;
                break;
            }
            vpc = vpc + NMBL + SMBL;
            et[nel] = b1;
            nel++;
            i_state = NEXTOP;
            break;
            // RSC(S);
        case RSC:
            SHB2 if (memcmp(vpc + NMBL, &b2->tag, SMBL) != 0)
            {
                i_state = FAIL;
                break;
            }
            vpc = vpc + NMBL + SMBL;
            et[nel] = b2;
            nel++;
            i_state = NEXTOP;
            break;
            // LSCO(N);
        case LSCO:
            SHB1 if (b1->tag != TAGO)
            {
                i_state = FAIL;
                break;
            }
            if (b1->info.infoc != (char)*(vpc + NMBL))
            {
                i_state = FAIL;
                break;
            }
            et[nel] = b1;
            nel++;
            vpc = vpc + NMBL + NMBL;
            i_state = NEXTOP;
            break;
            // RSCO(N);
        case RSCO:
            SHB2 if (b2->tag != TAGO)
            {
                i_state = FAIL;
                break;
            }
            if (b2->info.infoc != (char)*(vpc + NMBL))
            {
                i_state = FAIL;
                break;
            }
            et[nel] = b2;
            nel++;
            vpc = vpc + NMBL + NMBL;
            i_state = NEXTOP;
            break;
            // LSD(N);
        case LSD:
            SHB1
                n = *(vpc + NMBL);
            if (b1->tag != et[n]->tag)
            {
                i_state = FAIL;
                break;
            }
            if (b1->info.codef != et[n]->info.codef)
            {
                i_state = FAIL;
                break;
            }
            et[nel] = b1;
            nel++;
            vpc = vpc + NMBL + NMBL;
            i_state = NEXTOP;
            break;
            // RSD(N);
        case RSD:
            SHB2
                n = *(vpc + NMBL);
            if (b2->tag != et[n]->tag)
            {
                i_state = FAIL;
                break;
            }
            if (b2->info.codef != et[n]->info.codef)
            {
                i_state = FAIL;
                break;
            }
            et[nel] = b2;
            nel++;
            vpc = vpc + NMBL + NMBL;
            i_state = NEXTOP;
            break;
            // LTXT(N,S1,...,SN);
        case LTXT:
            n = *(vpc + NMBL);
            vpc = vpc + NMBL + NMBL;
            i_state = LTXT1;
            break;
        case LTXT1:
            SHB1 if (b1->tag != TAGO)
            {
                i_state = FAIL;
                break;
            }
            if (b1->info.infoc != (char)*vpc)
            {
                i_state = FAIL;
                break;
            }
            et[nel] = b1;
            nel++;
            vpc = vpc + NMBL;
            n--;
            if (n != 0)
                break;
            i_state = NEXTOP;
            break;
            // RTXT(N,S1,...,SN);
        case RTXT:
            n = *(vpc + NMBL);
            vpc = vpc + NMBL + NMBL;
            i_state = RTXT1;
            break;
        case RTXT1:
            SHB2 if (b2->tag != TAGO)
            {
                i_state = FAIL;
                break;
            }
            if (b2->info.infoc != (char)*vpc)
            {
                i_state = FAIL;
                break;
            }
            et[nel] = b2;
            nel++;
            vpc = vpc + NMBL;
            n--;
            if (n != 0)
                break;
            i_state = NEXTOP;
            break;
            // LB;
        case LB:
            SHB1 if ((b1->tag & 0001) == TAGO)
            {
                i_state = FAIL;
                break;
            }
            b2 = b1->info.codep;
            et[nel] = b1;
            et[nel + 1] = b2;
            nel = nel + 2;
            i_state = ADVANCE;
            break;
            // LBY;
        case LBY:
            SHB1 if ((b1->tag & 0001) == TAGO)
            {
                i_state = FAIL;
                break;
            }
            et[nel] = b1;
            b1 = b1->info.codep;
            et[nel + 1] = b1;
            nel = nel + 2;
            i_state = ADVANCE;
            break;
            // RB;
        case RB:
            SHB2 if ((b2->tag & 0001) == TAGO)
            {
                i_state = FAIL;
                break;
            }
            b1 = b2->info.codep;
            et[nel] = b1;
            et[nel + 1] = b2;
            nel = nel + 2;
            i_state = ADVANCE;
            break;
            // RBY;
        case RBY:
            SHB2 if ((b2->tag & 0001) == TAGO)
            {
                i_state = FAIL;
                break;
            }
            et[nel + 1] = b2;
            b2 = b2->info.codep;
            et[nel] = b2;
            nel = nel + 2;
            i_state = ADVANCE;
            break;
            // LS;
        case LS:
            SHB1 if ((b1->tag & 0001) != TAGO)
            {
                i_state = FAIL;
                break;
            }
            et[nel] = b1;
            nel++;
            i_state = ADVANCE;
            break;
            // RS;
        case RS:
            SHB2 if ((b2->tag & 0001) != TAGO)
            {
                i_state = FAIL;
                break;
            }
            et[nel] = b2;
            nel++;
            i_state = ADVANCE;
            break;
            // LW;
        case LW:
            SHB1
                et[nel] = b1;
            if ((b1->tag & 0001) != TAGO)
                b1 = b1->info.codep;
            et[nel + 1] = b1;
            nel = nel + 2;
            i_state = ADVANCE;
            break;
            // RW;
        case RW:
            SHB2
                et[nel + 1] = b2;
            if ((b2->tag & 0001) != TAGO)
                b2 = b2->info.codep;
            et[nel] = b2;
            nel = nel + 2;
            i_state = ADVANCE;
            break;
            // LBNIL
        case LBNIL:
            SHB1 if ((b1->tag & 0001) == TAGO)
            {
                i_state = FAIL;
                break;
            }
            b0 = b1;
            b1 = b1->info.codep;
            if (b0->next != b1)
            {
                i_state = FAIL;
                break;
            }
            et[nel] = b0;
            et[nel + 1] = b1;
            nel = nel + 2;
            i_state = ADVANCE;
            break;
            // RBNIL;
        case RBNIL:
            SHB2 if ((b2->tag & 0001) == TAGO)
            {
                i_state = FAIL;
                break;
            }
            b0 = b2;
            b2 = b2->info.codep;
            if (b2->next != b0)
            {
                i_state = FAIL;
                break;
            }
            et[nel] = b2;
            et[nel + 1] = b0;
            nel = nel + 2;
            i_state = ADVANCE;
            break;
            // LBCE;
        case LBCE:
            SHB1 if ((b1->tag & 0001) == TAGO)
            {
                i_state = FAIL;
                break;
            }
            b0 = b1;
            b1 = b1->info.codep;
            et[nel] = b0;
            et[nel + 1] = b1;
            et[nel + 2] = b0->next;
            et[nel + 3] = b1->prev;
            nel = nel + 4;
            i_state = ADVANCE;
            break;
            // RBCE;
        case RBCE:
            SHB2 if ((b2->tag & 0001) == TAGO)
            {
                i_state = FAIL;
                break;
            }
            b0 = b2;
            b2 = b2->info.codep;
            et[nel] = b2;
            et[nel + 1] = b0;
            et[nel + 2] = b2->next;
            et[nel + 3] = b0->prev;
            nel = nel + 4;
            i_state = ADVANCE;
            break;
            // NIL;
        case NIL:
            if (b1->next != b2)
            {
                i_state = FAIL;
                break;
            }
            i_state = ADVANCE;
            break;
            // CE;
        case CE:
            et[nel] = b1->next;
            et[nel + 1] = b2->prev;
            nel = nel + 2;
            i_state = ADVANCE;
            break;
            // LED(N);
        case LED:
            n = *(vpc + NMBL);
            et[nel] = b1->next;
            b0 = et[n - 1]->prev;
            i_state = LED1;
            break;
        case LED1:
            if (b0 == et[n])
            {
                i_state = LED2;
                break;
            }
            b0 = b0->next;
            SHB1 if (b1->tag != b0->tag)
            {
                i_state = FAIL;
                break;
            }
            if (b1->info.codef == b0->info.codef)
                break;
            if ((b1->tag & 0001) != TAGO)
                break;
            i_state = FAIL;
            break;
        case LED2:
            et[nel + 1] = b1;
            nel = nel + 2;
            vpc = vpc + NMBL * 2;
            i_state = NEXTOP;
            break;
            // RED(N);
        case RED:
            n = *(vpc + NMBL);
            et[nel + 1] = b2->prev;
            b0 = et[n]->next;
            i_state = LED1;
            break;
        case RED1:
            if (b0 == et[n - 1])
            {
                i_state = RED2;
                break;
            }
            b0 = b0->prev;
            SHB2 if (b2->tag != b0->tag)
            {
                i_state = FAIL;
                break;
            }
            if (b2->info.codef == b0->info.codef)
                break;
            if ((b2->tag & 0001) != TAGO)
                break;
            i_state = FAIL;
            break;
        case RED2:
            et[nel] = b2;
            nel = nel + 2;
            vpc = vpc + NMBL * 2;
            i_state = NEXTOP;
            break;
            // NNIL;
        case NNIL:
            if (et[nel - 1]->next == et[nel - 2])
            {
                i_state = FAIL;
                break;
            }
            i_state = ADVANCE;
            break;
            // PLE;
        case PLE:
            vpc = vpc + NMBL;
            putjs(jsp, &b1, &b2, &nel, &vpc);
            jsp++;
            et[nel] = b1->next;
            et[nel + 1] = b1;
            nel = nel + 2;
            i_state = ADVANCE;
            break;
            // PLV;
        case PLV:
            vpc = vpc + NMBL;
            putjs(jsp, &b1, &b2, &nel, &vpc);
            et[nel] = b1->next;
            et[nel + 1] = b1;
            i_state = NEXTOP;
            break;
            // LE;
        case LE:
            b1 = et[nel + 1];
            SHB1 if ((b1->tag & 0001) != TAGO) b1 = b1->info.codep;
            jsp++;
            et[nel + 1] = b1;
            nel = nel + 2;
            i_state = ADVANCE;
            break;
            // PRE;
        case PRE:
            vpc = vpc + NMBL;
            putjs(jsp, &b1, &b2, &nel, &vpc);
            jsp++;
            et[nel] = b2;
            et[nel + 1] = b2->prev;
            nel = nel + 2;
            i_state = ADVANCE;
            break;
            // PRV;
        case PRV:
            vpc = vpc + NMBL;
            putjs(jsp, &b1, &b2, &nel, &vpc);
            et[nel] = b2;
            et[nel + 1] = b2->prev;
            i_state = NEXTOP;
            break;
            // RE;
        case RE:
            b2 = et[nel];
            SHB2 if ((b2->tag & 0001) != TAGO) b2 = b2->info.codep;
            jsp++;
            et[nel] = b2;
            nel = nel + 2;
            i_state = ADVANCE;
            break;
            // PLESC;
        case PLESC:
            vpc = vpc + NMBL;
            putjs(jsp, &b1, &b2, &nel, &vpc);
            et[nel] = b1->next;
            et[nel + 2] = b1;
            i_state = NEXTOP;
            break;
            // PLVSC;
        case PLVSC:
            vpc = vpc + NMBL;
            putjs(jsp, &b1, &b2, &nel, &vpc);
            et[nel] = b1->next;
            SHB1 if ((b1->tag & 0001) != TAGO) b1 = b1->info.codep;
            et[nel + 2] = b1;
            i_state = NEXTOP;
            break;
            // LESC(S);
        case LESC:
            vpc = vpc + NMBL + SMBL;
            vpca = vpc - SMBL;
            i_state = LESC0;
            break;
        case LESC0:
            b1 = et[nel + 2];
            i_state = LESC1;
            break;
        case LESC1:
            SHB1 if ((b1->tag & 0001) != TAGO)
            { // if(BRA(B1))
                b1 = b1->info.codep;
                break;
            }
            if (memcmp(vpca, &b1->tag, SMBL) != 0)
                break;
            jsp++;
            et[nel + 1] = b1->prev;
            et[nel + 2] = b1;
            nel = nel + 3;
            i_state = NEXTOP;
            break;
            // PRESC;
        case PRESC:
            vpc = vpc + NMBL;
            putjs(jsp, &b1, &b2, &nel, &vpc);
            et[nel + 1] = b2->prev;
            et[nel + 2] = b2;
            i_state = NEXTOP;
            break;
            // PRVSC;
        case PRVSC:
            vpc = vpc + NMBL;
            putjs(jsp, &b1, &b2, &nel, &vpc);
            et[nel + 1] = b2->prev;
            SHB2 if ((b2->tag & 0001) != TAGO) b2 = b2->info.codep;
            et[nel + 2] = b2;
            i_state = NEXTOP;
            break;
            // RESC(S);
        case RESC:
            vpc = vpc + NMBL + SMBL;
            vpca = vpc - SMBL;
            i_state = RESC0;
            break;
        case RESC0:
            b2 = et[nel + 2];
            i_state = RESC1;
            break;
        case RESC1:
            SHB2 if ((b2->tag & 0001) != TAGO)
            { // if(BRA(B2))
                b2 = b2->info.codep;
                break;
            }
            if (memcmp(vpca, &b2->tag, SMBL) != 0)
                break;
            jsp++;
            et[nel + 2] = b2;
            et[nel] = b2->next;
            nel = nel + 3;
            i_state = NEXTOP;
            break;
            // LESD(N);
        case LESD:
            n = *(vpc + NMBL);
            vpca = (uint8_t *)&et[n]->tag;
            vpc = vpc + NMBL * 2;
            i_state = LESC0;
            break;
            // RESD(N);
        case RESD:
            n = *(vpc + NMBL);
            vpca = (uint8_t *)&et[n]->tag;
            vpc = vpc + NMBL * 2;
            i_state = RESC0;
            break;
            // PLEB;
        case PLEB:
            vpc = vpc + NMBL;
            putjs(jsp, &b1, &b2, &nel, &vpc);
            et[nel] = b1->next;
            et[nel + 3] = b1;
            i_state = NEXTOP;
            break;
            // PLVB;
        case PLVB:
            vpc = vpc + NMBL;
            putjs(jsp, &b1, &b2, &nel, &vpc);
            et[nel] = b1->next;
            SHB1 if ((b1->tag & 0001) != TAGO) b1 = b1->info.codep;
            et[nel + 3] = b1;
            i_state = NEXTOP;
            break;
            // LEB;
        case LEB:
            b1 = et[nel + 3];
            i_state = LEB1;
            break;
        case LEB1:
            SHB1 if ((b1->tag & 0001) == TAGO) break;
            jsp++;
            et[nel + 1] = b1->prev;
            et[nel + 2] = b1;
            b2 = b1->info.codep;
            et[nel + 3] = b2;
            nel = nel + 4;
            i_state = ADVANCE;
            break;
            // PREB;
        case PREB:
            vpc = vpc + NMBL;
            putjs(jsp, &b1, &b2, &nel, &vpc);
            et[nel + 1] = b2->prev;
            et[nel + 2] = b2;
            i_state = NEXTOP;
            break;
            // PRVB;
        case PRVB:
            vpc = vpc + NMBL;
            putjs(jsp, &b1, &b2, &nel, &vpc);
            et[nel + 1] = b2->prev;
            SHB2 if ((b2->tag & 0001) != TAGO) b2 = b2->info.codep;
            et[nel + 2] = b2;
            i_state = NEXTOP;
            break;
            //  REB;
        case REB:
            b2 = et[nel + 2];
            i_state = REB1;
            break;
        case REB1:
            SHB2 if ((b2->tag & 0001) == TAGO) break;
            jsp++;
            et[nel] = b2->next;
            et[nel + 3] = b2;
            b1 = b2->info.codep;
            et[nel + 2] = b1;
            nel = nel + 4;
            i_state = ADVANCE;
            break;
            // EOE(N);
        case EOE:
            n = *(vpc + NMBL);
            jsp = jsp - n;
            vpc = vpc + 2 * NMBL;
            i_state = NEXTOP;
            break;
            // EOEI;
        case EOEI:
            jsp--;
            i_state = ADVANCE;
            break;
            // LSRCH(S);
        case LSRCH:
            et[nel] = b1->next;
            i_state = LSRCH1;
            break;
        case LSRCH1:
            SHB1 if ((b1->tag & 0001) != TAGO)
            {
                b1 = b1->info.codep;
                break;
            };
            if (memcmp(vpc + NMBL, &b1->tag, SMBL) != 0)
                break;
            et[nel + 1] = b1->prev;
            et[nel + 2] = b1;
            nel = nel + 3;
            vpc = vpc + NMBL + SMBL;
            i_state = NEXTOP;
            break;
            // RSRCH(S);
        case RSRCH:
            et[nel + 1] = b2->prev;
            i_state = RSRCH1;
            break;
        case RSRCH1:
            SHB2 if ((b2->tag & 0001) != TAGO)
            {
                b2 = b2->info.codep;
                break;
            };
            if (memcmp(vpc + NMBL, &b2->tag, SMBL) != 0)
                break;
            et[nel] = b2->next;
            et[nel + 2] = b2;
            nel = nel + 3;
            vpc = vpc + NMBL + SMBL;
            i_state = NEXTOP;
            break;
            // WSPC(L);
        case WSPC:
            if (!spc((T_SPCS *)(jsp + 1), vpc, et[nel - 1]))
            {
                i_state = FAIL;
                break;
            }
            vpc = vpc + NMBL + LBLL;
            i_state = NEXTOP;
            break;
            // ESPC(L);
        case ESPC:
            b0 = et[nel - 2]->prev;
            bool fail = false;
            while (b0 != et[nel - 1])
            {
                b0 = b0->next;
                if ((b0->tag & 0001) != TAGO)
                    b0 = b0->info.codep;
                if (!spc((T_SPCS *)(jsp + 1), vpc, b0))
                {
                    i_state = FAIL;
                    fail = true;
                    break;
                }
            };
            if (fail)
                break;
            vpc = vpc + NMBL + LBLL;
            i_state = NEXTOP;
            break;
            // PLESPC;
        case PLESPC:
            vpc = vpc + NMBL;
            putjs(jsp, &b1, &b2, &nel, &vpc);
            jsp++;
            et[nel] = b1->next;
            et[nel + 1] = b1;
            nel = nel + 2;
            vpc = vpc + NMBL + LBLL;
            i_state = NEXTOP;
            break;
            // LESPC(L);
        case LESPC:
            b1 = et[nel + 1];
            SHB1 if ((b1->tag & 0001) != TAGO) b1 = b1->info.codep;
            if (!spc((T_SPCS *)(jsp + 1), vpc, b1))
            {
                i_state = FAIL;
                break;
            }
            jsp++;
            et[nel + 1] = b1;
            nel = nel + 2;
            vpc = vpc + NMBL + LBLL;
            i_state = NEXTOP;
            break;
            // PRESPC;
        case PRESPC:
            vpc = vpc + NMBL;
            putjs(jsp, &b1, &b2, &nel, &vpc);
            jsp++;
            et[nel + 1] = b2->prev;
            et[nel] = b2;
            nel = nel + 2;
            vpc = vpc + NMBL + LBLL;
            i_state = NEXTOP;
            break;
            // RESPC(L);
        case RESPC:
            b2 = et[nel];
            SHB2 if ((b2->tag & 0001) != TAGO) b2 = b2->info.codep;
            if (!spc((T_SPCS *)(jsp + 1), vpc, b2))
            {
                i_state = FAIL;
                break;
            }
            jsp++;
            et[nel] = b2;
            nel = nel + 2;
            vpc = vpc + NMBL + LBLL;
            i_state = NEXTOP;
            break;
            // LMAX(L);
        case LMAX:
            et[nel] = b1->next;
            b1 = b1->next;
            while (b1 != b2)
            {
                if (!spc((T_SPCS *)(jsp + 1), vpc, b1))
                    break;
                if ((b1->tag & 0001) != TAGO)
                    b1 = b1->info.codep;
                b1 = b1->next;
            };
            b1 = b1->prev;
            et[nel + 1] = b1;
            nel = nel + 2;
            vpc = vpc + NMBL + LBLL;
            i_state = NEXTOP;
            break;
            // RMAX(L);
        case RMAX:
            et[nel + 1] = b2->prev;
            b2 = b2->prev;
            while (b2 != b1)
            {
                if (!spc((T_SPCS *)(jsp + 1), vpc, b2))
                    break;
                if ((b2->tag & 0001) != TAGO)
                    b2 = b2->info.codep;
                b2 = b2->prev;
            };
            b2 = b2->next;
            et[nel] = b2;
            nel = nel + 2;
            vpc = vpc + NMBL + LBLL;
            i_state = NEXTOP;
            break;
            // EOR;
        case EOR:
            f = flhead;
            lastk = &quasik;
            lastb = NULL;
            tsp = (T_TS *)js;
            i_state = ADVANCE;
            break;
            // NS(S);
        case NS:
            SHF
                memcpy(&f->tag, vpc + NMBL, SMBL);
            vpc = vpc + NMBL + SMBL;
            i_state = NEXTOP;
            break;
            // NSO(N);
        case NSO:
            SHF
                f->tag = TAGO;
            f->info.codep = NULL;
            f->info.infoc = (char)*(vpc + 1);
            vpc = vpc + NMBL + NMBL;
            i_state = NEXTOP;
            break;
            // TEXT(N,S1,S2,...,SN);
        case TEXT:
            n = *(vpc + NMBL);
            // printf("\n TEXT uc0=%x uii=%x  %d",u.c[0],u.ii,u.ii);
            vpc = vpc + NMBL + NMBL;
            bool lack = false;
            for (i = 1; i <= n; i++)
            {
                f = f->next;
                if (f == flhead)
                {
                    i_state = LACK;
                    lack = true;
                    break;
                }
                f->tag = TAGO;
                f->info.codep = NULL;
                f->info.infoc = (char)*vpc;
                vpc = vpc + NMBL;
            };
            if (lack)
                break;
            i_state = NEXTOP;
            break;
            // BL;
        case BL:
            SHF
                f->info.codep = lastb;
            lastb = f;
            i_state = ADVANCE;
            break;
            // BR;
        case BR:
            SHF
                f->info.codep = lastb;
            f1 = lastb->info.codep;
            lastb->tag = TAGLB;
            lastb->info.codep = f;
            f->tag = TAGRB;
            lastb = f1;
            i_state = ADVANCE;
            break;
            // BLR;
        case BLR:
            SHF
                f1 = f;
            SHF
                f1->info.codep = f;
            f->info.codep = f1;
            f1->tag = TAGLB;
            f->tag = TAGRB;
            i_state = ADVANCE;
            break;
            // BRACT;
        case BRACT:
            SHF
                f->info.codep = lastb;
            f->tag = TAGD;
            lastk->info.codep = f;
            lastk->tag = TAGK;
            lastk = lastb;
            lastb = lastb->info.codep;
            i_state = ADVANCE;
            break;
            // ACT(N);
        case ACT:
            n = *(vpc + NMBL);
            lastk->info.codep = et[n];
            lastk->tag = TAGK;
            lastk = et[n]->info.codep;
            et[n]->tag = TAGD;
            vpc = vpc + NMBL + NMBL;
            i_state = NEXTOP;
            break;
            // MULS;
        case MULS:
            n = *(vpc + NMBL);
            SHF
                memcpy(&f->tag, &et[n]->tag, SMBL);
            vpc = vpc + NMBL + NMBL;
            i_state = NEXTOP;
            break;
            // MULE(N);
        case MULE:
            //                    if(ferr == 1){
            //                        lastb = lastb1;
            //                        ferr = 0;}
            //                    else
            //                        lastb1 = lastb;
            n = *(vpc + NMBL);
            vpc = vpc + NMBL + NMBL;
            f0 = et[n - 1]->prev;
            lack = false;
            while (f0 != et[n])
            {
                f0 = f0->next;
                f = f->next;
                if (f == flhead)
                {
                    i_state = LACK;
                    lack = true;
                    break;
                } //              f = f->next;       instead of SHF
                //             if (f == flhead){
                //               ferr = 1;
                //               goto LACK;}
                if ((f0->tag & 0001) != TAGO)
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
                else
                    memcpy(&f->tag, &f0->tag, SMBL);
            };
            if (lack)
                break;
            i_state = NEXTOP;
            break;
            // TPL(N,M);
            // TPLM(N,M);
        case TPL:
        case TPLM:
            n = *(vpc + NMBL);
            m = *(vpc + NMBL + NMBL);
            vpc = vpc + NMBL * 3;
            if (et[m]->next == et[n])
            {
                i_state = NEXTOP;
                break;
            }
            putts(tsp, &f, &et[n], &et[m]);
            tsp++;
            i_state = NEXTOP;
            break;
            // TPLE(N); (= TPL(N-1,N);)
            // TPLV(N);
        case TPLE:
        case TPLV:
            n = *(vpc + NMBL);
            vpc = vpc + NMBL + NMBL;
            if (et[n]->next == et[n - 1])
            {
                i_state = NEXTOP;
                break;
            }
            putts(tsp, &f, &et[n - 1], &et[n]);
            tsp++;
            i_state = NEXTOP;
            break;
            // TPLS(N); (= TPLM(N,N);)
        case TPLS:
            n = *(vpc + NMBL);
            vpc = vpc + NMBL + NMBL;
            putts(tsp, &f, &et[n], &et[n]);
            tsp++;
            i_state = NEXTOP;
            break;
            // EOS;
        case EOS:
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
            i_state = ADVSTEP;
            break;
            // SWAP;
            //  static box head is after operator code
        case SWAP:
            vpc = vpc + NMBL;
            f = (T_LINKCB *)vpc;
            if (f->prev != NULL)
            {
                i_state = SWAPREF;
                break;
            }
            link(f, f);
            f->info.codep = refal.svar;
            f->tag = TAGO;
            refal.svar = f;
            i_state = SWAPREF;
            break;
        case SWAPREF:
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
            i_state = ADVSTEP;
            break;
            // BLF(L);
        case BLF:
            SHF
                f->info.codep = lastb;
            lastb = f;
            SHF
                memcpy(&f->info.codef, vpc + NMBL, LBLL);
            f->tag = TAGF;
            vpc = vpc + NMBL + LBLL;
            i_state = NEXTOP;
            break;
            // EOSSN (NN);
            /*case EOSSN:
                memcpy(&refal.stmnmb, vpc + NMBL, NMBL + NMBL);
                i_state = EOS;
                break; */
            // SETNOS(L);
            /*case SETNOS:
                memcpy(&inch.inr, vpc + NMBL, LBLL);
                refal.nostm = *(inch.inr);
                vpc = vpc + NMBL + LBLL;
                i_state = NEXTOP;
                break; */
            // C-refal-function execution
        case CFUNC:
            memcpy(&fptr, vpc + NMBL + Z_0, LBLL);
            refal.upshot = 1;
            refal.prevr = b0->prev;
            refal.nextr = b0;
            refal.preva = b1;
            refal.nexta = b2;
            //        call  C - function
            (*fptr)();
            switch (refal.upshot)
            {
            case 1:
                i_state = CFDONE;
                break;
            case 2:
                i_state = RCGIMP;
                break;
            case 3:
                i_state = CFLACK;
                break;
            default:
                i_state = CFDONE;
            }
            break;
            //        return from C - function
            //          step is done
        case CFDONE:
            quasik.info.codep = refal.nextr->info.codep;
            link(refal.nextr->prev, refal.nexta->next);
            link(refal.nexta, flhead->next);
            link(flhead, refal.nextr);
            i_state = ADVSTEP;
            break;
            //        return from C - function
            //     free memory exhausted
        case CFLACK:
            if (refal.prevr->next != refal.nextr)
            {
                link(refal.nextr->prev, flhead->next);
                link(flhead, refal.prevr->next);
                link(refal.prevr, refal.nextr);
            }
            ast->state = 3;
            i_state = EXIT;
        }
}

//------------ end of file RFRUN1.C ----------
