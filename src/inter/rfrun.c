// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-04-23
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

#define SHIFT_LEFT_BOARD_HOLE                \
    left_board_hole = left_board_hole->next; \
    if (left_board_hole == right_board_hole) \
    {                                        \
        interpretator_state = FAIL;          \
        break;                               \
    }

#define SHIFT_RIGHT_BOARD_HOLE                     \
    right_board_hole = right_board_hole->previous; \
    if (right_board_hole == left_board_hole)       \
    {                                              \
        interpretator_state = FAIL;                \
        break;                                     \
    }

#define SHIFT_CURRENT_LINKCB_FREE_MEMORY                           \
    current_linkcb_free_memory = current_linkcb_free_memory->next; \
    if (current_linkcb_free_memory == free_memory_list_head)       \
    {                                                              \
        interpretator_state = LACK;                                \
        break;                                                     \
    }

#define LINK(x, y) \
    x->next = y;   \
    y->previous = x

#define PUT_JUMP_STACK(ab1, ab2, anel, avpc)    \
    jump_stack_pointer->left_board_hole = ab1;  \
    jump_stack_pointer->right_board_hole = ab2; \
    jump_stack_pointer->number_element = anel;  \
    jump_stack_pointer->virtual_program_counter = avpc

#define GET_JUMP_STACK(ab1, ab2, anel, avpc)    \
    ab1 = jump_stack_pointer->left_board_hole;  \
    ab2 = jump_stack_pointer->right_board_hole; \
    anel = jump_stack_pointer->number_element;  \
    avpc = jump_stack_pointer->virtual_program_counter

#define PUT_TRANSPLANTATION_STACK(az, ax, ay)                  \
    transplantation_stack_pointer->transplantation_where = az; \
    transplantation_stack_pointer->transplantation_from = ax;  \
    transplantation_stack_pointer->transplantation_to = ay

#define GET_TRANSPLANTATION_STACK(az, ax, ay)                  \
    az = transplantation_stack_pointer->transplantation_where; \
    ax = transplantation_stack_pointer->transplantation_from;  \
    ay = transplantation_stack_pointer->transplantation_to

typedef enum intepretator_states
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
} T_INTERPRETATOR_STATES;

typedef enum specifier_states
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
} T_SPECIFIER_STATES;

typedef struct save_area
{ // save area for var-part of refal-block
    uint8_t upshot;
    T_LINKCB *previous_argument;
    T_LINKCB *next_argument;
    T_LINKCB *previous_result;
    T_LINKCB *next_result;
    T_STATUS_TABLE *current_status_table;
} T_SAVE_AREA;

typedef struct jump_stack
{ // jump stack structure
    T_LINKCB *left_board_hole;
    T_LINKCB *right_board_hole;
    uint16_t number_element;
    uint8_t *virtual_program_counter;
} T_JUMP_STACK;

typedef struct transplantation_stack
{ // transplantation stack structure
    T_LINKCB *transplantation_where;
    T_LINKCB *transplantation_from;
    T_LINKCB *transplantation_to;
} T_TRANSPLANTATION_STACK;

typedef struct specifier_stack
{
    bool stack_plus;
    uint8_t *stack_virtual_private_code;
} T_SPECIFIER_STACK;

uint8_t *inch_ptr;

static T_LINKCB *table_elements[256]; // table of elements
static uint16_t number_element;       // adress of first free string in table of elements

static T_JUMP_STACK jump_stack[128];     // jump stack
static T_JUMP_STACK *jump_stack_pointer; // jump stack pointer

static T_TRANSPLANTATION_STACK transplantation_stack[256];     // transplantation stack
static T_TRANSPLANTATION_STACK *transplantation_stack_pointer; // transplantation stack pointer

static T_SPECIFIER_STACK specifier_stack[64]; // specifier stack

// definition of work variables and pointers
static uint8_t operation_program_code;   // current statement code
static uint8_t *virtual_program_counter; // virtual program counter
static T_LINKCB *last_acted_k;           // last acted '<' adress
static T_LINKCB *last_gen_left_bracket;  // last generated left bracket
static T_LINKCB *temp_board_hole, *left_board_hole, *right_board_hole;
static T_LINKCB *temp_linkcb, *temp_linkcb_free_memory, *current_linkcb_free_memory;
static const uint8_t *virtual_program_counter2; // additional virtual program counter
static uint8_t i, n, m;

static void (*function_c_pointer)(void);

static bool spc(const uint8_t *virtual_program_counter_, const T_LINKCB *b);
static bool letter(char s);
static bool digit(char s);

