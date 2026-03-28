// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-03-25
// https://github.com/Aleksandr3Bocharov/refalab

//----------   file  CCST.C  ---------------
//      Compiler file
//------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "refalab.h"
#include "ccst.h"
#include "cerr.h"
#include "cs.h"
#include "cgop.h"
#include "cj.h"
#include "refal.h"

#define PRINT_ERROR_303 \
    print_error_two_strings("303 differents for variable ", variables[variable_index].identifier, variables[variable_index].identifier_length)

#define PRINT_ERROR_406 \
    print_error_two_strings("406 in left part missing variable ", variables[variable_index].identifier, variables[variable_index].identifier_length)

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
    uint16_t left_number_element, right_number_element;
    uint8_t type;
    uint8_t variable_index;
    T_LINKTI code;
    uint8_t next_variable;
    uint8_t pair_bracket;
    T_LINKTI specifier;
    bool v_variable;
    bool eoe_mark;
    uint8_t jump_stack_pointer;
} left_part_elements[256];

static struct
{ // variable table elements
    uint8_t last_left_part_element;
    uint8_t type;
    uint16_t main_right_number_element;
    uint32_t rem;
    char identifier[MAX_IDENTIFIER_LENGTH];
    uint8_t identifier_length;
    bool v_variable;
} variables[256];

// hole list
static struct
{
    uint8_t next_hole;
    uint8_t left_board, right_board;
} hole_list[130];

static T_LINKTI xncode;           // work structure
static T_LINKTI function_pointer; // work pointer

static uint8_t current_left_part_element, current_left_board, current_right_board; // left part element pointers
static uint8_t variable_index, temp_variable_index;                                // element index
static uint16_t number_element;                                                    // current number of element
static uint8_t jump_stack_pointer;                                                 // counter of the longing levels
static bool not_nil;                                                               // working variables
static uint8_t variables_count;                                                    // subprogram of search in variable table
                                                                                   // table pointer
static uint8_t last_bracket, temp_last_bracket;                                    // variables for brackets linkage
static uint8_t stoped_bracket_flag;                                                // stoped bracket flag
static uint32_t brackets_count[513];                                               // stack for counting of the brackets balance
static uint16_t brackets_k_level;
static uint8_t symbols_count;          // counter of the symbol number
static char symbols_buffer[81];        // buffer for generating of the "text" statement
static uint8_t current_hole;           // current hole number
static uint8_t free_segment_hole_list; // free segment number in the hole  list
static uint8_t next_hole;              // next hole number
static uint8_t hole_x, hole_y;         // hole numbers (under enter in brackets)

static void search_variable(void);
static void generate_boards_stoping_brackets(void);
static bool lsg_p(void);
static bool rsg_p(void);
static void generate_operator_e_v(uint8_t operator_e, uint8_t operator_v);
static bool ortogonality(uint8_t on1, uint8_t on2);

