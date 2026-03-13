// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-03-12
// https://github.com/Aleksandr3Bocharov/refalab

//----------- file RFRUN.C -------------------
//      Refal-interpretator
//---------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "refalab.h"
#include "rfrun.h"
#include "rfintf.h"

// for unlooping:

#define SHB1                                 \
    left_board_hole = left_board_hole->next; \
    if (left_board_hole == right_board_hole) \
    {                                        \
        i_state = FAIL;                      \
        break;                               \
    }

#define SHB2                                   \
    right_board_hole = right_board_hole->prev; \
    if (right_board_hole == left_board_hole)   \
    {                                          \
        i_state = FAIL;                        \
        break;                                 \
    }

#define SHF             \
    f = f->next;        \
    if (f == flhead)    \
    {                   \
        i_state = LACK; \
        break;          \
    }

#define BRA(b) (b->tag & 0001) != 0

#define NBRA(b) (b->tag & 0001) == 0

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
    CFUNC,
    CFDONE,
    CFLACK
} T_I_STATES;

typedef enum sp_states
{
    SPCRET,
    SPCNXT,
    SPCCLL,
    SPCW,
    SPCNG,
    SPCNGW,
    SPCSC,
    SPCS,
    SPCB,
    SPCF,
    SPCN,
    SPCR,
    SPCD,
    SPCO,
    SPCL
} T_SP_STATES;

typedef struct sav_
{ // save area for var-part of refal-block
    uint32_t upshot_;
    T_LINKCB *preva_;
    T_LINKCB *nexta_;
    T_LINKCB *prevr_;
    T_LINKCB *nextr_;
    T_ST *currst_;
} T_SAV;

typedef struct w_jump_stack
{ // jump stack structure
    T_LINKCB *left_board_hole;
    T_LINKCB *right_board_hole;
    uint16_t number_element;
    uint8_t *virtual_program_counter;
} T_W_JUMP_STACK;

typedef struct ts
{ // translation stack structure
    T_LINKCB *ts0;
    T_LINKCB *ts1;
    T_LINKCB *ts2;
} T_TS;

typedef struct spcs
{
    bool spls;
    uint8_t *svpc;
} T_SPCS;

uint8_t *inch_ptr;

static T_LINKCB *table_elements[256]; // table of elements
static uint16_t number_element;       // adress of first free string in table of elements

static T_W_JUMP_STACK jump_stack[64];      // jump stack and planning translation stack
static T_W_JUMP_STACK *jump_stack_pointer; // jump stack pointer

static T_TS *tsp; // translation stack pointer

// definition of work variables and pointers
static uint8_t operation_program_code; // current statement code
static uint8_t *virtual_program_counter;                   // virtual program counter
static T_LINKCB *lastk;                // last acted sign-k adress
static T_LINKCB *lastb;                // last generated left bracket
static T_LINKCB *b0, *left_board_hole, *right_board_hole;
static T_LINKCB *f0, *f1, *f;
static const uint8_t *vpca; // additional virtual program counter
static uint8_t i, n, m;

static void (*fptr)(void);

static bool spc(T_SPCS *pspcsp, const uint8_t *vpc_, const T_LINKCB *b);
static bool letter(char s);
static bool digit(char s);
static void link(T_LINKCB *x, T_LINKCB *y);
static void putjs(T_W_JUMP_STACK *jsp_, T_LINKCB **ab1, T_LINKCB **ab2, const uint16_t *anel, uint8_t **avpc);
static void getjs(const T_W_JUMP_STACK *jsp_, T_LINKCB **ab1, T_LINKCB **ab2, uint16_t *anel, uint8_t **avpc);
static void putts(T_TS *tsp_, T_LINKCB **ax, T_LINKCB **ay, T_LINKCB **az);
static void getts(const T_TS *tsp_, T_LINKCB **ax, T_LINKCB **ay, T_LINKCB **az);