void refal_run(T_STATUS_TABLE *status_table) // adress of current state table
{
    // dynamic area DSA
    T_SAVE_AREA *save_process = malloc(sizeof(T_SAVE_AREA));
    if (save_process == NULL)
        refal_abort_end("refal_run: no memory");
    if (!exist_status_table(ast))
        refal_abort_end("refal_run: attempt to run unexisting process");
    if (ast->state == 4)
        refal_abort_end("refal_run: attampt to run process in state 4");
    // saving part of refal-block
    save_process->upshot = refal.upshot;
    save_process->previous_argument = refal.previous_argument;
    save_process->next_argument = refal.next_argument;
    save_process->previous_result = refal.previous_result;
    save_process->next_result = refal.next_result;
    save_process->current_status_table = refal.current_status_table;
    refal.current_status_table = ast;
    ast->state = 4;
    T_LINKCB quasik; // quasi-sign '<'
    quasik.info.codep = ast->dot;
    // adress of free memory list head
    T_LINKCB *free_memory_list_head = refal.free_memory_list_head;
    T_INTERPRETATOR_STATES interpretator_state = START;
    while (true)
        switch (interpretator_state)
        {
            // start of next step
        case START:
            if (ast->step >= ast->stop)
            {
                interpretator_state = DONE;
                break;
            }
            right_board_hole = quasik.info.codep;
            if (right_board_hole == NULL)
            {
                interpretator_state = DONE;
                break;
            }
            temp_board_hole = right_board_hole->info.codep;
            left_board_hole = temp_board_hole->next;
            virtual_program_counter = left_board_hole->info.codef;
            if (left_board_hole->tag != TAGF)
            {
                interpretator_state = REF;
                break;
            }
            // here must be check on c-function
            // if (c) goto CFUNC;
            jump_stack_pointer = jump_stack;
            table_elements[1] = temp_board_hole;
            table_elements[2] = right_board_hole;
            table_elements[3] = left_board_hole;
            number_element = 4;
            interpretator_state = NEXTOP;
            break;
            // increase step number
        case ADVSTEP:
            ast->step++;
            interpretator_state = START;
            break;
            // symbol - reference execution
        case REF:
            if (left_board_hole->tag != TAGR)
            {
                interpretator_state = RCGIMP;
                break;
            }
            table_elements[1] = temp_board_hole;
            table_elements[2] = right_board_hole;
            table_elements[3] = left_board_hole;
            current_linkcb_free_memory = (T_LINKCB *)virtual_program_counter;
            interpretator_state = SWAPREF;
            break;
            // interpreter exits
            // store state
        case DONE:
            ast->state = 1; // normal end
            interpretator_state = EXIT;
            break;
        case RCGIMP:
            ast->state = 2; // recognition impossible
            interpretator_state = EXIT;
            break;
        case LACK:
            ast->state = 3; // end of free memory
            quasik.info.codep = table_elements[2];
            interpretator_state = EXIT;
            break;
            // state remove from
        case EXIT:
            ast->dot = quasik.info.codep;
            // restore refal-block
            refal.upshot = save_process->upshot;
            refal.previous_argument = save_process->previous_argument;
            refal.next_argument = save_process->next_argument;
            refal.previous_result = save_process->previous_result;
            refal.next_result = save_process->next_result;
            refal.current_status_table = save_process->current_status_table;
            free(save_process);
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
                interpretator_state = ADVANCE;
                break;
            case 0001:
                interpretator_state = SJUMP;
                break;
            case 0002:
                interpretator_state = FAIL;
                break;
            case 0003:
                interpretator_state = SB;
                break;
            case 0004:
                interpretator_state = LSC;
                break;
            case 0005:
                interpretator_state = RSC;
                break;
            case 0006:
                interpretator_state = LSCO;
                break;
            case 0007:
                interpretator_state = RSCO;
                break;
            case 0010:
                interpretator_state = LSD;
                break;
            case 0011:
                interpretator_state = RSD;
                break;
            case 0012:
                interpretator_state = LTXT;
                break;
            case 0013:
                interpretator_state = RTXT;
                break;
            case 0014:
                interpretator_state = LB;
                break;
            case 0015:
                interpretator_state = LBY;
                break;
            case 0016:
                interpretator_state = RB;
                break;
            case 0017:
                interpretator_state = RBY;
                break;
            case 0020:
                interpretator_state = LS;
                break;
            case 0021:
                interpretator_state = RS;
                break;
            case 0022:
                interpretator_state = LW;
                break;
            case 0023:
                interpretator_state = RW;
                break;
            case 0024:
                interpretator_state = LBNIL;
                break;
            case 0025:
                interpretator_state = RBNIL;
                break;
            case 0026:
                interpretator_state = LBCE;
                break;
            case 0027:
                interpretator_state = RBCE;
                break;
            case 0030:
                interpretator_state = NIL;
                break;
            case 0031:
                interpretator_state = CE;
                break;
            case 0032:
                interpretator_state = LED;
                break;
            case 0033:
                interpretator_state = RED;
                break;
            case 0034:
                interpretator_state = NNIL;
                break;
            case 0035:
                interpretator_state = PLE;
                break;
            case 0036:
                interpretator_state = PLV;
                break;
            case 0037:
                interpretator_state = LE;
                break;
            case 0040:
                interpretator_state = PRE;
                break;
            case 0041:
                interpretator_state = PRV;
                break;
            case 0042:
                interpretator_state = RE;
                break;
            case 0043:
                interpretator_state = PLESC;
                break;
            case 0044:
                interpretator_state = PLVSC;
                break;
            case 0045:
                interpretator_state = LESC;
                break;
            case 0046:
                interpretator_state = PRESC;
                break;
            case 0047:
                interpretator_state = PRVSC;
                break;
            case 0050:
                interpretator_state = RESC;
                break;
            case 0051:
                interpretator_state = LESD;
                break;
            case 0052:
                interpretator_state = RESD;
                break;
            case 0053:
                interpretator_state = PLEB;
                break;
            case 0054:
                interpretator_state = PLVB;
                break;
            case 0055:
                interpretator_state = LEB;
                break;
            case 0056:
                interpretator_state = PREB;
                break;
            case 0057:
                interpretator_state = PRVB;
                break;
            case 0060:
                interpretator_state = REB;
                break;
            case 0061:
                interpretator_state = EOE;
                break;
            case 0062:
                interpretator_state = EOEI;
                break;
            case 0063:
                interpretator_state = LSRCH;
                break;
            case 0064:
                interpretator_state = RSRCH;
                break;
            case 0065:
                interpretator_state = WSPC;
                break;
            case 0066:
                interpretator_state = ESPC;
                break;
            case 0067:
                interpretator_state = PLESPC;
                break;
            case 0070:
                interpretator_state = LESPC;
                break;
            case 0071:
                interpretator_state = PRESPC;
                break;
            case 0072:
                interpretator_state = RESPC;
                break;
            case 0073:
                interpretator_state = LMAX;
                break;
            case 0074:
                interpretator_state = RMAX;
                break;
            case 0075:
                interpretator_state = EOR;
                break;
            case 0076:
                interpretator_state = NS;
                break;
            case 0077:
                interpretator_state = NSO;
                break;
            case 0100:
                interpretator_state = TEXT;
                break;
            case 0101:
                interpretator_state = BL;
                break;
            case 0102:
                interpretator_state = BR;
                break;
            case 0103:
                interpretator_state = BLR;
                break;
            case 0104:
                interpretator_state = BRACT;
                break;
            case 0106:
                interpretator_state = MULS;
                break;
            case 0107:
                interpretator_state = MULE;
                break;
            case 0110:
                interpretator_state = TPL;
                break;
            case 0111:
                interpretator_state = TPLM;
                break;
            case 0112:
                interpretator_state = TPLE;
                break;
            case 0113:
                interpretator_state = TPLV;
                break;
            case 0114:
                interpretator_state = TPLS;
                break;
            case 0115:
                interpretator_state = EOS;
                break;
            case 0116:
                interpretator_state = SWAP;
                break;
            case 0117:
                interpretator_state = BLF;
                break;
            case 0122:
                interpretator_state = CFUNC;
            };
            break;
            // select next statement assembly language
        case ADVANCE:
            virtual_program_counter += NMBL;
            interpretator_state = NEXTOP;
            break;
            // SJUMP(L);
        case SJUMP:
            memcpy(&inch_ptr, virtual_program_counter + NMBL, LBLL);
            PUT_JUMP_STACK(left_board_hole, right_board_hole, number_element, inch_ptr);
            jump_stack_pointer++;
            virtual_program_counter += NMBL + LBLL;
            interpretator_state = NEXTOP;
            break;
            // FAIL;
        case FAIL:
            if (jump_stack_pointer == jump_stack)
            {
                interpretator_state = RCGIMP;
                break;
            }
            jump_stack_pointer--;
            GET_JUMP_STACK(left_board_hole, right_board_hole, number_element, virtual_program_counter);
            interpretator_state = NEXTOP;
            break;
            // SB(N,M);
        case SB:
            n = *(virtual_program_counter + NMBL);
            m = *(virtual_program_counter + NMBL + NMBL);
            left_board_hole = table_elements[n];
            right_board_hole = table_elements[m];
            virtual_program_counter += 3 * NMBL;
            interpretator_state = NEXTOP;
            break;
            // LSC(S);
        case LSC:
            SHIFT_LEFT_BOARD_HOLE;
            if (memcmp(virtual_program_counter + NMBL, &left_board_hole->tag, SMBL) != 0)
            {
                interpretator_state = FAIL;
                break;
            }
            virtual_program_counter += NMBL + SMBL;
            table_elements[number_element] = left_board_hole;
            number_element++;
            interpretator_state = NEXTOP;
            break;
            // RSC(S);
        case RSC:
            SHIFT_RIGHT_BOARD_HOLE;
            if (memcmp(virtual_program_counter + NMBL, &right_board_hole->tag, SMBL) != 0)
            {
                interpretator_state = FAIL;
                break;
            }
            virtual_program_counter += NMBL + SMBL;
            table_elements[number_element] = right_board_hole;
            number_element++;
            interpretator_state = NEXTOP;
            break;
            // LSCO(N);
        case LSCO:
            SHIFT_LEFT_BOARD_HOLE;
            if (left_board_hole->tag != TAGO)
            {
                interpretator_state = FAIL;
                break;
            }
            if (left_board_hole->info.infoc != (char)*(virtual_program_counter + NMBL))
            {
                interpretator_state = FAIL;
                break;
            }
            table_elements[number_element] = left_board_hole;
            number_element++;
            virtual_program_counter += NMBL + NMBL;
            interpretator_state = NEXTOP;
            break;
            // RSCO(N);
        case RSCO:
            SHIFT_RIGHT_BOARD_HOLE;
            if (right_board_hole->tag != TAGO)
            {
                interpretator_state = FAIL;
                break;
            }
            if (right_board_hole->info.infoc != (char)*(virtual_program_counter + NMBL))
            {
                interpretator_state = FAIL;
                break;
            }
            table_elements[number_element] = right_board_hole;
            number_element++;
            virtual_program_counter += NMBL + NMBL;
            interpretator_state = NEXTOP;
            break;
            // LSD(N);
        case LSD:
            SHIFT_LEFT_BOARD_HOLE;
            n = *(virtual_program_counter + NMBL);
            if (left_board_hole->tag != table_elements[n]->tag)
            {
                interpretator_state = FAIL;
                break;
            }
            if (left_board_hole->info.code != table_elements[n]->info.code)
            {
                interpretator_state = FAIL;
                break;
            }
            table_elements[number_element] = left_board_hole;
            number_element++;
            virtual_program_counter += NMBL + NMBL;
            interpretator_state = NEXTOP;
            break;
            // RSD(N);
        case RSD:
            SHIFT_RIGHT_BOARD_HOLE;
            n = *(virtual_program_counter + NMBL);
            if (right_board_hole->tag != table_elements[n]->tag)
            {
                interpretator_state = FAIL;
                break;
            }
            if (right_board_hole->info.code != table_elements[n]->info.code)
            {
                interpretator_state = FAIL;
                break;
            }
            table_elements[number_element] = right_board_hole;
            number_element++;
            virtual_program_counter += NMBL + NMBL;
            interpretator_state = NEXTOP;
            break;
            // LTXT(N,S1,...,SN);
        case LTXT:
            n = *(virtual_program_counter + NMBL);
            virtual_program_counter += NMBL + NMBL;
            interpretator_state = LTXT1;
            break;
        case LTXT1:
            SHIFT_LEFT_BOARD_HOLE;
            if (left_board_hole->tag != TAGO)
            {
                interpretator_state = FAIL;
                break;
            }
            if (left_board_hole->info.infoc != (char)*virtual_program_counter)
            {
                interpretator_state = FAIL;
                break;
            }
            table_elements[number_element] = left_board_hole;
            number_element++;
            virtual_program_counter += NMBL;
            n--;
            if (n != 0)
                break;
            interpretator_state = NEXTOP;
            break;
            // RTXT(N,S1,...,SN);
        case RTXT:
            n = *(virtual_program_counter + NMBL);
            virtual_program_counter += NMBL + NMBL;
            interpretator_state = RTXT1;
            break;
        case RTXT1:
            SHIFT_RIGHT_BOARD_HOLE;
            if (right_board_hole->tag != TAGO)
            {
                interpretator_state = FAIL;
                break;
            }
            if (right_board_hole->info.infoc != (char)*virtual_program_counter)
            {
                interpretator_state = FAIL;
                break;
            }
            table_elements[number_element] = right_board_hole;
            number_element++;
            virtual_program_counter += NMBL;
            n--;
            if (n != 0)
                break;
            interpretator_state = NEXTOP;
            break;
            // LB;
        case LB:
            SHIFT_LEFT_BOARD_HOLE;
            if (NOT_BRACKET(left_board_hole))
            {
                interpretator_state = FAIL;
                break;
            }
            right_board_hole = left_board_hole->info.codep;
            table_elements[number_element] = left_board_hole;
            table_elements[number_element + 1] = right_board_hole;
            number_element += 2;
            interpretator_state = ADVANCE;
            break;
            // LBY;
        case LBY:
            SHIFT_LEFT_BOARD_HOLE;
            if (NOT_BRACKET(left_board_hole))
            {
                interpretator_state = FAIL;
                break;
            }
            table_elements[number_element] = left_board_hole;
            left_board_hole = left_board_hole->info.codep;
            table_elements[number_element + 1] = left_board_hole;
            number_element += 2;
            interpretator_state = ADVANCE;
            break;
            // RB;
        case RB:
            SHIFT_RIGHT_BOARD_HOLE;
            if (NOT_BRACKET(right_board_hole))
            {
                interpretator_state = FAIL;
                break;
            }
            left_board_hole = right_board_hole->info.codep;
            table_elements[number_element] = left_board_hole;
            table_elements[number_element + 1] = right_board_hole;
            number_element += 2;
            interpretator_state = ADVANCE;
            break;
            // RBY;
        case RBY:
            SHIFT_RIGHT_BOARD_HOLE;
            if (NOT_BRACKET(right_board_hole))
            {
                interpretator_state = FAIL;
                break;
            }
            table_elements[number_element + 1] = right_board_hole;
            right_board_hole = right_board_hole->info.codep;
            table_elements[number_element] = right_board_hole;
            number_element += 2;
            interpretator_state = ADVANCE;
            break;
            // LS;
        case LS:
            SHIFT_LEFT_BOARD_HOLE;
            if (BRACKET(left_board_hole))
            {
                interpretator_state = FAIL;
                break;
            }
            table_elements[number_element] = left_board_hole;
            number_element++;
            interpretator_state = ADVANCE;
            break;
            // RS;
        case RS:
            SHIFT_RIGHT_BOARD_HOLE;
            if (BRACKET(right_board_hole))
            {
                interpretator_state = FAIL;
                break;
            }
            table_elements[number_element] = right_board_hole;
            number_element++;
            interpretator_state = ADVANCE;
            break;
            // LW;
        case LW:
            SHIFT_LEFT_BOARD_HOLE;
            table_elements[number_element] = left_board_hole;
            if (BRACKET(left_board_hole))
                left_board_hole = left_board_hole->info.codep;
            table_elements[number_element + 1] = left_board_hole;
            number_element += 2;
            interpretator_state = ADVANCE;
            break;
            // RW;
        case RW:
            SHIFT_RIGHT_BOARD_HOLE;
            table_elements[number_element + 1] = right_board_hole;
            if (BRACKET(right_board_hole))
                right_board_hole = right_board_hole->info.codep;
            table_elements[number_element] = right_board_hole;
            number_element += 2;
            interpretator_state = ADVANCE;
            break;
            // LBNIL
        case LBNIL:
            SHIFT_LEFT_BOARD_HOLE;
            if (NOT_BRACKET(left_board_hole))
            {
                interpretator_state = FAIL;
                break;
            }
            temp_board_hole = left_board_hole;
            left_board_hole = left_board_hole->info.codep;
            if (temp_board_hole->next != left_board_hole)
            {
                interpretator_state = FAIL;
                break;
            }
            table_elements[number_element] = temp_board_hole;
            table_elements[number_element + 1] = left_board_hole;
            number_element += 2;
            interpretator_state = ADVANCE;
            break;
            // RBNIL;
        case RBNIL:
            SHIFT_RIGHT_BOARD_HOLE;
            if (NOT_BRACKET(right_board_hole))
            {
                interpretator_state = FAIL;
                break;
            }
            temp_board_hole = right_board_hole;
            right_board_hole = right_board_hole->info.codep;
            if (right_board_hole->next != temp_board_hole)
            {
                interpretator_state = FAIL;
                break;
            }
            table_elements[number_element] = right_board_hole;
            table_elements[number_element + 1] = temp_board_hole;
            number_element += 2;
            interpretator_state = ADVANCE;
            break;
            // LBCE;
        case LBCE:
            SHIFT_LEFT_BOARD_HOLE;
            if (NOT_BRACKET(left_board_hole))
            {
                interpretator_state = FAIL;
                break;
            }
            temp_board_hole = left_board_hole;
            left_board_hole = left_board_hole->info.codep;
            table_elements[number_element] = temp_board_hole;
            table_elements[number_element + 1] = left_board_hole;
            table_elements[number_element + 2] = temp_board_hole->next;
            table_elements[number_element + 3] = left_board_hole->previous;
            number_element += 4;
            interpretator_state = ADVANCE;
            break;
            // RBCE;
        case RBCE:
            SHIFT_RIGHT_BOARD_HOLE;
            if (NOT_BRACKET(right_board_hole))
            {
                interpretator_state = FAIL;
                break;
            }
            temp_board_hole = right_board_hole;
            right_board_hole = right_board_hole->info.codep;
            table_elements[number_element] = right_board_hole;
            table_elements[number_element + 1] = temp_board_hole;
            table_elements[number_element + 2] = right_board_hole->next;
            table_elements[number_element + 3] = temp_board_hole->previous;
            number_element += 4;
            interpretator_state = ADVANCE;
            break;
            // NIL;
        case NIL:
            if (left_board_hole->next != right_board_hole)
            {
                interpretator_state = FAIL;
                break;
            }
            interpretator_state = ADVANCE;
            break;
            // CE;
        case CE:
            table_elements[number_element] = left_board_hole->next;
            table_elements[number_element + 1] = right_board_hole->previous;
            number_element += 2;
            interpretator_state = ADVANCE;
            break;
            // LED(N);
        case LED:
            n = *(virtual_program_counter + NMBL);
            table_elements[number_element] = left_board_hole->next;
            temp_board_hole = table_elements[n - 1]->previous;
            interpretator_state = LED1;
            break;
        case LED1:
            if (temp_board_hole == table_elements[n])
            {
                interpretator_state = LED2;
                break;
            }
            temp_board_hole = temp_board_hole->next;
            SHIFT_LEFT_BOARD_HOLE;
            if (left_board_hole->tag != temp_board_hole->tag)
            {
                interpretator_state = FAIL;
                break;
            }
            if (left_board_hole->info.code == temp_board_hole->info.code)
                break;
            if (BRACKET(left_board_hole))
                break;
            interpretator_state = FAIL;
            break;
        case LED2:
            table_elements[number_element + 1] = left_board_hole;
            number_element += 2;
            virtual_program_counter += NMBL * 2;
            interpretator_state = NEXTOP;
            break;
            // RED(N);
        case RED:
            n = *(virtual_program_counter + NMBL);
            table_elements[number_element + 1] = right_board_hole->previous;
            temp_board_hole = table_elements[n]->next;
            interpretator_state = RED1;
            break;
        case RED1:
            if (temp_board_hole == table_elements[n - 1])
            {
                interpretator_state = RED2;
                break;
            }
            temp_board_hole = temp_board_hole->previous;
            SHIFT_RIGHT_BOARD_HOLE;
            if (right_board_hole->tag != temp_board_hole->tag)
            {
                interpretator_state = FAIL;
                break;
            }
            if (right_board_hole->info.code == temp_board_hole->info.code)
                break;
            if (BRACKET(right_board_hole))
                break;
            interpretator_state = FAIL;
            break;
        case RED2:
            table_elements[number_element] = right_board_hole;
            number_element += 2;
            virtual_program_counter += NMBL * 2;
            interpretator_state = NEXTOP;
            break;
            // NNIL;
        case NNIL:
            if (table_elements[number_element - 1]->next == table_elements[number_element - 2])
            {
                interpretator_state = FAIL;
                break;
            }
            interpretator_state = ADVANCE;
            break;
            // PLE;
        case PLE:
            virtual_program_counter += NMBL;
            PUT_JUMP_STACK(left_board_hole, right_board_hole, number_element, virtual_program_counter);
            jump_stack_pointer++;
            table_elements[number_element] = left_board_hole->next;
            table_elements[number_element + 1] = left_board_hole;
            number_element += 2;
            interpretator_state = ADVANCE;
            break;
            // PLV;
        case PLV:
            virtual_program_counter += NMBL;
            PUT_JUMP_STACK(left_board_hole, right_board_hole, number_element, virtual_program_counter);
            table_elements[number_element] = left_board_hole->next;
            table_elements[number_element + 1] = left_board_hole;
            interpretator_state = NEXTOP;
            break;
            // LE;
        case LE:
            left_board_hole = table_elements[number_element + 1];
            SHIFT_LEFT_BOARD_HOLE;
            if (BRACKET(left_board_hole))
                left_board_hole = left_board_hole->info.codep;
            jump_stack_pointer++;
            table_elements[number_element + 1] = left_board_hole;
            number_element += 2;
            interpretator_state = ADVANCE;
            break;
            // PRE;
        case PRE:
            virtual_program_counter += NMBL;
            PUT_JUMP_STACK(left_board_hole, right_board_hole, number_element, virtual_program_counter);
            jump_stack_pointer++;
            table_elements[number_element] = right_board_hole;
            table_elements[number_element + 1] = right_board_hole->previous;
            number_element += 2;
            interpretator_state = ADVANCE;
            break;
            // PRV;
        case PRV:
            virtual_program_counter += NMBL;
            PUT_JUMP_STACK(left_board_hole, right_board_hole, number_element, virtual_program_counter);
            table_elements[number_element] = right_board_hole;
            table_elements[number_element + 1] = right_board_hole->previous;
            interpretator_state = NEXTOP;
            break;
            // RE;
        case RE:
            right_board_hole = table_elements[number_element];
            SHIFT_RIGHT_BOARD_HOLE;
            if (BRACKET(right_board_hole))
                right_board_hole = right_board_hole->info.codep;
            jump_stack_pointer++;
            table_elements[number_element] = right_board_hole;
            number_element += 2;
            interpretator_state = ADVANCE;
            break;
            // PLESC;
        case PLESC:
            virtual_program_counter += NMBL;
            PUT_JUMP_STACK(left_board_hole, right_board_hole, number_element, virtual_program_counter);
            table_elements[number_element] = left_board_hole->next;
            table_elements[number_element + 2] = left_board_hole;
            interpretator_state = NEXTOP;
            break;
            // PLVSC;
        case PLVSC:
            virtual_program_counter += NMBL;
            PUT_JUMP_STACK(left_board_hole, right_board_hole, number_element, virtual_program_counter);
            table_elements[number_element] = left_board_hole->next;
            SHIFT_LEFT_BOARD_HOLE;
            if (BRACKET(left_board_hole))
                left_board_hole = left_board_hole->info.codep;
            table_elements[number_element + 2] = left_board_hole;
            interpretator_state = NEXTOP;
            break;
            // LESC(S);
        case LESC:
            virtual_program_counter += NMBL + SMBL;
            virtual_program_counter2 = virtual_program_counter - SMBL;
            interpretator_state = LESC0;
            break;
        case LESC0:
            left_board_hole = table_elements[number_element + 2];
            interpretator_state = LESC1;
            break;
        case LESC1:
            SHIFT_LEFT_BOARD_HOLE;
            if (BRACKET(left_board_hole))
            {
                left_board_hole = left_board_hole->info.codep;
                break;
            }
            if (memcmp(virtual_program_counter2, &left_board_hole->tag, SMBL) != 0)
                break;
            jump_stack_pointer++;
            table_elements[number_element + 1] = left_board_hole->previous;
            table_elements[number_element + 2] = left_board_hole;
            number_element += 3;
            interpretator_state = NEXTOP;
            break;
            // PRESC;
        case PRESC:
            virtual_program_counter += NMBL;
            PUT_JUMP_STACK(left_board_hole, right_board_hole, number_element, virtual_program_counter);
            table_elements[number_element + 1] = right_board_hole->previous;
            table_elements[number_element + 2] = right_board_hole;
            interpretator_state = NEXTOP;
            break;
            // PRVSC;
        case PRVSC:
            virtual_program_counter += NMBL;
            PUT_JUMP_STACK(left_board_hole, right_board_hole, number_element, virtual_program_counter);
            table_elements[number_element + 1] = right_board_hole->previous;
            SHIFT_RIGHT_BOARD_HOLE;
            if (BRACKET(right_board_hole))
                right_board_hole = right_board_hole->info.codep;
            table_elements[number_element + 2] = right_board_hole;
            interpretator_state = NEXTOP;
            break;
            // RESC(S);
        case RESC:
            virtual_program_counter += NMBL + SMBL;
            virtual_program_counter2 = virtual_program_counter - SMBL;
            interpretator_state = RESC0;
            break;
        case RESC0:
            right_board_hole = table_elements[number_element + 2];
            interpretator_state = RESC1;
            break;
        case RESC1:
            SHIFT_RIGHT_BOARD_HOLE;
            if (BRACKET(right_board_hole))
            {
                right_board_hole = right_board_hole->info.codep;
                break;
            }
            if (memcmp(virtual_program_counter2, &right_board_hole->tag, SMBL) != 0)
                break;
            jump_stack_pointer++;
            table_elements[number_element + 2] = right_board_hole;
            table_elements[number_element] = right_board_hole->next;
            number_element += 3;
            interpretator_state = NEXTOP;
            break;
            // LESD(N);
        case LESD:
            n = *(virtual_program_counter + NMBL);
            virtual_program_counter2 = (uint8_t *)&table_elements[n]->tag;
            virtual_program_counter += NMBL * 2;
            interpretator_state = LESC0;
            break;
            // RESD(N);
        case RESD:
            n = *(virtual_program_counter + NMBL);
            virtual_program_counter2 = (uint8_t *)&table_elements[n]->tag;
            virtual_program_counter += NMBL * 2;
            interpretator_state = RESC0;
            break;
            // PLEB;
        case PLEB:
            virtual_program_counter += NMBL;
            PUT_JUMP_STACK(left_board_hole, right_board_hole, number_element, virtual_program_counter);
            table_elements[number_element] = left_board_hole->next;
            table_elements[number_element + 3] = left_board_hole;
            interpretator_state = NEXTOP;
            break;
            // PLVB;
        case PLVB:
            virtual_program_counter += NMBL;
            PUT_JUMP_STACK(left_board_hole, right_board_hole, number_element, virtual_program_counter);
            table_elements[number_element] = left_board_hole->next;
            SHIFT_LEFT_BOARD_HOLE;
            if (BRACKET(left_board_hole))
                left_board_hole = left_board_hole->info.codep;
            table_elements[number_element + 3] = left_board_hole;
            interpretator_state = NEXTOP;
            break;
            // LEB;
        case LEB:
            left_board_hole = table_elements[number_element + 3];
            interpretator_state = LEB1;
            break;
        case LEB1:
            SHIFT_LEFT_BOARD_HOLE;
            if (NOT_BRACKET(left_board_hole))
                break;
            jump_stack_pointer++;
            table_elements[number_element + 1] = left_board_hole->previous;
            table_elements[number_element + 2] = left_board_hole;
            right_board_hole = left_board_hole->info.codep;
            table_elements[number_element + 3] = right_board_hole;
            number_element += 4;
            interpretator_state = ADVANCE;
            break;
            // PREB;
        case PREB:
            virtual_program_counter += NMBL;
            PUT_JUMP_STACK(left_board_hole, right_board_hole, number_element, virtual_program_counter);
            table_elements[number_element + 1] = right_board_hole->previous;
            table_elements[number_element + 2] = right_board_hole;
            interpretator_state = NEXTOP;
            break;
            // PRVB;
        case PRVB:
            virtual_program_counter += NMBL;
            PUT_JUMP_STACK(left_board_hole, right_board_hole, number_element, virtual_program_counter);
            table_elements[number_element + 1] = right_board_hole->previous;
            SHIFT_RIGHT_BOARD_HOLE;
            if (BRACKET(right_board_hole))
                right_board_hole = right_board_hole->info.codep;
            table_elements[number_element + 2] = right_board_hole;
            interpretator_state = NEXTOP;
            break;
            //  REB;
        case REB:
            right_board_hole = table_elements[number_element + 2];
            interpretator_state = REB1;
            break;
        case REB1:
            SHIFT_RIGHT_BOARD_HOLE;
            if (NOT_BRACKET(right_board_hole))
                break;
            jump_stack_pointer++;
            table_elements[number_element] = right_board_hole->next;
            table_elements[number_element + 3] = right_board_hole;
            left_board_hole = right_board_hole->info.codep;
            table_elements[number_element + 2] = left_board_hole;
            number_element += 4;
            interpretator_state = ADVANCE;
            break;
            // EOE(N);
        case EOE:
            n = *(virtual_program_counter + NMBL);
            jump_stack_pointer -= n;
            virtual_program_counter += 2 * NMBL;
            interpretator_state = NEXTOP;
            break;
            // EOEI;
        case EOEI:
            jump_stack_pointer--;
            interpretator_state = ADVANCE;
            break;
            // LSRCH(S);
        case LSRCH:
            table_elements[number_element] = left_board_hole->next;
            interpretator_state = LSRCH1;
            break;
        case LSRCH1:
            SHIFT_LEFT_BOARD_HOLE;
            if (BRACKET(left_board_hole))
            {
                left_board_hole = left_board_hole->info.codep;
                break;
            };
            if (memcmp(virtual_program_counter + NMBL, &left_board_hole->tag, SMBL) != 0)
                break;
            table_elements[number_element + 1] = left_board_hole->previous;
            table_elements[number_element + 2] = left_board_hole;
            number_element += 3;
            virtual_program_counter += NMBL + SMBL;
            interpretator_state = NEXTOP;
            break;
            // RSRCH(S);
        case RSRCH:
            table_elements[number_element + 1] = right_board_hole->previous;
            interpretator_state = RSRCH1;
            break;
        case RSRCH1:
            SHIFT_RIGHT_BOARD_HOLE;
            if (BRACKET(right_board_hole))
            {
                right_board_hole = right_board_hole->info.codep;
                break;
            };
            if (memcmp(virtual_program_counter + NMBL, &right_board_hole->tag, SMBL) != 0)
                break;
            table_elements[number_element] = right_board_hole->next;
            table_elements[number_element + 2] = right_board_hole;
            number_element += 3;
            virtual_program_counter += NMBL + SMBL;
            interpretator_state = NEXTOP;
            break;
            // WSPC(L);
        case WSPC:
            if (!spc(virtual_program_counter, table_elements[number_element - 1]))
            {
                interpretator_state = FAIL;
                break;
            }
            virtual_program_counter += NMBL + LBLL;
            interpretator_state = NEXTOP;
            break;
            // ESPC(L);
        case ESPC:
            temp_board_hole = table_elements[number_element - 2]->previous;
            bool fail = false;
            while (temp_board_hole != table_elements[number_element - 1])
            {
                temp_board_hole = temp_board_hole->next;
                if (BRACKET(temp_board_hole))
                    temp_board_hole = temp_board_hole->info.codep;
                if (!spc(virtual_program_counter, temp_board_hole))
                {
                    interpretator_state = FAIL;
                    fail = true;
                    break;
                }
            };
            if (fail)
                break;
            virtual_program_counter += NMBL + LBLL;
            interpretator_state = NEXTOP;
            break;
            // PLESPC;
        case PLESPC:
            virtual_program_counter += NMBL;
            PUT_JUMP_STACK(left_board_hole, right_board_hole, number_element, virtual_program_counter);
            jump_stack_pointer++;
            table_elements[number_element] = left_board_hole->next;
            table_elements[number_element + 1] = left_board_hole;
            number_element += 2;
            virtual_program_counter += NMBL + LBLL;
            interpretator_state = NEXTOP;
            break;
            // LESPC(L);
        case LESPC:
            left_board_hole = table_elements[number_element + 1];
            SHIFT_LEFT_BOARD_HOLE;
            if (BRACKET(left_board_hole))
                left_board_hole = left_board_hole->info.codep;
            if (!spc(virtual_program_counter, left_board_hole))
            {
                interpretator_state = FAIL;
                break;
            }
            jump_stack_pointer++;
            table_elements[number_element + 1] = left_board_hole;
            number_element += 2;
            virtual_program_counter += NMBL + LBLL;
            interpretator_state = NEXTOP;
            break;
            // PRESPC;
        case PRESPC:
            virtual_program_counter += NMBL;
            PUT_JUMP_STACK(left_board_hole, right_board_hole, number_element, virtual_program_counter);
            jump_stack_pointer++;
            table_elements[number_element + 1] = right_board_hole->previous;
            table_elements[number_element] = right_board_hole;
            number_element += 2;
            virtual_program_counter += NMBL + LBLL;
            interpretator_state = NEXTOP;
            break;
            // RESPC(L);
        case RESPC:
            right_board_hole = table_elements[number_element];
            SHIFT_RIGHT_BOARD_HOLE;
            if (BRACKET(right_board_hole))
                right_board_hole = right_board_hole->info.codep;
            if (!spc(virtual_program_counter, right_board_hole))
            {
                interpretator_state = FAIL;
                break;
            }
            jump_stack_pointer++;
            table_elements[number_element] = right_board_hole;
            number_element += 2;
            virtual_program_counter += NMBL + LBLL;
            interpretator_state = NEXTOP;
            break;
            // LMAX(L);
        case LMAX:
            table_elements[number_element] = left_board_hole->next;
            left_board_hole = left_board_hole->next;
            while (left_board_hole != right_board_hole)
            {
                if (!spc(virtual_program_counter, left_board_hole))
                    break;
                if (BRACKET(left_board_hole))
                    left_board_hole = left_board_hole->info.codep;
                left_board_hole = left_board_hole->next;
            };
            left_board_hole = left_board_hole->previous;
            table_elements[number_element + 1] = left_board_hole;
            number_element += 2;
            virtual_program_counter += NMBL + LBLL;
            interpretator_state = NEXTOP;
            break;
            // RMAX(L);
        case RMAX:
            table_elements[number_element + 1] = right_board_hole->previous;
            right_board_hole = right_board_hole->previous;
            while (right_board_hole != left_board_hole)
            {
                if (!spc(virtual_program_counter, right_board_hole))
                    break;
                if (BRACKET(right_board_hole))
                    right_board_hole = right_board_hole->info.codep;
                right_board_hole = right_board_hole->previous;
            };
            right_board_hole = right_board_hole->next;
            table_elements[number_element] = right_board_hole;
            number_element += 2;
            virtual_program_counter += NMBL + LBLL;
            interpretator_state = NEXTOP;
            break;
            // EOR;
        case EOR:
            current_linkcb_free_memory = free_memory_list_head;
            last_acted_k = &quasik;
            last_gen_left_bracket = NULL;
            transplantation_stack_pointer = transplantation_stack;
            interpretator_state = ADVANCE;
            break;
            // NS(S);
        case NS:
            SHIFT_CURRENT_LINKCB_FREE_MEMORY;
            memcpy(&current_linkcb_free_memory->tag, virtual_program_counter + NMBL, SMBL);
            virtual_program_counter += NMBL + SMBL;
            interpretator_state = NEXTOP;
            break;
            // NSO(N);
        case NSO:
            SHIFT_CURRENT_LINKCB_FREE_MEMORY;
            current_linkcb_free_memory->tag = TAGO;
            current_linkcb_free_memory->info.code = NULL;
            current_linkcb_free_memory->info.infoc = (char)*(virtual_program_counter + NMBL);
            virtual_program_counter += NMBL + NMBL;
            interpretator_state = NEXTOP;
            break;
            // TEXT(N,S1,S2,...,SN);
        case TEXT:
            n = *(virtual_program_counter + NMBL);
            virtual_program_counter += NMBL + NMBL;
            bool lack = false;
            for (i = 1; i <= n; i++)
            {
                current_linkcb_free_memory = current_linkcb_free_memory->next;
                if (current_linkcb_free_memory == free_memory_list_head)
                {
                    interpretator_state = LACK;
                    lack = true;
                    break;
                }
                current_linkcb_free_memory->tag = TAGO;
                current_linkcb_free_memory->info.code = NULL;
                current_linkcb_free_memory->info.infoc = (char)*virtual_program_counter;
                virtual_program_counter += NMBL;
            };
            if (lack)
                break;
            interpretator_state = NEXTOP;
            break;
            // BL;
        case BL:
            SHIFT_CURRENT_LINKCB_FREE_MEMORY;
            current_linkcb_free_memory->info.codep = last_gen_left_bracket;
            last_gen_left_bracket = current_linkcb_free_memory;
            interpretator_state = ADVANCE;
            break;
            // BR;
        case BR:
            SHIFT_CURRENT_LINKCB_FREE_MEMORY;
            current_linkcb_free_memory->info.codep = last_gen_left_bracket;
            temp_linkcb_free_memory = last_gen_left_bracket->info.codep;
            last_gen_left_bracket->tag = TAGLB;
            last_gen_left_bracket->info.codep = current_linkcb_free_memory;
            current_linkcb_free_memory->tag = TAGRB;
            last_gen_left_bracket = temp_linkcb_free_memory;
            interpretator_state = ADVANCE;
            break;
            // BLR;
        case BLR:
            SHIFT_CURRENT_LINKCB_FREE_MEMORY;
            temp_linkcb_free_memory = current_linkcb_free_memory;
            SHIFT_CURRENT_LINKCB_FREE_MEMORY;
            temp_linkcb_free_memory->info.codep = current_linkcb_free_memory;
            current_linkcb_free_memory->info.codep = temp_linkcb_free_memory;
            temp_linkcb_free_memory->tag = TAGLB;
            current_linkcb_free_memory->tag = TAGRB;
            interpretator_state = ADVANCE;
            break;
            // BRACT;
        case BRACT:
            SHIFT_CURRENT_LINKCB_FREE_MEMORY;
            current_linkcb_free_memory->info.codep = last_gen_left_bracket;
            current_linkcb_free_memory->tag = TAGD;
            last_acted_k->info.codep = current_linkcb_free_memory;
            last_acted_k->tag = TAGK;
            last_acted_k = last_gen_left_bracket;
            last_gen_left_bracket = last_gen_left_bracket->info.codep;
            interpretator_state = ADVANCE;
            break;
            // MULS;
        case MULS:
            n = *(virtual_program_counter + NMBL);
            SHIFT_CURRENT_LINKCB_FREE_MEMORY;
            memcpy(&current_linkcb_free_memory->tag, &table_elements[n]->tag, SMBL);
            virtual_program_counter += NMBL + NMBL;
            interpretator_state = NEXTOP;
            break;
            // MULE(N);
        case MULE:
            n = *(virtual_program_counter + NMBL);
            virtual_program_counter += NMBL + NMBL;
            temp_linkcb = table_elements[n - 1]->previous;
            lack = false;
            while (temp_linkcb != table_elements[n])
            {
                temp_linkcb = temp_linkcb->next;
                current_linkcb_free_memory = current_linkcb_free_memory->next;
                if (current_linkcb_free_memory == free_memory_list_head)
                {
                    interpretator_state = LACK;
                    lack = true;
                    break;
                }
                if (BRACKET(temp_linkcb))
                {
                    if (temp_linkcb->tag != TAGRB)
                    {
                        current_linkcb_free_memory->info.codep = last_gen_left_bracket;
                        last_gen_left_bracket = current_linkcb_free_memory;
                    }
                    else
                    {
                        current_linkcb_free_memory->info.codep = last_gen_left_bracket;
                        current_linkcb_free_memory->tag = TAGRB;
                        temp_linkcb_free_memory = last_gen_left_bracket->info.codep;
                        last_gen_left_bracket->info.codep = current_linkcb_free_memory;
                        last_gen_left_bracket->tag = TAGLB;
                        last_gen_left_bracket = temp_linkcb_free_memory;
                    };
                }
                else
                    memcpy(&current_linkcb_free_memory->tag, &temp_linkcb->tag, SMBL);
            };
            if (lack)
                break;
            interpretator_state = NEXTOP;
            break;
            // TPL(N,M);
        case TPL:
            n = *(virtual_program_counter + NMBL);
            m = *(virtual_program_counter + NMBL + NMBL);
            virtual_program_counter += NMBL * 3;
            if (table_elements[m]->next == table_elements[n])
            {
                interpretator_state = NEXTOP;
                break;
            }
            PUT_TRANSPLANTATION_STACK(current_linkcb_free_memory, table_elements[n], table_elements[m]);
            transplantation_stack_pointer++;
            interpretator_state = NEXTOP;
            break;
            // TPLM(N,M);
        case TPLM:
            n = *(virtual_program_counter + NMBL);
            m = *(virtual_program_counter + NMBL + NMBL);
            virtual_program_counter += NMBL * 3;
            PUT_TRANSPLANTATION_STACK(current_linkcb_free_memory, table_elements[n], table_elements[m]);
            transplantation_stack_pointer++;
            interpretator_state = NEXTOP;
            break;
            // TPLE(N); (= TPL(N-1,N);)
        case TPLE:
            n = *(virtual_program_counter + NMBL);
            virtual_program_counter += NMBL + NMBL;
            if (table_elements[n]->next == table_elements[n - 1])
            {
                interpretator_state = NEXTOP;
                break;
            }
            PUT_TRANSPLANTATION_STACK(current_linkcb_free_memory, table_elements[n - 1], table_elements[n]);
            transplantation_stack_pointer++;
            interpretator_state = NEXTOP;
            break;
            // TPLV(N); (= TPLM(N-1,N);)
        case TPLV:
            n = *(virtual_program_counter + NMBL);
            virtual_program_counter += NMBL + NMBL;
            PUT_TRANSPLANTATION_STACK(current_linkcb_free_memory, table_elements[n - 1], table_elements[n]);
            transplantation_stack_pointer++;
            interpretator_state = NEXTOP;
            break;
            // TPLS(N); (= TPLM(N,N);)
        case TPLS:
            n = *(virtual_program_counter + NMBL);
            virtual_program_counter += NMBL + NMBL;
            PUT_TRANSPLANTATION_STACK(current_linkcb_free_memory, table_elements[n], table_elements[n]);
            transplantation_stack_pointer++;
            interpretator_state = NEXTOP;
            break;
            // EOS;
        case EOS:
            last_acted_k->info.codep = table_elements[1]->info.codep;
            last_acted_k->tag = TAGK;
            // item adress followed by result
            T_LINKCB *next_result = current_linkcb_free_memory->next;
            // execute planned transplantation
            // EOS1:
            while (transplantation_stack_pointer != transplantation_stack)
            {
                transplantation_stack_pointer--;
                GET_TRANSPLANTATION_STACK(current_linkcb_free_memory, temp_linkcb, temp_linkcb_free_memory);
                LINK(temp_linkcb->previous, temp_linkcb_free_memory->next);
                LINK(temp_linkcb_free_memory, current_linkcb_free_memory->next);
                LINK(current_linkcb_free_memory, temp_linkcb);
            }
            // include replace result
            // INSRES:
            if (free_memory_list_head->next == next_result)
            {
                LINK(table_elements[1]->previous, table_elements[2]->next);
            }
            else
            {
                LINK(next_result->previous, table_elements[2]->next);
                LINK(table_elements[1]->previous, free_memory_list_head->next);
            };
            //  delete < and >
            // DELKD:
            LINK(table_elements[2], next_result);
            LINK(free_memory_list_head, table_elements[1]);
            interpretator_state = ADVSTEP;
            break;
            // SWAP;
            //  static box head is after operator code
        case SWAP:
            virtual_program_counter += NMBL;
            current_linkcb_free_memory = (T_LINKCB *)virtual_program_counter;
            if (current_linkcb_free_memory->previous != NULL)
            {
                interpretator_state = SWAPREF;
                break;
            }
            LINK(current_linkcb_free_memory, current_linkcb_free_memory);
            current_linkcb_free_memory->info.codep = refal.static_boxes;
            current_linkcb_free_memory->tag = TAGO;
            refal.static_boxes = current_linkcb_free_memory;
            interpretator_state = SWAPREF;
            break;
        case SWAPREF:
            quasik.info.codep = table_elements[1]->info.codep;
            if (current_linkcb_free_memory->next != current_linkcb_free_memory)
            {
                LINK(current_linkcb_free_memory->previous, table_elements[2]->next);
                LINK(table_elements[1]->previous, current_linkcb_free_memory->next);
            }
            else
            {
                LINK(table_elements[1]->previous, table_elements[2]->next);
            }
            if (table_elements[3]->next != table_elements[2])
            {
                LINK(table_elements[2]->previous, current_linkcb_free_memory);
                LINK(current_linkcb_free_memory, table_elements[3]->next);
                LINK(table_elements[3], table_elements[2]);
            }
            else
            {
                LINK(current_linkcb_free_memory, current_linkcb_free_memory);
            }
            LINK(table_elements[2], free_memory_list_head->next);
            LINK(free_memory_list_head, table_elements[1]);
            interpretator_state = ADVSTEP;
            break;
            // BLF(L);
        case BLF:
            SHIFT_CURRENT_LINKCB_FREE_MEMORY;
            current_linkcb_free_memory->info.codep = last_gen_left_bracket;
            last_gen_left_bracket = current_linkcb_free_memory;
            SHIFT_CURRENT_LINKCB_FREE_MEMORY;
            memcpy(&current_linkcb_free_memory->info.codef, virtual_program_counter + NMBL, LBLL);
            current_linkcb_free_memory->tag = TAGF;
            virtual_program_counter += NMBL + LBLL;
            interpretator_state = NEXTOP;
            break;
            // C-refal-function execution
        case CFUNC:
            memcpy(&function_c_pointer, virtual_program_counter + NMBL + Z_0, LBLL);
            refal.upshot = 1;
            refal.previous_result = temp_board_hole->previous;
            refal.next_result = temp_board_hole;
            refal.previous_argument = left_board_hole;
            refal.next_argument = right_board_hole;
            //        call  C - function
            (*function_c_pointer)();
            switch (refal.upshot)
            {
            case 1:
                interpretator_state = CFDONE;
                break;
            case 2:
                interpretator_state = RCGIMP;
                break;
            case 3:
                interpretator_state = CFLACK;
                break;
            default:
                interpretator_state = CFDONE;
            }
            break;
            //        return from C - function
            //          step is done
        case CFDONE:
            quasik.info.codep = refal.next_result->info.codep;
            LINK(refal.next_result->previous, refal.next_argument->next);
            LINK(refal.next_argument, free_memory_list_head->next);
            LINK(free_memory_list_head, refal.next_result);
            interpretator_state = ADVSTEP;
            break;
            //        return from C - function
            //     free memory exhausted
        case CFLACK:
            if (refal.previous_result->next != refal.next_result)
            {
                LINK(refal.next_result->previous, free_memory_list_head->next);
                LINK(free_memory_list_head, refal.previous_result->next);
                LINK(refal.previous_result, refal.next_result);
            }
            ast->state = 3;
            interpretator_state = EXIT;
        }
}