// read left part
// and full array X
void compile_sentence(bool direction, char *label, uint8_t label_length)
// direction;     matching feature :left to right or otherwise
// label;   sentence label
// label_length; sentence label length
{
    variables_count = 0;
    current_left_part_element = 0;
    last_bracket = 0;
    number_element = 0;
    T_STATES state = GET_LPE;
    while (true)
        switch (state)
        {
        case GET_LPE:
            // read left part element
            current_left_part_element++;
            scan_sentence_element();
            left_part_elements[current_left_part_element].type = current_sentence_element.type;
            left_part_elements[current_left_part_element].code.tag = current_sentence_element.code.tag;
            left_part_elements[current_left_part_element].code.info.codef = current_sentence_element.code.info.codef;
            left_part_elements[current_left_part_element].specifier.tag = current_sentence_element.specifier.tag;
            left_part_elements[current_left_part_element].specifier.info.codef = current_sentence_element.specifier.info.codef;
            left_part_elements[current_left_part_element].v_variable = current_sentence_element.v_variable;
            left_part_elements[current_left_part_element].next_variable = 0;
            left_part_elements[current_left_part_element].pair_bracket = 0;
            left_part_elements[current_left_part_element].eoe_mark = false;
            left_part_elements[current_left_part_element].jump_stack_pointer = 0;
            switch (current_sentence_element.type)
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
            number_element++;
            state = NEXT_LPE;
            break;
        case LPE2:
            // left bracket
            number_element++;
            left_part_elements[current_left_part_element].pair_bracket = last_bracket;
            last_bracket = current_left_part_element;
            state = NEXT_LPE;
            break;
        case LPE3:
            // right bracket
            if (last_bracket == 0)
            {
                print_error_string("302 too many ')' in left part");
                current_left_part_element--;
            }
            else
            {
                number_element++;
                temp_last_bracket = left_part_elements[last_bracket].pair_bracket;
                left_part_elements[last_bracket].pair_bracket = current_left_part_element;
                left_part_elements[current_left_part_element].pair_bracket = last_bracket;
                last_bracket = temp_last_bracket;
            }
            state = NEXT_LPE;
            break;
        case LPE4:
            // s-varyable
            search_variable();
            left_part_elements[current_left_part_element].variable_index = variable_index;
            switch (variables[variable_index].type)
            {
            case 0:
                variables[variable_index].type = 1; // yet isn't faced
                break;
            case 1:
                ++variables[variable_index].rem; // next position
                break;
            default: // invalid type pointer
                PRINT_ERROR_303;
                current_left_part_element--;
            }
            number_element++;
            state = NEXT_LPE;
            break;
        case LPE5:
            // w-varyable
            search_variable();
            left_part_elements[current_left_part_element].variable_index = variable_index;
            switch (variables[variable_index].type)
            {
            case 0:
                variables[variable_index].type = 2; // yet isn't faced
                break;
            case 2:
                ++variables[variable_index].rem; // next position
                break;
            default: // invalid type pointer
                PRINT_ERROR_303;
                current_left_part_element--;
            }
            number_element += 2;
            state = NEXT_LPE;
            break;
        case LPE6:
            // e- or v-varyable
            search_variable();
            left_part_elements[current_left_part_element].variable_index = variable_index;
            if (variables[variable_index].type == 0) // yet is't faced
                variables[variable_index].type = 3;
            else if (variables[variable_index].type == 3 && variables[variable_index].v_variable == current_sentence_element.v_variable)
                ++variables[variable_index].rem;
            else // invalid type pointer
            {
                PRINT_ERROR_303;
                current_left_part_element--;
            };
            number_element += 2;
            state = NEXT_LPE;
            break;
        case LPE7:
            // sign '<'
            print_error_string("306 sign '<' in left part");
            current_left_part_element--;
            state = NEXT_LPE;
            break;
        case LPE8:
            // sign '>'
            print_error_string("307 sign '>' in left part");
            current_left_part_element--;
            state = NEXT_LPE;
            break;
        case LPE9:
            // left part end
            if (last_bracket == 0)
            {
                state = RCG;
                break;
            }
            print_error_string("301 too many '(' in left part");
            state = OSH300;
            break;
        case LPE10:
            // sentence end
            print_error_string("304 under left part default sign '=' ");
            fndef(label, label_length);
            return;
        case NEXT_LPE:
            // end of element processing
            if (number_element <= 252)
            {
                state = GET_LPE;
                break;
            }
            print_error_string("305 very large left part");
            state = OSH300;
            break;
        case OSH300:
            fndef(label, label_length);
            state = RP_OSH300;
            break;
        //--------------------------------------------
        //         left part compilation
        //--------------------------------------------
        case RCG:
            fndef(label, label_length);
            current_left_board = 0;
            current_right_board = current_left_part_element;
            number_element = 4;
            left_part_elements[current_left_board].right_number_element = 3;
            left_part_elements[current_left_board].left_number_element = left_part_elements[current_left_board].right_number_element;
            left_part_elements[current_right_board].right_number_element = 2;
            left_part_elements[current_right_board].left_number_element = left_part_elements[current_right_board].right_number_element;
            hole_list[128].next_hole = 0;
            current_hole = 1;
            hole_list[1].next_hole = 128;
            free_segment_hole_list = 2;
            jump_stack_pointer = 0;
            if (direction)
            {
                state = RCGL;
                break;
            }
            state = RCGR;
            break;
        //    hard element projection from left part of current hole
        case RCGL:
            current_left_part_element = current_left_board + 1;
            if (current_left_part_element == current_right_board)
            {
                state = NIL;
                break;
            }
            switch (left_part_elements[current_left_part_element].type)
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
            if (left_part_elements[current_left_part_element].code.tag == TAGO)
            {
                state = LTXT;
                break;
            }
            generate_operator_s(n_lsc, &left_part_elements[current_left_part_element].code);
            state = L1;
            break;
        case LTXT:
            symbols_count = 1;
            state = LTXT1;
            break;
        case LTXT1:
            current_left_part_element++;
            if (current_left_part_element == current_right_board || left_part_elements[current_left_part_element].type != t_sc || left_part_elements[current_left_part_element].code.tag != TAGO)
            {
                state = LTXT2;
                break;
            }
            symbols_count++;
            break;
        case LTXT2:
            if (symbols_count == 1)
            {
                state = LSCO;
                break;
            }
            current_left_part_element = current_left_board;
            generate_operator_n(n_ltxt, symbols_count);
            state = LTXT3;
            break;
        case LTXT3:
            current_left_part_element++;
            jbyte((uint8_t)left_part_elements[current_left_part_element].code.info.infoc);
            left_part_elements[current_left_part_element].right_number_element = number_element;
            left_part_elements[current_left_part_element].left_number_element = left_part_elements[current_left_part_element].right_number_element;
            number_element++;
            symbols_count--;
            if (symbols_count == 0)
            {
                current_left_board = current_left_part_element;
                state = RCGL;
            }
            break;
        case LSCO:
            current_left_part_element = current_left_board + 1;
            generate_operator_n(n_lsco, (uint8_t)left_part_elements[current_left_part_element].code.info.infoc);
            state = L1;
            break;
        case LSW2:
            //  left bracket
            current_left_board = current_left_part_element;
            current_left_part_element = left_part_elements[current_left_board].pair_bracket;
            if (current_left_board + 1 == current_left_part_element)
            {
                state = LBNIL;
                break;
            }
            if (current_left_board + 2 != current_left_part_element)
            {
                state = GEN_LB;
                break;
            }
            current_left_part_element = current_left_board + 1;
            if (left_part_elements[current_left_part_element].type != t_e)
            {
                state = GEN_LB;
                break;
            }
            variable_index = left_part_elements[current_left_part_element].variable_index;
            if (variables[variable_index].last_left_part_element != 0)
            {
                state = GEN_LB;
                break;
            }
            state = LBCE;
            break;
        case LBCE:
            number_element += 2;
            jbyte(n_lbce);
            variables[variable_index].main_right_number_element = number_element + 1;
            left_part_elements[current_left_part_element].next_variable = variables[variable_index].last_left_part_element;
            variables[variable_index].last_left_part_element = current_left_part_element;
            variables[variable_index].rem--;
            if (left_part_elements[current_left_part_element].v_variable)
                jbyte(n_nnil);
            if (left_part_elements[current_left_part_element].specifier.info.codef != NULL)
                generate_operator_l(n_espc, left_part_elements[current_left_part_element].specifier.info.codef);
            left_part_elements[current_left_part_element].left_number_element = number_element;
            left_part_elements[current_left_part_element].right_number_element = number_element + 1;
            number_element += 2;
            left_part_elements[current_left_board].right_number_element = number_element - 4;
            left_part_elements[current_left_board].left_number_element = left_part_elements[current_left_board].right_number_element;
            current_left_board += 2;
            left_part_elements[current_left_board].right_number_element = number_element - 3;
            left_part_elements[current_left_board].left_number_element = left_part_elements[current_left_board].right_number_element;
            state = RCGL;
            break;
        case LBNIL:
            jbyte(n_lbnil);
            left_part_elements[current_left_board].right_number_element = number_element;
            left_part_elements[current_left_board].left_number_element = left_part_elements[current_left_board].right_number_element;
            current_left_board = current_left_part_element;
            left_part_elements[current_left_board].right_number_element = number_element + 1;
            left_part_elements[current_left_board].left_number_element = left_part_elements[current_left_board].right_number_element;
            number_element += 2;
            state = RCGL;
            break;
        case GEN_LB:
            current_left_part_element = current_left_board;
            stoped_bracket_flag = 1;
            state = LB1;
            break;
        case LB1:
            if (direction)
            {
                hole_x = current_hole;
                hole_y = free_segment_hole_list;
            }
            else
            {
                hole_x = free_segment_hole_list;
                hole_y = current_hole;
            };
            hole_list[free_segment_hole_list].next_hole = hole_list[current_hole].next_hole;
            hole_list[current_hole].next_hole = free_segment_hole_list;
            free_segment_hole_list++;
            current_left_board = current_left_part_element;
            current_left_part_element = current_right_board;
            current_right_board = left_part_elements[current_left_board].pair_bracket;
            current_hole = hole_x;
            hole_list[hole_y].left_board = current_right_board;
            hole_list[hole_y].right_board = current_left_part_element;
            left_part_elements[current_left_board].right_number_element = number_element;
            left_part_elements[current_left_board].left_number_element = left_part_elements[current_left_board].right_number_element;
            left_part_elements[current_right_board].right_number_element = number_element + 1;
            left_part_elements[current_right_board].left_number_element = left_part_elements[current_right_board].right_number_element;
            number_element += 2;
            state = HSCH;
            break;
        case LSW3:
            state = ERROR;
            break;
        case LSW4:
            //    s-variable
            variable_index = left_part_elements[current_left_part_element].variable_index;
            if (variables[variable_index].last_left_part_element != 0)
                generate_operator_n(n_lsd, (uint8_t)variables[variable_index].main_right_number_element);
            else
            {
                jbyte(n_ls);
                variables[variable_index].main_right_number_element = number_element;
            };
            state = LSMD;
            break;
        case LSMD:
            left_part_elements[current_left_part_element].next_variable = variables[variable_index].last_left_part_element;
            variables[variable_index].last_left_part_element = current_left_part_element;
            variables[variable_index].rem--;
            if (left_part_elements[current_left_part_element].specifier.info.codef != NULL)
                generate_operator_l(n_wspc, left_part_elements[current_left_part_element].specifier.info.codef);
            state = L1;
            break;
        case L1:
            left_part_elements[current_left_part_element].right_number_element = number_element;
            left_part_elements[current_left_part_element].left_number_element = left_part_elements[current_left_part_element].right_number_element;
            number_element++;
            current_left_board = current_left_part_element;
            state = RCGL;
            break;
        case LSW5:
            //   w-variable
            variable_index = left_part_elements[current_left_part_element].variable_index;
            if (variables[variable_index].last_left_part_element != 0)
            {
                state = LED;
                break;
            }
            jbyte(n_lw);
            variables[variable_index].main_right_number_element = number_element + 1;
            left_part_elements[current_left_part_element].next_variable = variables[variable_index].last_left_part_element;
            variables[variable_index].last_left_part_element = current_left_part_element;
            variables[variable_index].rem--;
            if (left_part_elements[current_left_part_element].specifier.info.codef != NULL)
                generate_operator_l(n_wspc, left_part_elements[current_left_part_element].specifier.info.codef);
            state = L2;
            break;
        case LED:
            generate_operator_n(n_led, (uint8_t)variables[variable_index].main_right_number_element);
            state = LEMD;
            break;
        case LEMD:
            left_part_elements[current_left_part_element].next_variable = variables[variable_index].last_left_part_element;
            variables[variable_index].last_left_part_element = current_left_part_element;
            variables[variable_index].rem--;
            if (left_part_elements[current_left_part_element].specifier.info.codef != NULL)
                generate_operator_l(n_espc, left_part_elements[current_left_part_element].specifier.info.codef);
            state = L2;
            break;
        case L2:
            left_part_elements[current_left_part_element].left_number_element = number_element;
            left_part_elements[current_left_part_element].right_number_element = number_element + 1;
            number_element += 2;
            current_left_board = current_left_part_element;
            state = RCGL;
            break;
        case LSW6:
            //   e-variable
            variable_index = left_part_elements[current_left_part_element].variable_index;
            if (variables[variable_index].last_left_part_element != 0)
            {
                state = LED;
                break;
            }
            if (direction)
            {
                state = RCGR;
                break;
            }
            if (current_left_part_element + 1 == current_right_board)
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
            current_left_part_element = current_right_board - 1;
            if (current_left_part_element == current_left_board)
            {
                state = NIL;
                break;
            }
            switch (left_part_elements[current_left_part_element].type)
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
            if (left_part_elements[current_left_part_element].code.tag == TAGO)
            {
                state = RTXT;
                break;
            }
            generate_operator_s(n_rsc, &left_part_elements[current_left_part_element].code);
            state = R1;
            break;
        case RTXT:
            symbols_count = 1;
            state = RTXT1;
            break;
        case RTXT1:
            current_left_part_element--;
            if (current_left_part_element == current_left_board || left_part_elements[current_left_part_element].type != t_sc || left_part_elements[current_left_part_element].code.tag != TAGO)
            {
                state = RTXT2;
                break;
            }
            symbols_count++;
            break;
        case RTXT2:
            if (symbols_count == 1)
            {
                state = RSCO;
                break;
            }
            current_left_part_element = current_right_board;
            generate_operator_n(n_rtxt, symbols_count);
            state = RTXT3;
            break;
        case RTXT3:
            current_left_part_element--;
            jbyte((uint8_t)left_part_elements[current_left_part_element].code.info.infoc);
            left_part_elements[current_left_part_element].right_number_element = number_element;
            left_part_elements[current_left_part_element].left_number_element = left_part_elements[current_left_part_element].right_number_element;
            number_element++;
            symbols_count--;
            if (symbols_count == 0)
            {
                current_right_board = current_left_part_element;
                state = RCGR;
            }
            break;
        case RSCO:
            current_left_part_element = current_right_board - 1;
            generate_operator_n(n_rsco, (uint8_t)left_part_elements[current_left_part_element].code.info.infoc);
            state = R1;
            break;
        case RSW2:
            state = ERROR;
            break;
        case RSW3:
            //     right bracket
            current_right_board = current_left_part_element;
            current_left_part_element = left_part_elements[current_right_board].pair_bracket;
            if (current_left_part_element + 1 == current_right_board)
            {
                state = RBNIL;
                break;
            }
            if (current_left_part_element + 2 != current_right_board)
            {
                state = GEN_RB;
                break;
            }
            current_left_part_element = current_right_board - 1;
            if (left_part_elements[current_left_part_element].type != t_e)
            {
                state = GEN_RB;
                break;
            }
            variable_index = left_part_elements[current_left_part_element].variable_index;
            if (variables[variable_index].last_left_part_element != 0)
            {
                state = GEN_RB;
                break;
            }
            state = RBCE;
            break;
        case RBCE:
            number_element += 2;
            jbyte(n_rbce);
            variables[variable_index].main_right_number_element = number_element + 1;
            left_part_elements[current_left_part_element].next_variable = variables[variable_index].last_left_part_element;
            variables[variable_index].last_left_part_element = current_left_part_element;
            variables[variable_index].rem--;
            if (left_part_elements[current_left_part_element].v_variable)
                jbyte(n_nnil);
            if (left_part_elements[current_left_part_element].specifier.info.codef != NULL)
                generate_operator_l(n_espc, left_part_elements[current_left_part_element].specifier.info.codef);
            left_part_elements[current_left_part_element].left_number_element = number_element;
            left_part_elements[current_left_part_element].right_number_element = number_element + 1;
            number_element += 2;
            left_part_elements[current_right_board].right_number_element = number_element - 3;
            left_part_elements[current_right_board].left_number_element = left_part_elements[current_right_board].right_number_element;
            current_right_board -= 2;
            left_part_elements[current_right_board].right_number_element = number_element - 4;
            left_part_elements[current_right_board].left_number_element = left_part_elements[current_right_board].right_number_element;
            state = RCGR;
            break;
        case RBNIL:
            jbyte(n_rbnil);
            left_part_elements[current_right_board].right_number_element = number_element + 1;
            left_part_elements[current_right_board].left_number_element = left_part_elements[current_right_board].right_number_element;
            current_right_board = current_left_part_element;
            left_part_elements[current_right_board].right_number_element = number_element;
            left_part_elements[current_right_board].left_number_element = left_part_elements[current_right_board].right_number_element;
            number_element += 2;
            state = RCGR;
            break;
        case GEN_RB:
            current_left_part_element = current_right_board;
            stoped_bracket_flag = 2;
            state = RB1;
            break;
        case RB1:
            if (!direction)
            {
                hole_x = current_hole;
                hole_y = free_segment_hole_list;
            }
            else
            {
                hole_x = free_segment_hole_list;
                hole_y = current_hole;
            };
            hole_list[free_segment_hole_list].next_hole = hole_list[current_hole].next_hole;
            hole_list[current_hole].next_hole = free_segment_hole_list;
            free_segment_hole_list++;
            current_right_board = current_left_part_element;
            current_left_part_element = current_left_board;
            current_left_board = left_part_elements[current_right_board].pair_bracket;
            current_hole = hole_x;
            hole_list[hole_y].left_board = current_left_part_element;
            hole_list[hole_y].right_board = current_left_board;
            left_part_elements[current_left_board].right_number_element = number_element;
            left_part_elements[current_left_board].left_number_element = left_part_elements[current_left_board].right_number_element;
            left_part_elements[current_right_board].right_number_element = number_element + 1;
            left_part_elements[current_right_board].left_number_element = left_part_elements[current_right_board].right_number_element;
            number_element += 2;
            state = HSCH;
            break;
        case RSW4:
            //     s_variable
            variable_index = left_part_elements[current_left_part_element].variable_index;
            if (variables[variable_index].last_left_part_element != 0)
                generate_operator_n(n_rsd, (uint8_t)variables[variable_index].main_right_number_element);
            else
            {
                jbyte(n_rs);
                variables[variable_index].main_right_number_element = number_element;
            };
            state = RSMD;
            break;
        case RSMD:
            left_part_elements[current_left_part_element].next_variable = variables[variable_index].last_left_part_element;
            variables[variable_index].last_left_part_element = current_left_part_element;
            variables[variable_index].rem--;
            if (left_part_elements[current_left_part_element].specifier.info.codef != NULL)
                generate_operator_l(n_wspc, left_part_elements[current_left_part_element].specifier.info.codef);
            state = R1;
            break;
        case R1:
            left_part_elements[current_left_part_element].right_number_element = number_element;
            left_part_elements[current_left_part_element].left_number_element = left_part_elements[current_left_part_element].right_number_element;
            number_element++;
            current_right_board = current_left_part_element;
            state = RCGR;
            break;
        case RSW5:
            //    w_variable
            variable_index = left_part_elements[current_left_part_element].variable_index;
            if (variables[variable_index].last_left_part_element != 0)
            {
                state = RED;
                break;
            }
            jbyte(n_rw);
            variables[variable_index].main_right_number_element = number_element + 1;
            left_part_elements[current_left_part_element].next_variable = variables[variable_index].last_left_part_element;
            variables[variable_index].last_left_part_element = current_left_part_element;
            variables[variable_index].rem--;
            if (left_part_elements[current_left_part_element].specifier.info.codef != NULL)
                generate_operator_l(n_wspc, left_part_elements[current_left_part_element].specifier.info.codef);
            state = R2;
            break;
        case RED:
            generate_operator_n(n_red, (uint8_t)variables[variable_index].main_right_number_element);
            state = REMD;
            break;
        case REMD:
            left_part_elements[current_left_part_element].next_variable = variables[variable_index].last_left_part_element;
            variables[variable_index].last_left_part_element = current_left_part_element;
            variables[variable_index].rem--;
            if (left_part_elements[current_left_part_element].specifier.info.codef != NULL)
                generate_operator_l(n_espc, left_part_elements[current_left_part_element].specifier.info.codef);
            state = R2;
            break;
        case R2:
            left_part_elements[current_left_part_element].left_number_element = number_element;
            left_part_elements[current_left_part_element].right_number_element = number_element + 1;
            number_element += 2;
            current_right_board = current_left_part_element;
            state = RCGR;
            break;
        case RSW6:
            //    e-variable
            variable_index = left_part_elements[current_left_part_element].variable_index;
            if (variables[variable_index].last_left_part_element != 0)
            {
                state = RED;
                break;
            }
            if (!direction)
            {
                state = RCGL;
                break;
            }
            if (current_left_board + 1 == current_left_part_element)
            {
                state = CE;
                break;
            }
            state = IMPASSE;
            break;
        case NIL:
            //     empty hole
            jbyte(n_nil);
            next_hole = hole_list[current_hole].next_hole;
            hole_list[current_hole].next_hole = hole_list[next_hole].next_hole;
            hole_list[current_hole].left_board = hole_list[next_hole].left_board;
            hole_list[current_hole].right_board = hole_list[next_hole].right_board;
            current_hole = next_hole;
            state = IMPASSE;
            break;
        case CE:
            //   closed including
            if (left_part_elements[current_left_part_element].eoe_mark)
            {
                state = IMPASSE;
                break;
            }
            state = CE1;
            break;
        case CE1:
            if (left_part_elements[current_left_part_element].specifier.info.codef == NULL)
            {
                state = CE2;
                break;
            }
            if (direction)
            {
                state = LMAX;
                break;
            }
            state = RMAX;
            break;
        case CE2:
            variable_index = left_part_elements[current_left_part_element].variable_index;
            variables[variable_index].main_right_number_element = number_element + 1;
            jbyte(n_ce);
            left_part_elements[current_left_part_element].next_variable = variables[variable_index].last_left_part_element;
            variables[variable_index].last_left_part_element = current_left_part_element;
            variables[variable_index].rem--;
            if (left_part_elements[current_left_part_element].v_variable)
                jbyte(n_nnil);
            left_part_elements[current_left_part_element].left_number_element = number_element;
            left_part_elements[current_left_part_element].right_number_element = number_element + 1;
            number_element += 2;
            next_hole = hole_list[current_hole].next_hole;
            hole_list[current_hole].next_hole = hole_list[next_hole].next_hole;
            hole_list[current_hole].left_board = hole_list[next_hole].left_board;
            hole_list[current_hole].right_board = hole_list[next_hole].right_board;
            current_hole = next_hole;
            state = IMPASSE;
            break;
        //
        //          It is impossible movement
        //          on hard element here or
        //          hole ended here
        case IMPASSE:
            stoped_bracket_flag = 0;
            hole_x = current_hole;
            state = HSCH;
            break;
        //          Search of hole with hard
        //          elements on its boards.
        //          If it not exist than project
        //          e-variable from first hole.
        case HSCH:
            hole_list[current_hole].left_board = current_left_board;
            hole_list[current_hole].right_board = current_right_board;
            current_hole = 1;
            if (hole_list[current_hole].next_hole == 0)
            {
                state = RCGFIN;
                break;
            }
            state = NHOLE;
            break;
        case NHOLE:
            current_left_board = hole_list[current_hole].left_board;
            current_right_board = hole_list[current_hole].right_board;
            current_left_part_element = current_left_board + 1;
            if (current_left_part_element == current_right_board)
            {
                generate_boards_stoping_brackets();
                state = NIL;
                break;
            }
            if (left_part_elements[current_left_part_element].type != t_e)
            {
                state = RIGID;
                break;
            }
            variable_index = left_part_elements[current_left_part_element].variable_index;
            if (variables[variable_index].last_left_part_element != 0)
            {
                state = RIGID;
                break;
            }
            if (current_left_part_element + 1 == current_right_board)
            {
                if (variables[variable_index].rem == 1)
                {
                    state = NHOLE1;
                    break;
                }
                generate_boards_stoping_brackets();
                state = CE1;
                break;
            }
            current_left_part_element = current_right_board - 1;
            if (left_part_elements[current_left_part_element].type != t_e)
            {
                state = RIGID;
                break;
            }
            variable_index = left_part_elements[current_left_part_element].variable_index;
            if (variables[variable_index].last_left_part_element != 0)
            {
                state = RIGID;
                break;
            }
            state = NHOLE1;
            break;
        case NHOLE1:
            current_hole = hole_list[current_hole].next_hole;
            if (hole_list[current_hole].next_hole == 0)
            {
                state = OE;
                break;
            }
            state = NHOLE;
            break;
        case RIGID:
            //  hard element on the both hole boards
            generate_boards_stoping_brackets();
            if (direction)
            {
                state = RCGL;
                break;
            }
            state = RCGR;
            break;
        //  opened e_variable processing
        case OE:
            current_hole = 1;
            current_left_board = hole_list[current_hole].left_board;
            current_right_board = hole_list[current_hole].right_board;
            generate_boards_stoping_brackets();
            if (direction)
                current_left_part_element = current_left_board + 1;
            else
                current_left_part_element = current_right_board - 1;
            variable_index = left_part_elements[current_left_part_element].variable_index;
            if (left_part_elements[current_left_part_element].eoe_mark)
            {
                const uint8_t diff = jump_stack_pointer - left_part_elements[current_left_part_element].jump_stack_pointer;
                if (diff == 1)
                    jbyte(n_eoei);
                else
                    generate_operator_n(n_eoe, diff);
                jump_stack_pointer = left_part_elements[current_left_part_element].jump_stack_pointer;
                left_part_elements[current_left_part_element].eoe_mark = false;
                left_part_elements[current_left_part_element].jump_stack_pointer = 0;
            };
            if (current_left_board + 2 == current_right_board)
            {
                state = CE1;
                break;
            }
            if (direction)
                current_left_part_element = current_right_board - 1;
            else
                current_left_part_element = current_left_board + 1;
            if (left_part_elements[current_left_part_element].specifier.info.codef == NULL)
            {
                state = OE1;
                break;
            }
            state = OE0;
            break;
        case OE0:
            variable_index = left_part_elements[current_left_part_element].variable_index;
            if (variables[variable_index].last_left_part_element != 0 || variables[variable_index].rem != 1)
            {
                state = OE1;
                break;
            }
            if (direction)
            {
                current_left_part_element--;
                if (current_left_part_element == current_left_board)
                {
                    state = OERMAX;
                    break;
                }
            }
            else
            {
                current_left_part_element++;
                if (current_left_part_element == current_right_board)
                {
                    state = OELMAX;
                    break;
                }
            };
            if (left_part_elements[current_left_part_element].type != t_e || left_part_elements[current_left_part_element].v_variable)
            {
                state = OE1;
                break;
            }
            state = OE0;
            break;
        case OERMAX:
            current_left_part_element = current_right_board - 1;
            variable_index = left_part_elements[current_left_part_element].variable_index;
            state = RMAX;
            break;
        case RMAX:
            generate_operator_l(n_rmax, left_part_elements[current_left_part_element].specifier.info.codef);
            if (left_part_elements[current_left_part_element].v_variable)
                jbyte(n_nnil);
            left_part_elements[current_left_part_element].specifier.info.codef = NULL;
            state = REM;
            break;
        case REM:
            variables[variable_index].main_right_number_element = number_element + 1;
            state = REMD;
            break;
        case OELMAX:
            current_left_part_element = current_left_board + 1;
            variable_index = left_part_elements[current_left_part_element].variable_index;
            state = LMAX;
            break;
        case LMAX:
            generate_operator_l(n_lmax, left_part_elements[current_left_part_element].specifier.info.codef);
            if (left_part_elements[current_left_part_element].v_variable)
                jbyte(n_nnil);
            left_part_elements[current_left_part_element].specifier.info.codef = NULL;
            state = LEM;
            break;
        case LEM:
            variables[variable_index].main_right_number_element = number_element + 1;
            state = LEMD;
            break;
        case OE1:
            if (direction)
            {
                state = LOE;
                break;
            }
            state = ROE;
            break;
        case LOE:
            current_left_part_element = current_left_board + 1;
            temp_variable_index = left_part_elements[current_left_part_element].variable_index;
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
            current_left_part_element = current_left_board + 1;
            switch (left_part_elements[current_left_part_element].type)
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
            xncode.tag = left_part_elements[current_left_part_element].code.tag;
            xncode.info.codef = left_part_elements[current_left_part_element].code.info.codef;
            current_left_part_element++;
            if (!not_nil && left_part_elements[current_left_part_element].eoe_mark)
            {
                left_part_elements[current_left_part_element].eoe_mark = false;
                left_part_elements[current_left_part_element].jump_stack_pointer = 0;
                jump_stack_pointer--;
                generate_operator_s(n_lsrch, &xncode);
            }
            else
            {
                generate_operator_e_v(n_plesc, n_plvsc);
                generate_operator_s(n_lesc, &xncode);
            };
            current_left_part_element--;
            state = L1;
            break;
        case LESW2:
            //   ei ( . . . ) . . .
            generate_operator_e_v(n_pleb, n_plvb);
            jbyte(n_leb);
            stoped_bracket_flag = 0;
            state = LB1;
            break;
        case LESW3:
            state = ERROR;
            break;
        case LESW4:
            //  ei sj . . .
            variable_index = left_part_elements[current_left_part_element].variable_index;
            if (variables[variable_index].last_left_part_element == 0)
            {
                state = LE;
                break;
            }
            generate_operator_e_v(n_plesc, n_plvsc);
            generate_operator_n(n_lesd, (uint8_t)variables[variable_index].main_right_number_element);
            state = LSMD;
            break;
        case LESW5:
        case LESW6:
            //  ei . . .
            state = LE;
            break;
        case LE:
            generate_operator_e_v(n_ple, n_plv);
            jbyte(n_le);
            state = RCGL;
            break;
        case ROE:
            current_left_part_element = current_right_board - 1;
            temp_variable_index = left_part_elements[current_left_part_element].variable_index;
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
            current_left_part_element = current_right_board - 1;
            switch (left_part_elements[current_left_part_element].type)
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
            xncode.tag = left_part_elements[current_left_part_element].code.tag;
            xncode.info.codef = left_part_elements[current_left_part_element].code.info.codef;
            current_left_part_element--;
            if (!not_nil && left_part_elements[current_left_part_element].eoe_mark)
            {
                left_part_elements[current_left_part_element].eoe_mark = false;
                left_part_elements[current_left_part_element].jump_stack_pointer = 0;
                jump_stack_pointer--;
                generate_operator_s(n_rsrch, &xncode);
            }
            else
            {
                generate_operator_e_v(n_presc, n_prvsc);
                generate_operator_s(n_resc, &xncode);
            };
            current_left_part_element++;
            state = R1;
            break;
        case RESW2:
            state = ERROR;
            break;
        case RESW3:
            // . . .  ( . . .  ) ei
            generate_operator_e_v(n_preb, n_prvb);
            jbyte(n_reb);
            stoped_bracket_flag = 0;
            state = RB1;
            break;
        case RESW4:
            //  . . . sj ei
            variable_index = left_part_elements[current_left_part_element].variable_index;
            if (variables[variable_index].last_left_part_element == 0)
            {
                state = RE;
                break;
            }
            generate_operator_e_v(n_presc, n_prvsc);
            generate_operator_n(n_resd, (uint8_t)variables[variable_index].main_right_number_element);
            state = RSMD;
            break;
        case RESW5:
        case RESW6:
            // . . .  ei
            state = RE;
            break;
        case RE:
            generate_operator_e_v(n_pre, n_prv);
            jbyte(n_re);
            state = RCGR;
            break;
        //                 identification end
        case RCGFIN:
            jbyte(n_eor);
            //--------------------------------------------
            //         right part compilation
            //--------------------------------------------
            brackets_k_level = 1;
            brackets_count[brackets_k_level] = 0;
            state = GET_RPE;
            break;
        //  read next element of right part
        case GET_RPE:
            scan_sentence_element();
            state = SW_RPE;
            break;
        case SW_RPE:
            switch (current_sentence_element.type)
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
            if (current_sentence_element.code.tag == TAGO)
            {
                state = TEXT;
                break;
            }
            generate_operator_s(n_ns, &current_sentence_element.code);
            state = GET_RPE;
            break;
        case TEXT:
            symbols_count = 0;
            state = TEXT1;
            break;
        case TEXT1:
            symbols_count++;
            symbols_buffer[symbols_count] = current_sentence_element.code.info.infoc;
            scan_sentence_element();
            if (symbols_count < 80 && current_sentence_element.type == t_sc && current_sentence_element.code.tag == TAGO)
                break;
            if (symbols_count == 1)
                generate_operator_n(n_nso, (uint8_t)symbols_buffer[1]);
            else
            {
                generate_operator_n(n_text, symbols_count);
                for (uint8_t k = 1; k <= symbols_count; k++)
                    jbyte((uint8_t)symbols_buffer[k]);
            };
            state = SW_RPE;
            break;
        case RPE2:
            // left bracket
            scan_sentence_element();
            if (current_sentence_element.type == t_rb)
            {
                jbyte(n_blr);
                state = GET_RPE;
                break;
            }
            brackets_count[brackets_k_level]++;
            if (current_sentence_element.type == t_sc && current_sentence_element.code.tag == TAGF)
            {
                function_pointer.info.codef = current_sentence_element.code.info.codef;
                generate_operator_l(n_blf, function_pointer.info.codef);
                state = GET_RPE;
                break;
            }
            jbyte(n_bl);
            state = SW_RPE;
            break;
        case RPE3:
            // right bracket
            jbyte(n_br);
            if (brackets_count[brackets_k_level] == 0)
                print_error_string("402 too many ')' in right part");
            else
                brackets_count[brackets_k_level]--;
            state = GET_RPE;
            break;
        case RPE4:
            // s - varyable
            search_variable();
            switch (variables[variable_index].type)
            {
            case 0:
                PRINT_ERROR_406;
                break;
            case 1:
                current_left_part_element = variables[variable_index].last_left_part_element;
                if (current_left_part_element == 0)
                    generate_operator_n(n_muls, (uint8_t)variables[variable_index].main_right_number_element);
                else
                {
                    generate_operator_n(n_tpls, (uint8_t)left_part_elements[current_left_part_element].right_number_element);
                    variables[variable_index].last_left_part_element = left_part_elements[current_left_part_element].next_variable;
                };
                break;
            default:
                PRINT_ERROR_303;
            };
            state = GET_RPE;
            break;
        case RPE5:
            // w - varyable
            search_variable();
            switch (variables[variable_index].type)
            {
            case 0:
                PRINT_ERROR_406;
                break;
            case 2:
                current_left_part_element = variables[variable_index].last_left_part_element;
                if (current_left_part_element == 0)
                    generate_operator_n(n_mule, (uint8_t)variables[variable_index].main_right_number_element);
                else
                {
                    generate_operator_n(n_tplv, (uint8_t)left_part_elements[current_left_part_element].right_number_element);
                    variables[variable_index].last_left_part_element = left_part_elements[current_left_part_element].next_variable;
                };
                break;
            default:
                PRINT_ERROR_303;
            };
            state = GET_RPE;
            break;
        case RPE6:
            // e- or v-varyable
            search_variable();
            if (variables[variable_index].type == 0)
                PRINT_ERROR_406;
            else if (variables[variable_index].type == 3 && variables[variable_index].v_variable == current_sentence_element.v_variable)
            {
                current_left_part_element = variables[variable_index].last_left_part_element;
                if (current_left_part_element == 0)
                    generate_operator_n(n_mule, (uint8_t)variables[variable_index].main_right_number_element);
                else
                {
                    if (variables[variable_index].v_variable)
                        generate_operator_n(n_tplv, (uint8_t)left_part_elements[current_left_part_element].right_number_element);
                    else
                        generate_operator_n(n_tple, (uint8_t)left_part_elements[current_left_part_element].right_number_element);
                    variables[variable_index].last_left_part_element = left_part_elements[current_left_part_element].next_variable;
                };
            }
            else
                PRINT_ERROR_303;
            state = GET_RPE;
            break;
        case RPE7:
            // sign '<'
            if (brackets_k_level > 511)
            {
                print_error_string("407 including of the signs '<' > 511");
                state = RP_OSH300;
                break;
            }
            brackets_count[++brackets_k_level] = 0;
            scan_sentence_element();
            if (current_sentence_element.type == t_sc && current_sentence_element.code.tag == TAGF)
            {
                function_pointer.info.codef = current_sentence_element.code.info.codef;
                function_pointer.tag = TAGO;
                generate_operator_l(n_blf, function_pointer.info.codef);
                state = GET_RPE;
                break;
            }
            jbyte(n_bl);
            state = SW_RPE;
            break;
        case RPE8:
            // sign '>'
            if (brackets_k_level == 1)
                print_error_string("404 too many sign '>' in right part");
            else
            {
                if (brackets_count[brackets_k_level] != 0)
                    print_error_string("401 too many '(' in right part");
                jbyte(n_bract);
                brackets_k_level--;
            };
            state = GET_RPE;
            break;
        case RPE9:
            // sign '=' in right part
            print_error_string("405 sign '=' in right part");
            state = GET_RPE;
            break;
        case RPE10:
            // sentence end
            jbyte(n_eos);
            if (brackets_k_level != 1)
                print_error_string("403 too many signs '<' in right part");
            if (brackets_count[brackets_k_level] != 0)
                print_error_string("401 too many '(' in right part");
            return;
        case RP_OSH300:
            print_error_string("300 sentence is't scanned");
            return;
        //                      place of compiler's error
        case ERROR:
            printf("Compiler's error\n");
            exit(1);
            return;
        }
}