void rfrun(T_ST *ast) // adress of current state table
{
    // dynamic area DSA
    T_SAV *savecr = malloc(sizeof(T_SAV));
    if (savecr == NULL)
        rfabe("rfrun: no memory");
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
    refal.currst = ast;
    ast->state = 4;
    T_LINKCB quasik; // quasi-sign k
    quasik.info.codep = ast->dot;
    // adress of free memory list head
    T_LINKCB *flhead = refal.flhead;
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
            right_board_hole = quasik.info.codep;
            if (right_board_hole == NULL)
            {
                i_state = DONE;
                break;
            }
            b0 = right_board_hole->info.codep;
            left_board_hole = b0->next;
            virtual_program_counter = left_board_hole->info.codef;
            if (left_board_hole->tag != TAGF)
            {
                i_state = REF;
                break;
            }
            // here must be check on c-function
            // if (c) goto CFUNC;
            jump_stack_pointer = jump_stack;
            table_elements[1] = b0;
            table_elements[2] = right_board_hole;
            table_elements[3] = left_board_hole;
            number_element = 4;
            i_state = NEXTOP;
            break;
            // increase step number
        case ADVSTEP:
            ast->step++;
            i_state = START;
            break;
            // symbol - reference execution
        case REF:
            if (left_board_hole->tag != TAGR)
            {
                i_state = RCGIMP;
                break;
            }
            table_elements[1] = b0;
            table_elements[2] = right_board_hole;
            table_elements[3] = left_board_hole;
            f = (T_LINKCB *)virtual_program_counter;
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
            quasik.info.codep = table_elements[2];
            i_state = EXIT;
            break;
            // state remove from
        case EXIT:
            ast->dot = quasik.info.codep;
            // restore refal-block
            refal.upshot = savecr->upshot_;
            refal.preva = savecr->preva_;
            refal.nexta = savecr->nexta_;
            refal.prevr = savecr->prevr_;
            refal.nextr = savecr->nextr_;
            refal.currst = savecr->currst_;
            free(savecr);
            return;
        case NEXTOP:
            operation_program_code = *virtual_program_counter;
#if defined mdebug
            fprintf(stderr, " Switcher");
            fprintf(stderr, " code8=%o\t(D=%d,H=%x)\n", operation_program_code, operation_program_code, operation_program_code);
#endif
            // operation switcher
            switch (operation_program_code)
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
            case 0122:
                i_state = CFUNC;
            };
            break;
            // select next statement assembly language
        case ADVANCE:
            virtual_program_counter += NMBL;
            i_state = NEXTOP;
            break;
            // SJUMP(L);
        case SJUMP:
            memcpy(&inch_ptr, vpc + NMBL, LBLL);
            putjs(jump_stack_pointer, &left_board_hole, &right_board_hole, &number_element, &inch_ptr);
            jump_stack_pointer++;
            vpc = vpc + NMBL + LBLL;
            i_state = NEXTOP;
            break;
            // FAIL;
        case FAIL:
            if (jump_stack_pointer == jump_stack)
            {
                i_state = RCGIMP;
                break;
            }
            jump_stack_pointer--;
            getjs(jump_stack_pointer, &left_board_hole, &right_board_hole, &number_element, &vpc);
            i_state = NEXTOP;
            break;
            // SB(N,M);
        case SB:
            n = *(vpc + NMBL);
            m = *(vpc + NMBL + NMBL);
            left_board_hole = table_elements[n];
            right_board_hole = table_elements[m];
            vpc = vpc + 3 * NMBL;
            i_state = NEXTOP;
            break;
            // LSC(S);
        case LSC:
            SHB1;
            if (memcmp(vpc + NMBL, &left_board_hole->tag, SMBL) != 0)
            {
                i_state = FAIL;
                break;
            }
            vpc = vpc + NMBL + SMBL;
            table_elements[number_element] = left_board_hole;
            number_element++;
            i_state = NEXTOP;
            break;
            // RSC(S);
        case RSC:
            SHB2;
            if (memcmp(vpc + NMBL, &right_board_hole->tag, SMBL) != 0)
            {
                i_state = FAIL;
                break;
            }
            vpc = vpc + NMBL + SMBL;
            table_elements[number_element] = right_board_hole;
            number_element++;
            i_state = NEXTOP;
            break;
            // LSCO(N);
        case LSCO:
            SHB1;
            if (left_board_hole->tag != TAGO)
            {
                i_state = FAIL;
                break;
            }
            if (left_board_hole->info.infoc != (char)*(vpc + NMBL))
            {
                i_state = FAIL;
                break;
            }
            table_elements[number_element] = left_board_hole;
            number_element++;
            vpc = vpc + NMBL + NMBL;
            i_state = NEXTOP;
            break;
            // RSCO(N);
        case RSCO:
            SHB2;
            if (right_board_hole->tag != TAGO)
            {
                i_state = FAIL;
                break;
            }
            if (right_board_hole->info.infoc != (char)*(vpc + NMBL))
            {
                i_state = FAIL;
                break;
            }
            table_elements[number_element] = right_board_hole;
            number_element++;
            vpc = vpc + NMBL + NMBL;
            i_state = NEXTOP;
            break;
            // LSD(N);
        case LSD:
            SHB1;
            n = *(vpc + NMBL);
            if (left_board_hole->tag != table_elements[n]->tag)
            {
                i_state = FAIL;
                break;
            }
            if (left_board_hole->info.codep != table_elements[n]->info.codep)
            {
                i_state = FAIL;
                break;
            }
            table_elements[number_element] = left_board_hole;
            number_element++;
            vpc = vpc + NMBL + NMBL;
            i_state = NEXTOP;
            break;
            // RSD(N);
        case RSD:
            SHB2;
            n = *(vpc + NMBL);
            if (right_board_hole->tag != table_elements[n]->tag)
            {
                i_state = FAIL;
                break;
            }
            if (right_board_hole->info.codep != table_elements[n]->info.codep)
            {
                i_state = FAIL;
                break;
            }
            table_elements[number_element] = right_board_hole;
            number_element++;
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
            SHB1;
            if (left_board_hole->tag != TAGO)
            {
                i_state = FAIL;
                break;
            }
            if (left_board_hole->info.infoc != (char)*vpc)
            {
                i_state = FAIL;
                break;
            }
            table_elements[number_element] = left_board_hole;
            number_element++;
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
            SHB2;
            if (right_board_hole->tag != TAGO)
            {
                i_state = FAIL;
                break;
            }
            if (right_board_hole->info.infoc != (char)*vpc)
            {
                i_state = FAIL;
                break;
            }
            table_elements[number_element] = right_board_hole;
            number_element++;
            vpc = vpc + NMBL;
            n--;
            if (n != 0)
                break;
            i_state = NEXTOP;
            break;
            // LB;
        case LB:
            SHB1;
            if (NBRA(left_board_hole))
            {
                i_state = FAIL;
                break;
            }
            right_board_hole = left_board_hole->info.codep;
            table_elements[number_element] = left_board_hole;
            table_elements[number_element + 1] = right_board_hole;
            number_element += 2;
            i_state = ADVANCE;
            break;
            // LBY;
        case LBY:
            SHB1;
            if (NBRA(left_board_hole))
            {
                i_state = FAIL;
                break;
            }
            table_elements[number_element] = left_board_hole;
            left_board_hole = left_board_hole->info.codep;
            table_elements[number_element + 1] = left_board_hole;
            number_element += 2;
            i_state = ADVANCE;
            break;
            // RB;
        case RB:
            SHB2;
            if (NBRA(right_board_hole))
            {
                i_state = FAIL;
                break;
            }
            left_board_hole = right_board_hole->info.codep;
            table_elements[number_element] = left_board_hole;
            table_elements[number_element + 1] = right_board_hole;
            number_element += 2;
            i_state = ADVANCE;
            break;
            // RBY;
        case RBY:
            SHB2;
            if (NBRA(right_board_hole))
            {
                i_state = FAIL;
                break;
            }
            table_elements[number_element + 1] = right_board_hole;
            right_board_hole = right_board_hole->info.codep;
            table_elements[number_element] = right_board_hole;
            number_element += 2;
            i_state = ADVANCE;
            break;
            // LS;
        case LS:
            SHB1;
            if (BRA(left_board_hole))
            {
                i_state = FAIL;
                break;
            }
            table_elements[number_element] = left_board_hole;
            number_element++;
            i_state = ADVANCE;
            break;
            // RS;
        case RS:
            SHB2;
            if (BRA(right_board_hole))
            {
                i_state = FAIL;
                break;
            }
            table_elements[number_element] = right_board_hole;
            number_element++;
            i_state = ADVANCE;
            break;
            // LW;
        case LW:
            SHB1;
            table_elements[number_element] = left_board_hole;
            if (BRA(left_board_hole))
                left_board_hole = left_board_hole->info.codep;
            table_elements[number_element + 1] = left_board_hole;
            number_element += 2;
            i_state = ADVANCE;
            break;
            // RW;
        case RW:
            SHB2;
            table_elements[number_element + 1] = right_board_hole;
            if (BRA(right_board_hole))
                right_board_hole = right_board_hole->info.codep;
            table_elements[number_element] = right_board_hole;
            number_element += 2;
            i_state = ADVANCE;
            break;
            // LBNIL
        case LBNIL:
            SHB1;
            if (NBRA(left_board_hole))
            {
                i_state = FAIL;
                break;
            }
            b0 = left_board_hole;
            left_board_hole = left_board_hole->info.codep;
            if (b0->next != left_board_hole)
            {
                i_state = FAIL;
                break;
            }
            table_elements[number_element] = b0;
            table_elements[number_element + 1] = left_board_hole;
            number_element += 2;
            i_state = ADVANCE;
            break;
            // RBNIL;
        case RBNIL:
            SHB2;
            if (NBRA(right_board_hole))
            {
                i_state = FAIL;
                break;
            }
            b0 = right_board_hole;
            right_board_hole = right_board_hole->info.codep;
            if (right_board_hole->next != b0)
            {
                i_state = FAIL;
                break;
            }
            table_elements[number_element] = right_board_hole;
            table_elements[number_element + 1] = b0;
            number_element += 2;
            i_state = ADVANCE;
            break;
            // LBCE;
        case LBCE:
            SHB1;
            if (NBRA(left_board_hole))
            {
                i_state = FAIL;
                break;
            }
            b0 = left_board_hole;
            left_board_hole = left_board_hole->info.codep;
            table_elements[number_element] = b0;
            table_elements[number_element + 1] = left_board_hole;
            table_elements[number_element + 2] = b0->next;
            table_elements[number_element + 3] = left_board_hole->prev;
            number_element += 4;
            i_state = ADVANCE;
            break;
            // RBCE;
        case RBCE:
            SHB2;
            if (NBRA(right_board_hole))
            {
                i_state = FAIL;
                break;
            }
            b0 = right_board_hole;
            right_board_hole = right_board_hole->info.codep;
            table_elements[number_element] = right_board_hole;
            table_elements[number_element + 1] = b0;
            table_elements[number_element + 2] = right_board_hole->next;
            table_elements[number_element + 3] = b0->prev;
            number_element += 4;
            i_state = ADVANCE;
            break;
            // NIL;
        case NIL:
            if (left_board_hole->next != right_board_hole)
            {
                i_state = FAIL;
                break;
            }
            i_state = ADVANCE;
            break;
            // CE;
        case CE:
            table_elements[number_element] = left_board_hole->next;
            table_elements[number_element + 1] = right_board_hole->prev;
            number_element += 2;
            i_state = ADVANCE;
            break;
            // LED(N);
        case LED:
            n = *(vpc + NMBL);
            table_elements[number_element] = left_board_hole->next;
            b0 = table_elements[n - 1]->prev;
            i_state = LED1;
            break;
        case LED1:
            if (b0 == table_elements[n])
            {
                i_state = LED2;
                break;
            }
            b0 = b0->next;
            SHB1;
            if (left_board_hole->tag != b0->tag)
            {
                i_state = FAIL;
                break;
            }
            if (left_board_hole->info.codep == b0->info.codep)
                break;
            if (BRA(left_board_hole))
                break;
            i_state = FAIL;
            break;
        case LED2:
            table_elements[number_element + 1] = left_board_hole;
            number_element += 2;
            vpc = vpc + NMBL * 2;
            i_state = NEXTOP;
            break;
            // RED(N);
        case RED:
            n = *(vpc + NMBL);
            table_elements[number_element + 1] = right_board_hole->prev;
            b0 = table_elements[n]->next;
            i_state = RED1;
            break;
        case RED1:
            if (b0 == table_elements[n - 1])
            {
                i_state = RED2;
                break;
            }
            b0 = b0->prev;
            SHB2;
            if (right_board_hole->tag != b0->tag)
            {
                i_state = FAIL;
                break;
            }
            if (right_board_hole->info.codep == b0->info.codep)
                break;
            if (BRA(right_board_hole))
                break;
            i_state = FAIL;
            break;
        case RED2:
            table_elements[number_element] = right_board_hole;
            number_element += 2;
            vpc = vpc + NMBL * 2;
            i_state = NEXTOP;
            break;
            // NNIL;
        case NNIL:
            if (table_elements[number_element - 1]->next == table_elements[number_element - 2])
            {
                i_state = FAIL;
                break;
            }
            i_state = ADVANCE;
            break;
            // PLE;
        case PLE:
            vpc = vpc + NMBL;
            putjs(jump_stack_pointer, &left_board_hole, &right_board_hole, &number_element, &vpc);
            jump_stack_pointer++;
            table_elements[number_element] = left_board_hole->next;
            table_elements[number_element + 1] = left_board_hole;
            number_element += 2;
            i_state = ADVANCE;
            break;
            // PLV;
        case PLV:
            vpc = vpc + NMBL;
            putjs(jump_stack_pointer, &left_board_hole, &right_board_hole, &number_element, &vpc);
            table_elements[number_element] = left_board_hole->next;
            table_elements[number_element + 1] = left_board_hole;
            i_state = NEXTOP;
            break;
            // LE;
        case LE:
            left_board_hole = table_elements[number_element + 1];
            SHB1;
            if (BRA(left_board_hole))
                left_board_hole = left_board_hole->info.codep;
            jump_stack_pointer++;
            table_elements[number_element + 1] = left_board_hole;
            number_element += 2;
            i_state = ADVANCE;
            break;
            // PRE;
        case PRE:
            vpc = vpc + NMBL;
            putjs(jump_stack_pointer, &left_board_hole, &right_board_hole, &number_element, &vpc);
            jump_stack_pointer++;
            table_elements[number_element] = right_board_hole;
            table_elements[number_element + 1] = right_board_hole->prev;
            number_element += 2;
            i_state = ADVANCE;
            break;
            // PRV;
        case PRV:
            vpc = vpc + NMBL;
            putjs(jump_stack_pointer, &left_board_hole, &right_board_hole, &number_element, &vpc);
            table_elements[number_element] = right_board_hole;
            table_elements[number_element + 1] = right_board_hole->prev;
            i_state = NEXTOP;
            break;
            // RE;
        case RE:
            right_board_hole = table_elements[number_element];
            SHB2;
            if (BRA(right_board_hole))
                right_board_hole = right_board_hole->info.codep;
            jump_stack_pointer++;
            table_elements[number_element] = right_board_hole;
            number_element += 2;
            i_state = ADVANCE;
            break;
            // PLESC;
        case PLESC:
            vpc = vpc + NMBL;
            putjs(jump_stack_pointer, &left_board_hole, &right_board_hole, &number_element, &vpc);
            table_elements[number_element] = left_board_hole->next;
            table_elements[number_element + 2] = left_board_hole;
            i_state = NEXTOP;
            break;
            // PLVSC;
        case PLVSC:
            vpc = vpc + NMBL;
            putjs(jump_stack_pointer, &left_board_hole, &right_board_hole, &number_element, &vpc);
            table_elements[number_element] = left_board_hole->next;
            SHB1;
            if (BRA(left_board_hole))
                left_board_hole = left_board_hole->info.codep;
            table_elements[number_element + 2] = left_board_hole;
            i_state = NEXTOP;
            break;
            // LESC(S);
        case LESC:
            vpc = vpc + NMBL + SMBL;
            vpca = vpc - SMBL;
            i_state = LESC0;
            break;
        case LESC0:
            left_board_hole = table_elements[number_element + 2];
            i_state = LESC1;
            break;
        case LESC1:
            SHB1;
            if (BRA(left_board_hole))
            {
                left_board_hole = left_board_hole->info.codep;
                break;
            }
            if (memcmp(vpca, &left_board_hole->tag, SMBL) != 0)
                break;
            jump_stack_pointer++;
            table_elements[number_element + 1] = left_board_hole->prev;
            table_elements[number_element + 2] = left_board_hole;
            number_element += 3;
            i_state = NEXTOP;
            break;
            // PRESC;
        case PRESC:
            vpc = vpc + NMBL;
            putjs(jump_stack_pointer, &left_board_hole, &right_board_hole, &number_element, &vpc);
            table_elements[number_element + 1] = right_board_hole->prev;
            table_elements[number_element + 2] = right_board_hole;
            i_state = NEXTOP;
            break;
            // PRVSC;
        case PRVSC:
            vpc = vpc + NMBL;
            putjs(jump_stack_pointer, &left_board_hole, &right_board_hole, &number_element, &vpc);
            table_elements[number_element + 1] = right_board_hole->prev;
            SHB2;
            if (BRA(right_board_hole))
                right_board_hole = right_board_hole->info.codep;
            table_elements[number_element + 2] = right_board_hole;
            i_state = NEXTOP;
            break;
            // RESC(S);
        case RESC:
            vpc = vpc + NMBL + SMBL;
            vpca = vpc - SMBL;
            i_state = RESC0;
            break;
        case RESC0:
            right_board_hole = table_elements[number_element + 2];
            i_state = RESC1;
            break;
        case RESC1:
            SHB2;
            if (BRA(right_board_hole))
            {
                right_board_hole = right_board_hole->info.codep;
                break;
            }
            if (memcmp(vpca, &right_board_hole->tag, SMBL) != 0)
                break;
            jump_stack_pointer++;
            table_elements[number_element + 2] = right_board_hole;
            table_elements[number_element] = right_board_hole->next;
            number_element += 3;
            i_state = NEXTOP;
            break;
            // LESD(N);
        case LESD:
            n = *(vpc + NMBL);
            vpca = (uint8_t *)&table_elements[n]->tag;
            vpc = vpc + NMBL * 2;
            i_state = LESC0;
            break;
            // RESD(N);
        case RESD:
            n = *(vpc + NMBL);
            vpca = (uint8_t *)&table_elements[n]->tag;
            vpc = vpc + NMBL * 2;
            i_state = RESC0;
            break;
            // PLEB;
        case PLEB:
            vpc = vpc + NMBL;
            putjs(jump_stack_pointer, &left_board_hole, &right_board_hole, &number_element, &vpc);
            table_elements[number_element] = left_board_hole->next;
            table_elements[number_element + 3] = left_board_hole;
            i_state = NEXTOP;
            break;
            // PLVB;
        case PLVB:
            vpc = vpc + NMBL;
            putjs(jump_stack_pointer, &left_board_hole, &right_board_hole, &number_element, &vpc);
            table_elements[number_element] = left_board_hole->next;
            SHB1;
            if (BRA(left_board_hole))
                left_board_hole = left_board_hole->info.codep;
            table_elements[number_element + 3] = left_board_hole;
            i_state = NEXTOP;
            break;
            // LEB;
        case LEB:
            left_board_hole = table_elements[number_element + 3];
            i_state = LEB1;
            break;
        case LEB1:
            SHB1;
            if (NBRA(left_board_hole))
                break;
            jump_stack_pointer++;
            table_elements[number_element + 1] = left_board_hole->prev;
            table_elements[number_element + 2] = left_board_hole;
            right_board_hole = left_board_hole->info.codep;
            table_elements[number_element + 3] = right_board_hole;
            number_element += 4;
            i_state = ADVANCE;
            break;
            // PREB;
        case PREB:
            vpc = vpc + NMBL;
            putjs(jump_stack_pointer, &left_board_hole, &right_board_hole, &number_element, &vpc);
            table_elements[number_element + 1] = right_board_hole->prev;
            table_elements[number_element + 2] = right_board_hole;
            i_state = NEXTOP;
            break;
            // PRVB;
        case PRVB:
            vpc = vpc + NMBL;
            putjs(jump_stack_pointer, &left_board_hole, &right_board_hole, &number_element, &vpc);
            table_elements[number_element + 1] = right_board_hole->prev;
            SHB2;
            if (BRA(right_board_hole))
                right_board_hole = right_board_hole->info.codep;
            table_elements[number_element + 2] = right_board_hole;
            i_state = NEXTOP;
            break;
            //  REB;
        case REB:
            right_board_hole = table_elements[number_element + 2];
            i_state = REB1;
            break;
        case REB1:
            SHB2;
            if (NBRA(right_board_hole))
                break;
            jump_stack_pointer++;
            table_elements[number_element] = right_board_hole->next;
            table_elements[number_element + 3] = right_board_hole;
            left_board_hole = right_board_hole->info.codep;
            table_elements[number_element + 2] = left_board_hole;
            number_element += 4;
            i_state = ADVANCE;
            break;
            // EOE(N);
        case EOE:
            n = *(vpc + NMBL);
            jump_stack_pointer -= n;
            vpc = vpc + 2 * NMBL;
            i_state = NEXTOP;
            break;
            // EOEI;
        case EOEI:
            jump_stack_pointer--;
            i_state = ADVANCE;
            break;
            // LSRCH(S);
        case LSRCH:
            table_elements[number_element] = left_board_hole->next;
            i_state = LSRCH1;
            break;
        case LSRCH1:
            SHB1;
            if (BRA(left_board_hole))
            {
                left_board_hole = left_board_hole->info.codep;
                break;
            };
            if (memcmp(vpc + NMBL, &left_board_hole->tag, SMBL) != 0)
                break;
            table_elements[number_element + 1] = left_board_hole->prev;
            table_elements[number_element + 2] = left_board_hole;
            number_element += 3;
            vpc = vpc + NMBL + SMBL;
            i_state = NEXTOP;
            break;
            // RSRCH(S);
        case RSRCH:
            table_elements[number_element + 1] = right_board_hole->prev;
            i_state = RSRCH1;
            break;
        case RSRCH1:
            SHB2;
            if (BRA(right_board_hole))
            {
                right_board_hole = right_board_hole->info.codep;
                break;
            };
            if (memcmp(vpc + NMBL, &right_board_hole->tag, SMBL) != 0)
                break;
            table_elements[number_element] = right_board_hole->next;
            table_elements[number_element + 2] = right_board_hole;
            number_element += 3;
            vpc = vpc + NMBL + SMBL;
            i_state = NEXTOP;
            break;
            // WSPC(L);
        case WSPC:
            if (!spc((T_SPCS *)(jump_stack_pointer + 1), vpc, table_elements[number_element - 1]))
            {
                i_state = FAIL;
                break;
            }
            vpc = vpc + NMBL + LBLL;
            i_state = NEXTOP;
            break;
            // ESPC(L);
        case ESPC:
            b0 = table_elements[number_element - 2]->prev;
            bool fail = false;
            while (b0 != table_elements[number_element - 1])
            {
                b0 = b0->next;
                if (BRA(b0))
                    b0 = b0->info.codep;
                if (!spc((T_SPCS *)(jump_stack_pointer + 1), vpc, b0))
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
            putjs(jump_stack_pointer, &left_board_hole, &right_board_hole, &number_element, &vpc);
            jump_stack_pointer++;
            table_elements[number_element] = left_board_hole->next;
            table_elements[number_element + 1] = left_board_hole;
            number_element += 2;
            vpc = vpc + NMBL + LBLL;
            i_state = NEXTOP;
            break;
            // LESPC(L);
        case LESPC:
            left_board_hole = table_elements[number_element + 1];
            SHB1;
            if (BRA(left_board_hole))
                left_board_hole = left_board_hole->info.codep;
            if (!spc((T_SPCS *)(jump_stack_pointer + 1), vpc, left_board_hole))
            {
                i_state = FAIL;
                break;
            }
            jump_stack_pointer++;
            table_elements[number_element + 1] = left_board_hole;
            number_element += 2;
            vpc = vpc + NMBL + LBLL;
            i_state = NEXTOP;
            break;
            // PRESPC;
        case PRESPC:
            vpc = vpc + NMBL;
            putjs(jump_stack_pointer, &left_board_hole, &right_board_hole, &number_element, &vpc);
            jump_stack_pointer++;
            table_elements[number_element + 1] = right_board_hole->prev;
            table_elements[number_element] = right_board_hole;
            number_element += 2;
            vpc = vpc + NMBL + LBLL;
            i_state = NEXTOP;
            break;
            // RESPC(L);
        case RESPC:
            right_board_hole = table_elements[number_element];
            SHB2;
            if (BRA(right_board_hole))
                right_board_hole = right_board_hole->info.codep;
            if (!spc((T_SPCS *)(jump_stack_pointer + 1), vpc, right_board_hole))
            {
                i_state = FAIL;
                break;
            }
            jump_stack_pointer++;
            table_elements[number_element] = right_board_hole;
            number_element += 2;
            vpc = vpc + NMBL + LBLL;
            i_state = NEXTOP;
            break;
            // LMAX(L);
        case LMAX:
            table_elements[number_element] = left_board_hole->next;
            left_board_hole = left_board_hole->next;
            while (left_board_hole != right_board_hole)
            {
                if (!spc((T_SPCS *)(jump_stack_pointer + 1), vpc, left_board_hole))
                    break;
                if (BRA(left_board_hole))
                    left_board_hole = left_board_hole->info.codep;
                left_board_hole = left_board_hole->next;
            };
            left_board_hole = left_board_hole->prev;
            table_elements[number_element + 1] = left_board_hole;
            number_element += 2;
            vpc = vpc + NMBL + LBLL;
            i_state = NEXTOP;
            break;
            // RMAX(L);
        case RMAX:
            table_elements[number_element + 1] = right_board_hole->prev;
            right_board_hole = right_board_hole->prev;
            while (right_board_hole != left_board_hole)
            {
                if (!spc((T_SPCS *)(jump_stack_pointer + 1), vpc, right_board_hole))
                    break;
                if (BRA(right_board_hole))
                    right_board_hole = right_board_hole->info.codep;
                right_board_hole = right_board_hole->prev;
            };
            right_board_hole = right_board_hole->next;
            table_elements[number_element] = right_board_hole;
            number_element += 2;
            vpc = vpc + NMBL + LBLL;
            i_state = NEXTOP;
            break;
            // EOR;
        case EOR:
            f = flhead;
            lastk = &quasik;
            lastb = NULL;
            tsp = (T_TS *)jump_stack;
            i_state = ADVANCE;
            break;
            // NS(S);
        case NS:
            SHF;
            memcpy(&f->tag, vpc + NMBL, SMBL);
            vpc = vpc + NMBL + SMBL;
            i_state = NEXTOP;
            break;
            // NSO(N);
        case NSO:
            SHF;
            f->tag = TAGO;
            f->info.codep = NULL;
            f->info.infoc = (char)*(vpc + 1);
            vpc = vpc + NMBL + NMBL;
            i_state = NEXTOP;
            break;
            // TEXT(N,S1,S2,...,SN);
        case TEXT:
            n = *(vpc + NMBL);
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
            SHF;
            f->info.codep = lastb;
            lastb = f;
            i_state = ADVANCE;
            break;
            // BR;
        case BR:
            SHF;
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
            SHF;
            f1 = f;
            SHF;
            f1->info.codep = f;
            f->info.codep = f1;
            f1->tag = TAGLB;
            f->tag = TAGRB;
            i_state = ADVANCE;
            break;
            // BRACT;
        case BRACT:
            SHF;
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
            lastk->info.codep = table_elements[n];
            lastk->tag = TAGK;
            lastk = table_elements[n]->info.codep;
            table_elements[n]->tag = TAGD;
            vpc = vpc + NMBL + NMBL;
            i_state = NEXTOP;
            break;
            // MULS;
        case MULS:
            n = *(vpc + NMBL);
            SHF;
            memcpy(&f->tag, &table_elements[n]->tag, SMBL);
            vpc = vpc + NMBL + NMBL;
            i_state = NEXTOP;
            break;
            // MULE(N);
        case MULE:
            n = *(vpc + NMBL);
            vpc = vpc + NMBL + NMBL;
            f0 = table_elements[n - 1]->prev;
            lack = false;
            while (f0 != table_elements[n])
            {
                f0 = f0->next;
                f = f->next;
                if (f == flhead)
                {
                    i_state = LACK;
                    lack = true;
                    break;
                }
                if (BRA(f0))
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
            if (table_elements[m]->next == table_elements[n])
            {
                i_state = NEXTOP;
                break;
            }
            putts(tsp, &f, &table_elements[n], &table_elements[m]);
            tsp++;
            i_state = NEXTOP;
            break;
            // TPLE(N); (= TPL(N-1,N);)
            // TPLV(N);
        case TPLE:
        case TPLV:
            n = *(vpc + NMBL);
            vpc = vpc + NMBL + NMBL;
            if (table_elements[n]->next == table_elements[n - 1])
            {
                i_state = NEXTOP;
                break;
            }
            putts(tsp, &f, &table_elements[n - 1], &table_elements[n]);
            tsp++;
            i_state = NEXTOP;
            break;
            // TPLS(N); (= TPLM(N,N);)
        case TPLS:
            n = *(vpc + NMBL);
            vpc = vpc + NMBL + NMBL;
            putts(tsp, &f, &table_elements[n], &table_elements[n]);
            tsp++;
            i_state = NEXTOP;
            break;
            // EOS;
        case EOS:
            lastk->info.codep = table_elements[1]->info.codep;
            lastk->tag = TAGK;
            // item adress followed by result
            T_LINKCB *nextr = f->next;
            // execute planned transplantation
            // EOS1:
            while (tsp != (T_TS *)jump_stack)
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
                link(table_elements[1]->prev, table_elements[2]->next);
            else
            {
                link(nextr->prev, table_elements[2]->next);
                link(table_elements[1]->prev, flhead->next);
            };
            //  delete < and >
            // DELKD:
            link(table_elements[2], nextr);
            link(flhead, table_elements[1]);
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
            quasik.info.codep = table_elements[1]->info.codep;
            if (f->next != f)
            {
                link(f->prev, table_elements[2]->next);
                link(table_elements[1]->prev, f->next);
            }
            else
                link(table_elements[1]->prev, table_elements[2]->next);
            if (table_elements[3]->next != table_elements[2])
            {
                link(table_elements[2]->prev, f);
                link(f, table_elements[3]->next);
                link(table_elements[3], table_elements[2]);
            }
            else
                link(f, f);
            link(table_elements[2], flhead->next);
            link(flhead, table_elements[1]);
            i_state = ADVSTEP;
            break;
            // BLF(L);
        case BLF:
            SHF;
            f->info.codep = lastb;
            lastb = f;
            SHF;
            memcpy(&f->info.codef, vpc + NMBL, LBLL);
            f->tag = TAGF;
            vpc = vpc + NMBL + LBLL;
            i_state = NEXTOP;
            break;
            // C-refal-function execution
        case CFUNC:
            memcpy(&fptr, vpc + NMBL + Z_0, LBLL);
            refal.upshot = 1;
            refal.prevr = b0->prev;
            refal.nextr = b0;
            refal.preva = left_board_hole;
            refal.nexta = right_board_hole;
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

static bool spc(T_SPCS *pspcsp, const uint8_t *vpc_, const T_LINKCB *b)
// specifier interpreter
{
    // spcs-pointer
    T_SPCS *spcsp = pspcsp;
    uint8_t *spcvpc; // virtual specifier counter
    memcpy(&spcvpc, vpc_ + NMBL, LBLL);
    uint8_t spcopc;
    // positiveness feature of specifier element
    bool spcpls = true;
    T_SP_STATES sp_state = SPCNXT;
    while (true)
        switch (sp_state)
        {
        // return from specifier element if "YES"
        case SPCRET:
            if (spcsp == pspcsp)
                return spcpls;
            spcsp--;
            // work variable
            const bool spcwrk = spcpls;
            spcpls = spcsp->spls;
            spcvpc = spcsp->svpc;
            if (spcwrk)
                break;
            spcvpc = spcvpc + LBLL;
            sp_state = SPCNXT;
            break;
        // return from specifier element if "NO"
        case SPCNXT:
            // specifier code
            spcopc = *spcvpc;
            spcvpc = spcvpc + NMBL;
            // switch
            // SPCOP
            switch (spcopc)
            {
            case 0000:
                sp_state = SPCCLL;
                break;
            case 0001:
                sp_state = SPCW;
                break;
            case 0002:
                sp_state = SPCNG;
                break;
            case 0003:
                sp_state = SPCNGW;
                break;
            case 0004:
                sp_state = SPCSC;
                break;
            case 0005:
                sp_state = SPCS;
                break;
            case 0006:
                sp_state = SPCB;
                break;
            case 0007:
                sp_state = SPCF;
                break;
            case 0010:
                sp_state = SPCN;
                break;
            case 0011:
                sp_state = SPCR;
                break;
            case 0012:
                sp_state = SPCO;
                break;
            case 0013:
                sp_state = SPCD;
                break;
            case 0014:
                sp_state = SPCL;
            }
            break;
            // SPCCLL(L);
        case SPCCLL:
            spcsp->spls = spcpls;
            spcsp->svpc = spcvpc;
            memcpy(&spcvpc, spcvpc, LBLL);
            spcsp++;
            spcpls = true;
            sp_state = SPCNXT;
            break;
        case SPCW:
            sp_state = SPCRET;
            break;
        case SPCNG:
            spcpls = !spcpls;
            sp_state = SPCNXT;
            break;
        case SPCNGW:
            spcpls = !spcpls;
            sp_state = SPCRET;
            break;
        case SPCSC:
            if (memcmp(spcvpc, &b->tag, SMBL) == 0)
            {
                sp_state = SPCRET;
                break;
            }
            spcvpc = spcvpc + SMBL;
            sp_state = SPCNXT;
            break;
        case SPCS:
            if (NBRA(b))
            {
                sp_state = SPCRET;
                break;
            }
            sp_state = SPCNXT;
            break;
        case SPCB:
            if (BRA(b))
            {
                sp_state = SPCRET;
                break;
            }
            sp_state = SPCNXT;
            break;
        case SPCF:
            if (b->tag == TAGF)
            {
                sp_state = SPCRET;
                break;
            }
            sp_state = SPCNXT;
            break;
        case SPCN:
            if (b->tag == TAGN)
            {
                sp_state = SPCRET;
                break;
            }
            sp_state = SPCNXT;
            break;
        case SPCR:
            if (b->tag == TAGR)
            {
                sp_state = SPCRET;
                break;
            }
            sp_state = SPCNXT;
            break;
        case SPCO:
            if (b->tag == TAGO)
            {
                sp_state = SPCRET;
                break;
            }
            sp_state = SPCNXT;
            break;
        case SPCD:
            if (b->tag != TAGO)
            {
                sp_state = SPCNXT;
                break;
            }
            if (digit(b->info.infoc))
            {
                sp_state = SPCRET;
                break;
            }
            sp_state = SPCNXT;
            break;
        case SPCL:
            if (b->tag != TAGO)
            {
                sp_state = SPCNXT;
                break;
            }
            if (letter(b->info.infoc))
            {
                sp_state = SPCRET;
                break;
            }
            sp_state = SPCNXT;
        }
} //             end      spc

static bool letter(char s)
{
    if ((s >= 'A' && s <= 'Z') || // A..Z
        (s >= 'a' && s <= 'z'))   // a..z
        return true;
    return false;
}

static bool digit(char s)
{
    if (s >= '0' && s <= '9')
        return true;
    return false;
}

static void link(T_LINKCB *x, T_LINKCB *y)
{
    x->next = y;
    y->prev = x;
    return;
}

static void putjs(T_W_JUMP_STACK *jsp_, T_LINKCB **ab1, T_LINKCB **ab2, const uint16_t *anel, uint8_t **avpc)
{
    jsp_->left_board_hole = *ab1;
    jsp_->right_board_hole = *ab2;
    jsp_->number_element = *anel;
    jsp_->vpc = *avpc;
    return;
}

static void getjs(const T_W_JUMP_STACK *jsp_, T_LINKCB **ab1, T_LINKCB **ab2, uint16_t *anel, uint8_t **avpc)
{
    *ab1 = jsp_->left_board_hole;
    *ab2 = jsp_->right_board_hole;
    *anel = jsp_->number_element;
    *avpc = jsp_->vpc;
    return;
}

static void putts(T_TS *tsp_, T_LINKCB **ax, T_LINKCB **ay, T_LINKCB **az)
{
    tsp_->ts0 = *ax;
    tsp_->ts1 = *ay;
    tsp_->ts2 = *az;
    return;
}

static void getts(const T_TS *tsp_, T_LINKCB **ax, T_LINKCB **ay, T_LINKCB **az)
{
    *ax = tsp_->ts0;
    *ay = tsp_->ts1;
    *az = tsp_->ts2;
    return;
}

//------------ end of file RFRUN.C ----------