static bool spc(const uint8_t *virtual_program_counter_, const T_LINKCB *b)
// specifier interpreter
{
    // spcs-pointer
    T_SPECIFIER_STACK *specifier_stack_pointer = specifier_stack;
    uint8_t *specifier_virtual_program_counter; // virtual specifier counter
    memcpy(&specifier_virtual_program_counter, virtual_program_counter_ + NMBL, LBLL);
    uint8_t specifier_oeration_program_code;
    // positiveness feature of specifier element
    bool specifier_plus = true;
    T_SPECIFIER_STATES specifier_state = SPCNXT;
    while (true)
        switch (specifier_state)
        {
        // return from specifier element if "YES"
        case SPCRET:
            if (specifier_stack_pointer == specifier_stack)
                return specifier_plus;
            specifier_stack_pointer--;
            // work variable
            const bool specifier_work = specifier_plus;
            specifier_plus = specifier_stack_pointer->stack_plus;
            specifier_virtual_program_counter = specifier_stack_pointer->stack_virtual_private_code;
            if (specifier_work)
                break;
            specifier_virtual_program_counter += LBLL;
            specifier_state = SPCNXT;
            break;
        // return from specifier element if "NO"
        case SPCNXT:
            // specifier code
            specifier_oeration_program_code = *specifier_virtual_program_counter;
            specifier_virtual_program_counter += NMBL;
            // switch
            // SPCOP
            switch (specifier_oeration_program_code)
            {
            case 0000:
                specifier_state = SPCCLL;
                break;
            case 0001:
                specifier_state = SPCW;
                break;
            case 0002:
                specifier_state = SPCNG;
                break;
            case 0003:
                specifier_state = SPCNGW;
                break;
            case 0004:
                specifier_state = SPCSC;
                break;
            case 0005:
                specifier_state = SPCS;
                break;
            case 0006:
                specifier_state = SPCB;
                break;
            case 0007:
                specifier_state = SPCF;
                break;
            case 0010:
                specifier_state = SPCN;
                break;
            case 0011:
                specifier_state = SPCR;
                break;
            case 0012:
                specifier_state = SPCO;
                break;
            case 0013:
                specifier_state = SPCD;
                break;
            case 0014:
                specifier_state = SPCL;
            }
            break;
            // SPCCLL(L);
        case SPCCLL:
            specifier_stack_pointer->stack_plus = specifier_plus;
            specifier_stack_pointer->stack_virtual_private_code = specifier_virtual_program_counter;
            memcpy(&specifier_virtual_program_counter, specifier_virtual_program_counter, LBLL);
            specifier_stack_pointer++;
            specifier_plus = true;
            specifier_state = SPCNXT;
            break;
        case SPCW:
            specifier_state = SPCRET;
            break;
        case SPCNG:
            specifier_plus = !specifier_plus;
            specifier_state = SPCNXT;
            break;
        case SPCNGW:
            specifier_plus = !specifier_plus;
            specifier_state = SPCRET;
            break;
        case SPCSC:
            if (memcmp(specifier_virtual_program_counter, &b->tag, SMBL) == 0)
            {
                specifier_state = SPCRET;
                break;
            }
            specifier_virtual_program_counter += SMBL;
            specifier_state = SPCNXT;
            break;
        case SPCS:
            if (NOT_BRACKET(b))
            {
                specifier_state = SPCRET;
                break;
            }
            specifier_state = SPCNXT;
            break;
        case SPCB:
            if (BRACKET(b))
            {
                specifier_state = SPCRET;
                break;
            }
            specifier_state = SPCNXT;
            break;
        case SPCF:
            if (b->tag == TAGF)
            {
                specifier_state = SPCRET;
                break;
            }
            specifier_state = SPCNXT;
            break;
        case SPCN:
            if (b->tag == TAGN)
            {
                specifier_state = SPCRET;
                break;
            }
            specifier_state = SPCNXT;
            break;
        case SPCR:
            if (b->tag == TAGR)
            {
                specifier_state = SPCRET;
                break;
            }
            specifier_state = SPCNXT;
            break;
        case SPCO:
            if (b->tag == TAGO)
            {
                specifier_state = SPCRET;
                break;
            }
            specifier_state = SPCNXT;
            break;
        case SPCD:
            if (b->tag != TAGO)
            {
                specifier_state = SPCNXT;
                break;
            }
            if (digit(b->info.infoc))
            {
                specifier_state = SPCRET;
                break;
            }
            specifier_state = SPCNXT;
            break;
        case SPCL:
            if (b->tag != TAGO)
            {
                specifier_state = SPCNXT;
                break;
            }
            if (letter(b->info.infoc))
            {
                specifier_state = SPCRET;
                break;
            }
            specifier_state = SPCNXT;
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

//------------ end of file RFRUN.C ----------