static void search_variable(void)
{
    for (variable_index = 1; variable_index <= variables_count; variable_index++)
        if (variables[variable_index].identifier_length == current_sentence_element.identifier_length && strncmp(variables[variable_index].identifier, current_sentence_element.identifier, variables[variable_index].identifier_length) == 0)
            return;
    variable_index = ++variables_count;
    strncpy(variables[variable_index].identifier, current_sentence_element.identifier, current_sentence_element.identifier_length);
    variables[variable_index].identifier_length = current_sentence_element.identifier_length;
    variables[variable_index].type = 0;
    variables[variable_index].rem = 1;
    variables[variable_index].last_left_part_element = 0;
    variables[variable_index].v_variable = current_sentence_element.v_variable;
    return;
}

//   generation of stoped brackets and setting boards
static void generate_boards_stoping_brackets(void)
{
    switch (stoped_bracket_flag)
    {
    case 0: // no stoped brackets
        if (current_hole != hole_x)
            break;
        return;
    case 1: // left stoped brackets
        if (current_hole == hole_x)
            jbyte(n_lb);
        else if (current_hole == hole_y)
            jbyte(n_lby);
        else
        {
            jbyte(n_lb);
            break;
        };
        return;
    case 2: // right stoped brackets
        if (current_hole == hole_x)
            jbyte(n_rb);
        else if (current_hole == hole_y)
            jbyte(n_rby);
        else
        {
            jbyte(n_rb);
            break;
        };
        return;
    };
    generate_operator_n_m(n_sb1b2, (uint8_t)left_part_elements[current_left_board].right_number_element, (uint8_t)left_part_elements[current_right_board].left_number_element);
    return;
}

//    attempt to extract left support group
static bool lsg_p(void)
{
    while (true)
    {
        current_left_part_element++;
        if (current_left_part_element == current_right_board)
            break;
        if (left_part_elements[current_left_part_element].type != t_lb)
        {
            if (left_part_elements[current_left_part_element].type != t_e)
                continue;
            variable_index = left_part_elements[current_left_part_element].variable_index;
            if (variable_index == temp_variable_index || variables[variable_index].last_left_part_element != 0)
                continue;
            if (left_part_elements[current_left_part_element].specifier.info.codef != NULL || variables[variable_index].rem != 1)
                break;
            if (!ortogonality(current_left_board, current_left_part_element))
                break;
            left_part_elements[current_left_part_element].eoe_mark = true;
            left_part_elements[current_left_part_element].jump_stack_pointer = jump_stack_pointer;
            break;
        }
        current_left_part_element = left_part_elements[current_left_part_element].pair_bracket;
    }
    current_left_board++;
    current_left_part_element = current_left_board;
    variable_index = left_part_elements[current_left_part_element].variable_index;
    variables[variable_index].main_right_number_element = number_element + 1;
    left_part_elements[current_left_part_element].next_variable = variables[variable_index].last_left_part_element;
    variables[variable_index].last_left_part_element = current_left_part_element;
    variables[variable_index].rem--;
    left_part_elements[current_left_part_element].left_number_element = number_element;
    left_part_elements[current_left_part_element].right_number_element = number_element + 1;
    number_element += 2;
    jump_stack_pointer++;
    not_nil = left_part_elements[current_left_part_element].v_variable;
    if (left_part_elements[current_left_part_element].specifier.info.codef == NULL)
        return true;
    generate_operator_e_v(n_plespc, n_plv);
    generate_operator_l(n_lespc, left_part_elements[current_left_part_element].specifier.info.codef);
    return false;
}

//        attempt to extract right support group
static bool rsg_p(void)
{
    while (true)
    {
        current_left_part_element--;
        if (current_left_part_element == current_left_board)
            break;
        if (left_part_elements[current_left_part_element].type != t_rb)
        {
            if (left_part_elements[current_left_part_element].type != t_e)
                continue;
            variable_index = left_part_elements[current_left_part_element].variable_index;
            if (variable_index == temp_variable_index || variables[variable_index].last_left_part_element != 0)
                continue;
            if (left_part_elements[current_left_part_element].specifier.info.codef != NULL || variables[variable_index].rem != 1)
                break;
            if (!ortogonality(current_left_part_element, current_right_board))
                break;
            left_part_elements[current_left_part_element].eoe_mark = true;
            left_part_elements[current_left_part_element].jump_stack_pointer = jump_stack_pointer;
            break;
        }
        current_left_part_element = left_part_elements[current_left_part_element].pair_bracket;
    }
    current_right_board--;
    current_left_part_element = current_right_board;
    variable_index = left_part_elements[current_left_part_element].variable_index;
    variables[variable_index].main_right_number_element = number_element + 1;
    left_part_elements[current_left_part_element].next_variable = variables[variable_index].last_left_part_element;
    variables[variable_index].last_left_part_element = current_left_part_element;
    variables[variable_index].rem--;
    left_part_elements[current_left_part_element].left_number_element = number_element;
    left_part_elements[current_left_part_element].right_number_element = number_element + 1;
    number_element += 2;
    jump_stack_pointer++;
    not_nil = left_part_elements[current_left_part_element].v_variable;
    if (left_part_elements[current_left_part_element].specifier.info.codef == NULL)
        return true;
    generate_operator_e_v(n_prespc, n_prv);
    generate_operator_l(n_respc, left_part_elements[current_left_part_element].specifier.info.codef);
    return false;
}

//    check ortogonality of this sentence against left part
static bool ortogonality(uint8_t on1, uint8_t on2)
{
    uint8_t on = on1;
    uint8_t i;
    while (true)
    {
        on++;
        if (on == on2)
            break;
        if (left_part_elements[on].type <= 3)
            continue;
        i = left_part_elements[on].variable_index;
        if (variables[i].last_left_part_element != 0)
            continue;
        variables[i].rem--;
    }
    bool res = true;
    on = on1;
    while (true)
    {
        on++;
        if (on == on2)
            break;
        if (left_part_elements[on].type <= 3)
            continue;
        i = left_part_elements[on].variable_index;
        if (variables[i].last_left_part_element != 0)
            continue;
        if (variables[i].rem == 0)
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
        if (left_part_elements[on].type <= 3)
            continue;
        i = left_part_elements[on].variable_index;
        if (variables[i].last_left_part_element != 0)
            continue;
        variables[i].rem++;
    }
    return res;
}

static void generate_operator_e_v(uint8_t operator_e, uint8_t operator_v)
{
    if (not_nil)
        jbyte(operator_e);
    else
        jbyte(operator_v);
}

//----------  end of file CCST.C  -------------------
