// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-11-21
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file compile_sentence.c  ----------
//   Compilation of a sentence: left and right part
//-----------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "refalab.h"
#include "compile_sentence.h"
#include "print_errors.h"
#include "identifiers.h"
#include "generate_operators.h"
#include "macrocode.h"
#include "compiler.h"
#include "compile_output.h"

#define PRINT_ERROR_303 \
    print_error_two_strings(303, "Differents for variable ", variables[variable_index].identifier, variables[variable_index].identifier_length)

#define PRINT_ERROR_406 \
    print_error_two_strings(406, "In left part missing variable ", current_sentence_element.identifier, current_sentence_element.identifier_length)

// States of the compiler automaton. The compilation of a sentence is a state machine
// that works in three stages:
//   GET_LPE .. NEXT_LPE - reading and checking of the left part (the pattern);
//   RCG .. RCGFIN       - generation of the matching code: first the rigid elements,
//                         then the e-variables, which need a search;
//   GET_RPE ..          - reading of the right part (the result) and generation of its code.
// OSH300, RP_OSH300 and ERROR are the error exits.
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
    ODECOMP,
    OCLASS,
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

// Kind of a variable in the variable table: NEW - not met yet, S - a symbol,
// W - a term (a symbol or a bracketed expression), E - an expression (also v-variables).
typedef enum variable_types
{
    NEW,
    S,
    W,
    E
} T_VARIABLE_TYPES;

// Elements of the left part in the order of reading. Element 0 and the last element
// (the '=' sign, number left_part_elements_count) are the ends of the pattern.
// left_number_element/right_number_element: numbers of the element's boundaries in the
//   matching stack of the generated code (operands of the generated statements);
// pair_bracket: the index of the matching bracket; next_variable: the previous
//   occurrence of the same variable; specifier: the type specifier of a variable;
// eoe_mark, jump_stack_pointer, used: attributes used by the search for e-variables.
static struct
{ // left part buffer elements
    uint16_t left_number_element, right_number_element;
    T_SENTENCE_ELEMENT_TYPES type;
    uint8_t variable_index;
    T_LINKTI code;
    uint8_t next_variable;
    uint8_t pair_bracket;
    T_LINKTI specifier;
    bool v_variable;
    bool eoe_mark;
    uint8_t jump_stack_pointer;
    bool used;
} left_part_elements[UINT8_MAX];

// Variable table. last_left_part_element: the last occurrence of the variable that
// has been generated (0 - none yet, i.e. the variable is still free); remains: the number
// of the occurrences left to generate; main_right_number_element: number of the first
// occurrence, which later occurrences are compared with.
static struct
{ // variable table elements
    uint8_t last_left_part_element;
    T_VARIABLE_TYPES type;
    uint16_t main_right_number_element;
    uint8_t remains;
    char identifier[MAX_IDENTIFIER_LENGTH];
    uint8_t identifier_length;
    bool v_variable;
} variables[UINT8_MAX];

// hole list
static struct
{
    uint8_t next_hole;
    uint8_t left_board, right_board;
} hole_list[130];

// Classes of independent holes (see decompose_holes). When the holes of the pattern split into
// several classes that have no free variable in common, the classes are compiled one after another.
// The holes of the classes that wait for their turn are kept in class_holes (a stack, the last
// class pushed is on the top), class_stack describes these classes: where its holes start in
// class_holes, how many they are, and the value of jump_stack_pointer when the classes were set aside.
static struct
{
    uint8_t left_board, right_board;
} class_holes[130];
static uint8_t class_holes_count;
static struct
{
    uint8_t first_hole, hole_count;
    uint8_t jump_stack_pointer;
} class_stack[130];
static uint8_t class_stack_count;

// Work structures and counters of the compiler.
// The left part is a list of holes: a hole is a segment of the pattern between two
// boards (left_board, right_board) that is not matched yet. Brackets split a hole in two.
static T_LINKTI xncode;           // work structure
static T_LINKTI function_pointer; // work pointer

static uint8_t current_left_part_element, current_left_board, current_right_board; // left part element pointers
// Limits of the search for the best transplantation (see try_transplant).
#define SENTENCE_SEARCH_BUDGET 5000  // nodes of transplantation search for one sentence
#define DECISION_SEARCH_BUDGET 2000  // nodes of transplantation search for one decision
static uint8_t left_part_elements_count;                                           // left part element count
static int32_t search_budget;                                                    // nodes left for transplantation search of the sentence
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
static char symbols_buffer[UINT8_MAX]; // buffer for generating of the "text" statement
static uint8_t current_hole;           // current hole number
static uint8_t free_segment_hole_list; // free segment number in the hole  list
static uint8_t next_hole;              // next hole number
static uint8_t hole_x, hole_y;         // hole numbers (under enter in brackets)

// Forward declarations of the helper functions.
static bool search_variable(bool left_part);
static void generate_boards_stoping_brackets(void);
static bool lsg_p(void);
static bool rsg_p(void);
static void generate_operator_e_v(uint8_t operator_e, uint8_t operator_v);
static bool ortogonality(uint8_t on1, uint8_t on2);
static bool try_transplant(void);
static void decompose_holes(void);
static void load_class_holes(const uint8_t *left_boards, const uint8_t *right_boards, uint8_t count);

#if defined mdebug
// ----------  debug trace helpers  ----------
static const char *const state_names[] = {
    "GET_LPE",
    "LPE0",
    "LPE1",
    "LPE2",
    "LPE3",
    "LPE4",
    "LPE5",
    "LPE6",
    "LPE7",
    "LPE8",
    "LPE9",
    "LPE10",
    "NEXT_LPE",
    "OSH300",
    "RCG",
    "RCGL",
    "LSW1",
    "LTXT",
    "LTXT1",
    "LTXT2",
    "LTXT3",
    "LSCO",
    "LSW2",
    "LBCE",
    "LBNIL",
    "GEN_LB",
    "LB1",
    "LSW3",
    "LSW4",
    "LSMD",
    "L1",
    "LSW5",
    "LED",
    "LEMD",
    "L2",
    "LSW6",
    "RCGR",
    "RSW1",
    "RTXT",
    "RTXT1",
    "RTXT2",
    "RTXT3",
    "RSCO",
    "RSW2",
    "RBCE",
    "RBNIL",
    "GEN_RB",
    "RB1",
    "RSW3",
    "RSW4",
    "RSMD",
    "R1",
    "RSW5",
    "RED",
    "REMD",
    "R2",
    "RSW6",
    "NIL",
    "CE",
    "CE1",
    "CE2",
    "IMPASSE",
    "HSCH",
    "NHOLE",
    "NHOLE1",
    "RIGID",
    "ODECOMP",
    "OCLASS",
    "OE",
    "OE0",
    "OERMAX",
    "RMAX",
    "REM",
    "OELMAX",
    "LMAX",
    "LEM",
    "OE1",
    "LOE",
    "LSG",
    "LE_CASE",
    "LESW1",
    "LESW2",
    "LESW3",
    "LESW4",
    "LESW5",
    "LESW6",
    "LE",
    "ROE",
    "RSG",
    "RE_CASE",
    "RESW1",
    "RESW2",
    "RESW3",
    "RESW4",
    "RESW5",
    "RESW6",
    "RE",
    "RCGFIN",
    "GET_RPE",
    "SW_RPE",
    "RPE0",
    "RPE1",
    "TEXT",
    "TEXT1",
    "RPE2",
    "RPE3",
    "RPE4",
    "RPE5",
    "RPE6",
    "RPE7",
    "RPE8",
    "RPE9",
    "RPE10",
    "RP_OSH300",
    "ERROR"
};

static T_STATES debug_trace_state(T_STATES state)
{
    fprintf(stderr, "compile_sentence: state %s\n", state_names[state]);
    return state;
}

static const char *debug_element_type_name(T_SENTENCE_ELEMENT_TYPES type)
{
    switch (type)
    {
    case NONE:
        return "NONE";
    case SC:
        return "SC";
    case LB:
        return "LB";
    case RB:
        return "RB";
    case S_V:
        return "S_V";
    case W_V:
        return "W_V";
    case E_V:
        return "E_V";
    case K:
        return "K";
    case DOT:
        return "DOT";
    case EQ:
        return "EQ";
    case END:
        return "END";
    }
    return "?";
}

static const char *debug_variable_type_name(T_VARIABLE_TYPES type)
{
    switch (type)
    {
    case NEW:
        return "NEW";
    case S:
        return "S";
    case W:
        return "W";
    case E:
        return "E";
    }
    return "?";
}

// prints a sentence element in readable form (without the new line)
static void debug_print_sentence_element(const T_SENTENCE_ELEMENT *element)
{
    fprintf(stderr, "%s", debug_element_type_name(element->type));
    switch (element->type)
    {
    case SC:
        fprintf(stderr, " ");
        debug_print_symbol(&element->code);
        break;
    case S_V:
    case W_V:
    case E_V:
        fprintf(stderr, " %.*s%s", element->identifier_length, element->identifier, element->v_variable ? " (v-variable)" : "");
        if (element->specifier.info.codef != NULL)
        {
            fprintf(stderr, " specifier=");
            debug_print_symbol(&element->specifier);
        }
        break;
    default:
        break;
    }
    return;
}

// prints the text chain collected in symbols_buffer
static void debug_print_text(const char *where)
{
    fprintf(stderr, "compile_sentence: %s text of %u symbols: \"", where, symbols_count);
    for (uint8_t k = 0; k < symbols_count; k++)
    {
        const uint8_t ch = (uint8_t)symbols_buffer[k];
        if (ch >= 0x20 && ch < 0x7F)
            fputc(ch, stderr);
        else
            fprintf(stderr, "\\x%02X", ch);
    }
    fprintf(stderr, "\"\n");
    return;
}

// prints the text chain made of the left part elements: the chain begins at the element "first" and goes by "step"
static void debug_print_left_text(const char *where, uint8_t first, int step)
{
    fprintf(stderr, "compile_sentence: %s text of %u symbols: \"", where, symbols_count);
    int index = first;
    for (uint8_t k = 0; k < symbols_count; k++, index += step)
    {
        const uint8_t ch = (uint8_t)left_part_elements[index].code.info.infoc;
        if (ch >= 0x20 && ch < 0x7F)
            fputc(ch, stderr);
        else
            fprintf(stderr, "\\x%02X", ch);
    }
    fprintf(stderr, "\"\n");
    return;
}

static void debug_dump_left_part(void)
{
    fprintf(stderr, "compile_sentence: left part table (%u elements):\n", left_part_elements_count);
    for (uint8_t i = 0; i <= left_part_elements_count; i++)
    {
        const char *type_name = debug_element_type_name(left_part_elements[i].type);
        fprintf(stderr, "    [%3u] %-4s", i, type_name);
        if (left_part_elements[i].type == S_V || left_part_elements[i].type == W_V || left_part_elements[i].type == E_V)
            fprintf(stderr, " %.*s", variables[left_part_elements[i].variable_index].identifier_length, variables[left_part_elements[i].variable_index].identifier);
        else if (left_part_elements[i].type == SC)
        {
            fprintf(stderr, " ");
            debug_print_symbol(&left_part_elements[i].code);
        }
        if (left_part_elements[i].pair_bracket != 0)
            fprintf(stderr, " pair=%u", left_part_elements[i].pair_bracket);
        if (left_part_elements[i].v_variable)
            fprintf(stderr, " v");
        fprintf(stderr, "\n");
    }
    return;
}

static void debug_dump_variables(void)
{
    fprintf(stderr, "compile_sentence: variable table (%u variables):\n", variables_count);
    for (uint8_t v = 0; v < variables_count; v++)
    {
        fprintf(stderr, "    [%3u] %-3s %.*s remains=%u last_left_part_element=%u%s\n", v, debug_variable_type_name(variables[v].type), variables[v].identifier_length, variables[v].identifier, variables[v].remains, variables[v].last_left_part_element, variables[v].v_variable ? " v" : "");
    }
    return;
}

static void debug_dump_holes(void)
{
    fprintf(stderr, "compile_sentence: hole list (current_hole=%u, free_segment=%u):\n", current_hole, free_segment_hole_list);
    for (uint8_t h = 1; h != 0 && h != 128; h = hole_list[h].next_hole)
        fprintf(stderr, "    hole %3u: boards (%u, %u) next=%u\n", h, hole_list[h].left_board, hole_list[h].right_board, hole_list[h].next_hole);
    return;
}
// -------------------------------------------
#endif

// read left part
// and full array X
// Compiles one sentence, from the first element of its left part to its end.
// direction is the direction of matching: true - from left to right, false - from right to left.
// The sentence is not written directly: its code goes to the function pool.
// After an error the rest of the sentence is skipped and the compilation goes on.
void compile_sentence(bool direction)
// direction;     matching feature :left to right or otherwise
{
    variables_count = 0;
    current_left_part_element = 0;
    last_bracket = 0;
    number_element = 0;
    T_STATES state = GET_LPE;
#if defined mdebug
    fprintf(stderr, "compile_sentence: begin direction=%s\n", direction ? "LEFT (left to right)" : "RIGHT (right to left)");
#endif
    while (true)
#if defined mdebug
        switch (debug_trace_state(state))
#else
        switch (state)
#endif
        {
        // ---------- reading of the left part ----------
        // Every element is stored in left_part_elements, and the state is chosen by its type.
        // Variables are entered in the variable table, brackets are linked in pairs.
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
            left_part_elements[current_left_part_element].used = false;
#if defined mdebug
            {
                size_t line, column;
                get_location(&line, &column, current_sentence_element.cursor_number);
                fprintf(stderr, "compile_sentence: left part element [%u] line=%zu column=%zu: ", current_left_part_element, line, column);
                debug_print_sentence_element(&current_sentence_element);
                fprintf(stderr, "\n");
            }
#endif
            switch (left_part_elements[current_left_part_element].type)
            {
            case NONE:
                state = LPE0;
                break;
            case SC:
                state = LPE1;
                break;
            case LB:
                state = LPE2;
                break;
            case RB:
                state = LPE3;
                break;
            case S_V:
                state = LPE4;
                break;
            case W_V:
                state = LPE5;
                break;
            case E_V:
                state = LPE6;
                break;
            case K:
                state = LPE7;
                break;
            case DOT:
                state = LPE8;
                break;
            case EQ:
                state = LPE9;
                break;
            case END:
                state = LPE10;
                break;
            default:
                state = OSH300;
            }
            break;
        // The scanner found an error.
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
                scanner.last_error_cursor = current_sentence_element.cursor_number;
                print_error_string(302, "Too many ')' in left part");
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
            search_variable(true);
            left_part_elements[current_left_part_element].variable_index = variable_index;
            switch (variables[variable_index].type)
            {
            case NEW:
                variables[variable_index].type = S; // yet isn't faced
                break;
            case S:
                ++variables[variable_index].remains; // next position
                break;
            default: // invalid type pointer
                scanner.last_error_cursor = current_sentence_element.cursor_number;
                PRINT_ERROR_303;
                current_left_part_element--;
            }
            number_element++;
            state = NEXT_LPE;
            break;
        case LPE5:
            // w-varyable
            search_variable(true);
            left_part_elements[current_left_part_element].variable_index = variable_index;
            switch (variables[variable_index].type)
            {
            case NEW:
                variables[variable_index].type = W; // yet isn't faced
                break;
            case W:
                ++variables[variable_index].remains; // next position
                break;
            default: // invalid type pointer
                scanner.last_error_cursor = current_sentence_element.cursor_number;
                PRINT_ERROR_303;
                current_left_part_element--;
            }
            number_element += 2;
            state = NEXT_LPE;
            break;
        case LPE6:
            // e- or v-varyable
            search_variable(true);
            left_part_elements[current_left_part_element].variable_index = variable_index;
            if (variables[variable_index].type == NEW) // yet is't faced
                variables[variable_index].type = E;
            else if (variables[variable_index].type == E && variables[variable_index].v_variable == current_sentence_element.v_variable)
                ++variables[variable_index].remains;
            else // invalid type pointer
            {
                scanner.last_error_cursor = current_sentence_element.cursor_number;
                PRINT_ERROR_303;
                current_left_part_element--;
            };
            number_element += 2;
            state = NEXT_LPE;
            break;
        case LPE7:
            // sign '<'
            scanner.last_error_cursor = current_sentence_element.cursor_number;
            print_error_string(306, "Sign '<' in left part");
            current_left_part_element--;
            state = NEXT_LPE;
            break;
        case LPE8:
            // sign '>'
            scanner.last_error_cursor = current_sentence_element.cursor_number;
            print_error_string(307, "Sign '>' in left part");
            current_left_part_element--;
            state = NEXT_LPE;
            break;
        // The end of the left part ('=' was met): the brackets must be balanced.
        case LPE9:
            // left part end
            if (last_bracket == 0)
            {
                state = RCG;
                break;
            }
            scanner.last_error_cursor = current_sentence_element.cursor_number;
            print_error_string(301, "Too many '(' in left part");
            state = OSH300;
            break;
        // The sentence ended before the '=' sign.
        case LPE10:
            // sentence end
            scanner.last_error_cursor = current_sentence_element.cursor_number;
            print_error_string(304, "Under left part default sign '=' ");
            compile_output_switch_to_macrocode();
            function_definition();
            return;
        // The sentence is limited to 252 numbered elements.
        case NEXT_LPE:
            // end of element processing
            if (number_element <= 252)
            {
                state = GET_LPE;
                break;
            }
            scanner.last_error_cursor = current_sentence_element.cursor_number;
            print_error_string(305, "Very large left part");
            state = OSH300;
            break;
        // Error exit: the sentence is replaced by an empty one and the rest is skipped.
        case OSH300:
            compile_output_switch_to_macrocode();
            function_definition();
            state = RP_OSH300;
            break;
        //--------------------------------------------
        //         left part compilation
        //--------------------------------------------
        // ---------- generation of the left part code ----------
        // The pattern is matched from its both ends towards the middle. First the rigid elements
        // (symbols, brackets, variables that are already bound) are projected, as they are
        // checked without any search. direction selects the end to start from.
        case RCG:
            function_definition();
            left_part_elements_count = current_left_part_element;
#if defined mdebug
            debug_dump_left_part();
            debug_dump_variables();
#endif
            search_budget = SENTENCE_SEARCH_BUDGET;
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
            class_holes_count = 0;
            class_stack_count = 0;
            if (direction)
            {
                state = RCGL;
                break;
            }
            state = RCGR;
            break;
        //    hard element projection from left part of current hole
        // Rigid element projection from the left board of the current hole.
        case RCGL:
            current_left_part_element = current_left_board + 1;
            if (current_left_part_element == current_right_board)
            {
                state = NIL;
                break;
            }
            switch (left_part_elements[current_left_part_element].type)
            {
            case SC:
                state = LSW1;
                break;
            case LB:
                state = LSW2;
                break;
            case RB:
                state = LSW3;
                break;
            case S_V:
                state = LSW4;
                break;
            case W_V:
                state = LSW5;
                break;
            case E_V:
                state = LSW6;
                break;
            default:
                state = OSH300;
            };
            break;
        // A constant symbol: a text of several characters is collected into one n_ltxt statement.
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
            if (current_left_part_element == current_right_board || left_part_elements[current_left_part_element].type != SC || left_part_elements[current_left_part_element].code.tag != TAGO)
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
#if defined mdebug
            debug_print_left_text("left part (left to right)", current_left_board + 1, 1);
#endif
            state = LTXT3;
            break;
        case LTXT3:
            current_left_part_element++;
            compile_output_byte((uint8_t)left_part_elements[current_left_part_element].code.info.infoc);
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
        // A left bracket: an empty pair of brackets and a pair holding one free e-variable
        // get special statements, any other pair opens a new hole.
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
            if (left_part_elements[current_left_part_element].type != E_V)
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
            generate_operator(n_lbce);
            variables[variable_index].main_right_number_element = number_element + 1;
            left_part_elements[current_left_part_element].next_variable = variables[variable_index].last_left_part_element;
            variables[variable_index].last_left_part_element = current_left_part_element;
            variables[variable_index].remains--;
            if (left_part_elements[current_left_part_element].v_variable)
                generate_operator(n_nnil);
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
            generate_operator(n_lbnil);
            left_part_elements[current_left_board].right_number_element = number_element;
            left_part_elements[current_left_board].left_number_element = left_part_elements[current_left_board].right_number_element;
            current_left_board = current_left_part_element;
            left_part_elements[current_left_board].right_number_element = number_element + 1;
            left_part_elements[current_left_board].left_number_element = left_part_elements[current_left_board].right_number_element;
            number_element += 2;
            state = RCGL;
            break;
        // A bracket pair that opens a new hole: the hole inside the brackets is separated
        // from the rest of the pattern.
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
        // A right bracket cannot be at the left board of a hole.
        case LSW3:
            state = ERROR;
            break;
        // An s-variable: the first occurrence is read (n_ls), later ones are compared (n_lsd).
        case LSW4:
            //    s-variable
            variable_index = left_part_elements[current_left_part_element].variable_index;
            if (variables[variable_index].last_left_part_element != 0)
                generate_operator_n(n_lsd, (uint8_t)variables[variable_index].main_right_number_element);
            else
            {
                generate_operator(n_ls);
                variables[variable_index].main_right_number_element = number_element;
            };
            state = LSMD;
            break;
        case LSMD:
            left_part_elements[current_left_part_element].next_variable = variables[variable_index].last_left_part_element;
            variables[variable_index].last_left_part_element = current_left_part_element;
            variables[variable_index].remains--;
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
        // A w-variable: the first occurrence is read (n_lw), later ones are compared (n_led).
        case LSW5:
            //   w-variable
            variable_index = left_part_elements[current_left_part_element].variable_index;
            if (variables[variable_index].last_left_part_element != 0)
            {
                state = LED;
                break;
            }
            generate_operator(n_lw);
            variables[variable_index].main_right_number_element = number_element + 1;
            left_part_elements[current_left_part_element].next_variable = variables[variable_index].last_left_part_element;
            variables[variable_index].last_left_part_element = current_left_part_element;
            variables[variable_index].remains--;
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
            variables[variable_index].remains--;
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
        // An e-variable: an already bound one is compared; a free one needs a search,
        // so it is left for later (or is closed when it is alone in its hole).
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
        // Rigid element projection from the right board of the current hole
        // (the same as RCGL, mirrored).
        case RCGR:
            current_left_part_element = current_right_board - 1;
            if (current_left_part_element == current_left_board)
            {
                state = NIL;
                break;
            }
            switch (left_part_elements[current_left_part_element].type)
            {
            case SC:
                state = RSW1;
                break;
            case LB:
                state = RSW2;
                break;
            case RB:
                state = RSW3;
                break;
            case S_V:
                state = RSW4;
                break;
            case W_V:
                state = RSW5;
                break;
            case E_V:
                state = RSW6;
                break;
            default:
                state = OSH300;
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
            if (current_left_part_element == current_left_board || left_part_elements[current_left_part_element].type != SC || left_part_elements[current_left_part_element].code.tag != TAGO)
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
#if defined mdebug
            debug_print_left_text("left part (right to left)", current_right_board - 1, -1);
#endif
            state = RTXT3;
            break;
        case RTXT3:
            current_left_part_element--;
            compile_output_byte((uint8_t)left_part_elements[current_left_part_element].code.info.infoc);
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
            if (left_part_elements[current_left_part_element].type != E_V)
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
            generate_operator(n_rbce);
            variables[variable_index].main_right_number_element = number_element + 1;
            left_part_elements[current_left_part_element].next_variable = variables[variable_index].last_left_part_element;
            variables[variable_index].last_left_part_element = current_left_part_element;
            variables[variable_index].remains--;
            if (left_part_elements[current_left_part_element].v_variable)
                generate_operator(n_nnil);
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
            generate_operator(n_rbnil);
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
                generate_operator(n_rs);
                variables[variable_index].main_right_number_element = number_element;
            };
            state = RSMD;
            break;
        case RSMD:
            left_part_elements[current_left_part_element].next_variable = variables[variable_index].last_left_part_element;
            variables[variable_index].last_left_part_element = current_left_part_element;
            variables[variable_index].remains--;
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
            generate_operator(n_rw);
            variables[variable_index].main_right_number_element = number_element + 1;
            left_part_elements[current_left_part_element].next_variable = variables[variable_index].last_left_part_element;
            variables[variable_index].last_left_part_element = current_left_part_element;
            variables[variable_index].remains--;
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
            variables[variable_index].remains--;
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
        // The hole is empty: n_nil checks it and the hole is removed from the list.
        case NIL:
            //     empty hole
            generate_operator(n_nil);
            next_hole = hole_list[current_hole].next_hole;
            hole_list[current_hole].next_hole = hole_list[next_hole].next_hole;
            hole_list[current_hole].left_board = hole_list[next_hole].left_board;
            hole_list[current_hole].right_board = hole_list[next_hole].right_board;
            current_hole = next_hole;
            state = IMPASSE;
            break;
        // Closed e-variable: the only element of its hole, it takes the whole contents of the hole.
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
            generate_operator(n_ce);
            left_part_elements[current_left_part_element].next_variable = variables[variable_index].last_left_part_element;
            variables[variable_index].last_left_part_element = current_left_part_element;
            variables[variable_index].remains--;
            if (left_part_elements[current_left_part_element].v_variable)
                generate_operator(n_nnil);
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
        // No rigid element can be projected from this hole any more.
        case IMPASSE:
            stoped_bracket_flag = 0;
            hole_x = current_hole;
            state = HSCH;
            break;
        //          Search of hole with hard
        //          elements on its boards.
        //          If it not exist than project
        //          e-variable from first hole.
        // Search for a hole with rigid elements on its boards;
        // when there is none, an e-variable has to be projected from the first hole.
        case HSCH:
            hole_list[current_hole].left_board = current_left_board;
            hole_list[current_hole].right_board = current_right_board;
            current_hole = 1;
#if defined mdebug
            debug_dump_holes();
#endif
            if (hole_list[current_hole].next_hole == 0)
            {
                // No holes left: the next class of independent holes is taken, if there is one.
                state = (class_stack_count > 0) ? OCLASS : RCGFIN;
                break;
            }
            state = NHOLE;
            break;
        // Takes the next hole: a hole whose boards still carry rigid elements goes on being
        // projected (RIGID); an empty hole, and a hole with a single e-variable, are closed at once.
        case NHOLE:
            current_left_board = hole_list[current_hole].left_board;
            current_right_board = hole_list[current_hole].right_board;
#if defined mdebug
            fprintf(stderr, "compile_sentence: hole %u boards (%u, %u)\n", current_hole, current_left_board, current_right_board);
#endif
            current_left_part_element = current_left_board + 1;
            if (current_left_part_element == current_right_board)
            {
                generate_boards_stoping_brackets();
                state = NIL;
                break;
            }
            if (left_part_elements[current_left_part_element].type != E_V)
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
                if (variables[variable_index].remains == 1)
                {
                    state = NHOLE1;
                    break;
                }
                generate_boards_stoping_brackets();
                state = CE1;
                break;
            }
            current_left_part_element = current_right_board - 1;
            if (left_part_elements[current_left_part_element].type != E_V)
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
        // The hole is left for the e-variable stage; after the last hole the search starts.
        case NHOLE1:
            current_hole = hole_list[current_hole].next_hole;
            if (hole_list[current_hole].next_hole == 0)
            {
                state = ODECOMP;
                break;
            }
            state = NHOLE;
            break;
        // Every hole has an open e-variable on both boards: the holes are split into classes of
        // independent holes, the first class is compiled now and the others wait.
        case ODECOMP:
            decompose_holes();
            state = OE;
            break;
        // The class of independent holes is compiled: the points of the searches that are left
        // are removed (the other classes do not depend on them), then the next class is taken.
        case OCLASS:
        {
            const uint8_t top = class_stack_count - 1;
            const uint8_t diff = jump_stack_pointer - class_stack[top].jump_stack_pointer;
            if (diff == 1)
                generate_operator(n_eoei);
            else if (diff > 1)
                generate_operator_n(n_eoe, diff);
            jump_stack_pointer = class_stack[top].jump_stack_pointer;
            uint8_t left_boards[130], right_boards[130];
            for (uint8_t i = 0; i < class_stack[top].hole_count; i++)
            {
                left_boards[i] = class_holes[class_stack[top].first_hole + i].left_board;
                right_boards[i] = class_holes[class_stack[top].first_hole + i].right_board;
            }
            class_holes_count = class_stack[top].first_hole;
            const uint8_t count = class_stack[top].hole_count;
            class_stack_count--;
            load_class_holes(left_boards, right_boards, count);
            // The boards of the generated code are those of the last hole of the finished class.
            hole_x = 0;
            state = NHOLE;
            break;
        }
        // A rigid element is on the boards of the hole: the projection goes on.
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
        // ---------- opened e-variables ----------
        // No hole has rigid elements left, so an e-variable has to be opened: the first hole is
        // taken, the pending end-of-expression mark (n_eoe/n_eoei) is generated when an earlier
        // search is complete, and the variable at the board is processed depending on the direction.
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
                    generate_operator(n_eoei);
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
        // Checks whether the e-variable with a specifier can be expanded to the maximal length
        // (n_lmax/n_rmax): only if it is the last occurrence and no other free e-variable is left in the hole.
        case OE0:
            variable_index = left_part_elements[current_left_part_element].variable_index;
            if (variables[variable_index].last_left_part_element != 0 || variables[variable_index].remains != 1)
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
            if (left_part_elements[current_left_part_element].type != E_V || left_part_elements[current_left_part_element].v_variable)
            {
                state = OE1;
                break;
            }
            state = OE0;
            break;
        // The hole holds only one free e-variable besides this one: the maximal right projection.
        case OERMAX:
            current_left_part_element = current_right_board - 1;
            variable_index = left_part_elements[current_left_part_element].variable_index;
            state = RMAX;
            break;
        // The e-variable takes the maximal part to the right (n_rmax); its specifier is
        // checked by this statement, so it is not checked again.
        case RMAX:
            generate_operator_l(n_rmax, left_part_elements[current_left_part_element].specifier.info.codef);
            if (left_part_elements[current_left_part_element].v_variable)
                generate_operator(n_nnil);
            left_part_elements[current_left_part_element].specifier.info.codef = NULL;
            state = REM;
            break;
        case REM:
            variables[variable_index].main_right_number_element = number_element + 1;
            state = REMD;
            break;
        // The same for the maximal left projection.
        case OELMAX:
            current_left_part_element = current_left_board + 1;
            variable_index = left_part_elements[current_left_part_element].variable_index;
            state = LMAX;
            break;
        // The e-variable takes the maximal part to the left (n_lmax).
        case LMAX:
            generate_operator_l(n_lmax, left_part_elements[current_left_part_element].specifier.info.codef);
            if (left_part_elements[current_left_part_element].v_variable)
                generate_operator(n_nnil);
            left_part_elements[current_left_part_element].specifier.info.codef = NULL;
            state = LEM;
            break;
        case LEM:
            variables[variable_index].main_right_number_element = number_element + 1;
            state = LEMD;
            break;
        // The general case: the e-variable is expanded step by step (a search with backtracking).
        case OE1:
            if (direction)
            {
                state = LOE;
                break;
            }
            state = ROE;
            break;
        // Expansion from the left board.
        case LOE:
            current_left_part_element = current_left_board + 1;
            temp_variable_index = left_part_elements[current_left_part_element].variable_index;
            state = LSG;
            break;
        //         attempt to extract left support group
        // Attempt to extract the left support group: elements right after the e-variable
        // that can be found faster than by stepping (a symbol, a bracket, a bound variable).
        case LSG:
            if (lsg_p())
            {
                state = LE_CASE;
                break;
            }
            state = RCGL;
            break;
        // The element after the e-variable decides the kind of the search statements.
        case LE_CASE:
            current_left_part_element = current_left_board + 1;
            switch (left_part_elements[current_left_part_element].type)
            {
            case SC:
                state = LESW1;
                break;
            case LB:
                state = LESW2;
                break;
            case RB:
                state = LESW3;
                break;
            case S_V:
                state = LESW4;
                break;
            case W_V:
                state = LESW5;
                break;
            case E_V:
                state = LESW6;
                break;
            default:
                state = OSH300;
            };
            break;
        // e 'a' ...: a search for the symbol (n_lsrch) when the search is the last one,
        // otherwise an expansion that stops at the symbol (n_plesc, n_lesc).
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
        // e ( ... ) ...: an expansion that stops at a bracket (n_pleb, n_leb); the bracket opens a hole.
        case LESW2:
            //   ei ( . . . ) . . .
            generate_operator_e_v(n_pleb, n_plvb);
            generate_operator(n_leb);
            stoped_bracket_flag = 0;
            state = LB1;
            break;
        case LESW3:
            state = ERROR;
            break;
        // e s ...: an expansion that stops at a symbol equal to the bound s-variable (n_lesd);
        // a free s-variable cannot be used for the stop, so a plain expansion is generated.
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
        // e w ... and e e ...: a plain expansion by one term.
        case LESW5:
        case LESW6:
            //  ei . . .
            state = LE;
            break;
        // Plain expansion of the e-variable from the left: n_ple/n_plv saves the point to return to
        // (v-variables must not be empty), n_le makes one step.
        case LE:
            generate_operator_e_v(n_ple, n_plv);
            generate_operator(n_le);
            state = RCGL;
            break;
        // Expansion from the right board (the mirror image of LOE).
        case ROE:
            current_left_part_element = current_right_board - 1;
            temp_variable_index = left_part_elements[current_left_part_element].variable_index;
            state = RSG;
            break;
        //                 attempt to extract right support group
        // Attempt to extract the right support group (the mirror image of LSG).
        case RSG:
            if (rsg_p())
            {
                state = RE_CASE;
                break;
            }
            state = RCGR;
            break;
        // The element before the e-variable decides the kind of the search statements.
        case RE_CASE:
            current_left_part_element = current_right_board - 1;
            switch (left_part_elements[current_left_part_element].type)
            {
            case SC:
                state = RESW1;
                break;
            case LB:
                state = RESW2;
                break;
            case RB:
                state = RESW3;
                break;
            case S_V:
                state = RESW4;
                break;
            case W_V:
                state = RESW5;
                break;
            case E_V:
                state = RESW6;
                break;
            default:
                state = OSH300;
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
            generate_operator(n_reb);
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
            generate_operator(n_re);
            state = RCGR;
            break;
        //                 identification end
        // The left part is compiled: n_eor ends it and the output switches to the right part.
        // A counter of bracket levels is initialised for the right part.
        case RCGFIN:
#if defined mdebug
            fprintf(stderr, "compile_sentence: left part compiled, jump_stack_pointer=%u, search_budget=%d\n", jump_stack_pointer, (int)search_budget);
            debug_dump_variables();
#endif
            generate_operator(n_eor);
            compile_output_switch_to_right_part();
            //--------------------------------------------
            //         right part compilation
            //--------------------------------------------
            brackets_k_level = 1;
            brackets_count[brackets_k_level] = 0;
            state = GET_RPE;
            break;
        //  read next element of right part
        // ---------- right part ----------
        // Every element is read and the code that builds the result is generated.
        case GET_RPE:
            scan_sentence_element();
            state = SW_RPE;
            break;
        // The element type selects the handler. Before an element that can be a part of a piece
        // of the left part, a transplantation is tried (see try_transplant): when it succeeds, the
        // elements of the piece are already consumed and the next element is read.
        case SW_RPE:
#if defined mdebug
            fprintf(stderr, "compile_sentence: right part element: ");
            debug_print_sentence_element(&current_sentence_element);
            fprintf(stderr, "\n");
#endif
            switch (current_sentence_element.type)
            {
            case NONE:
                state = RPE0;
                break;
            case SC:
                if (try_transplant())
                    state = GET_RPE;
                else
                    state = RPE1;
                break;
            case LB:
                if (try_transplant())
                    state = GET_RPE;
                else
                    state = RPE2;
                break;
            case RB:
                state = RPE3;
                break;
            case S_V:
                if (try_transplant())
                    state = GET_RPE;
                else
                    state = RPE4;
                break;
            case W_V:
                if (try_transplant())
                    state = GET_RPE;
                else
                    state = RPE5;
                break;
            case E_V:
                if (try_transplant())
                    state = GET_RPE;
                else
                    state = RPE6;
                break;
            case K:
                state = RPE7;
                break;
            case DOT:
                state = RPE8;
                break;
            case EQ:
                state = RPE9;
                break;
            case END:
                state = RPE10;
                break;
            default:
                state = RP_OSH300;
            };
            break;
        // The scanner found an error.
        case RPE0:
            // scanner error
            state = RP_OSH300;
            break;
        // A constant symbol: characters are collected into one n_text statement
        // (n_nso for a single character), any other symbol is generated by n_ns.
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
            symbols_buffer[symbols_count] = current_sentence_element.code.info.infoc;
            symbols_count++;
            scan_sentence_element();
            if (symbols_count < UINT8_MAX && current_sentence_element.type == SC && current_sentence_element.code.tag == TAGO)
                break;
            if (symbols_count == 1)
                generate_operator_n(n_nso, (uint8_t)symbols_buffer[0]);
            else
            {
                generate_operator_n(n_text, symbols_count);
#if defined mdebug
                debug_print_text("right part");
#endif
                for (uint8_t k = 0; k < symbols_count; k++)
                    compile_output_byte((uint8_t)symbols_buffer[k]);
            };
            state = SW_RPE;
            break;
        // A left bracket: "()" is generated as one statement (n_blr); a bracket followed by a
        // function label as n_blf; any other bracket as n_bl. The counter of the open
        // brackets of the current call level is increased.
        case RPE2:
            // left bracket
            scan_sentence_element();
            if (current_sentence_element.type == RB)
            {
                generate_operator(n_blr);
                state = GET_RPE;
                break;
            }
            brackets_count[brackets_k_level]++;
            if (current_sentence_element.type == SC && current_sentence_element.code.tag == TAGF)
            {
                function_pointer.info.codef = current_sentence_element.code.info.codef;
                generate_operator_l(n_blf, function_pointer.info.codef);
                state = GET_RPE;
                break;
            }
            generate_operator(n_bl);
            state = SW_RPE;
            break;
        // A right bracket: n_br; it must have an open bracket at the current call level (402).
        case RPE3:
            // right bracket
            generate_operator(n_br);
            if (brackets_count[brackets_k_level] == 0)
            {
                scanner.last_error_cursor = current_sentence_element.cursor_number;
                print_error_string(402, "Too many ')' in right part");
            }
            else
                brackets_count[brackets_k_level]--;
            state = GET_RPE;
            break;
        // An s-variable of the result: copied by n_muls from its first occurrence in the left part;
        // it must be present there (406) and be of the same kind (303).
        case RPE4:
            // s - varyable
            if (!search_variable(false))
            {
                scanner.last_error_cursor = current_sentence_element.cursor_number;
                PRINT_ERROR_406;
            }
            else
                switch (variables[variable_index].type)
                {
                case S:
                    generate_operator_n(n_muls, (uint8_t)variables[variable_index].main_right_number_element);
                    break;
                default:
                    scanner.last_error_cursor = current_sentence_element.cursor_number;
                    PRINT_ERROR_303;
                };
            state = GET_RPE;
            break;
        // A w-variable of the result: copied by n_mule (a term is copied like an expression).
        case RPE5:
            // w - varyable
            if (!search_variable(false))
            {
                scanner.last_error_cursor = current_sentence_element.cursor_number;
                PRINT_ERROR_406;
            }
            else
                switch (variables[variable_index].type)
                {
                case W:
                    generate_operator_n(n_mule, (uint8_t)variables[variable_index].main_right_number_element);
                    break;
                default:
                    scanner.last_error_cursor = current_sentence_element.cursor_number;
                    PRINT_ERROR_303;
                };
            state = GET_RPE;
            break;
        // An e- or v-variable of the result: copied by n_mule.
        case RPE6:
            // e- or v-varyable
            if (!search_variable(false))
            {
                scanner.last_error_cursor = current_sentence_element.cursor_number;
                PRINT_ERROR_406;
            }
            else if (variables[variable_index].type == E && variables[variable_index].v_variable == current_sentence_element.v_variable)
                generate_operator_n(n_mule, (uint8_t)variables[variable_index].main_right_number_element);
            else
            {
                scanner.last_error_cursor = current_sentence_element.cursor_number;
                PRINT_ERROR_303;
            }
            state = GET_RPE;
            break;
        // The call bracket '<': a new level is opened (at most 511 levels), the function label after
        // it is generated together with the bracket (n_blf).
        case RPE7:
            // sign '<'
            if (brackets_k_level > 511)
            {
                scanner.last_error_cursor = current_sentence_element.cursor_number;
                print_error_string(407, "Including of the signs '<' > 511");
                state = RP_OSH300;
                break;
            }
            brackets_count[++brackets_k_level] = 0;
            scan_sentence_element();
            if (current_sentence_element.type == SC && current_sentence_element.code.tag == TAGF)
            {
                function_pointer.info.codef = current_sentence_element.code.info.codef;
                function_pointer.tag = TAGO;
                generate_operator_l(n_blf, function_pointer.info.codef);
                state = GET_RPE;
                break;
            }
            generate_operator(n_bl);
            state = SW_RPE;
            break;
        // The call bracket '>': n_bract closes the call; its level must be open (404)
        // and all its structure brackets must be closed (401).
        case RPE8:
            // sign '>'
            if (brackets_k_level == 1)
            {
                scanner.last_error_cursor = current_sentence_element.cursor_number;
                print_error_string(404, "Too many sign '>' in right part");
            }
            else
            {
                if (brackets_count[brackets_k_level] != 0)
                {
                    scanner.last_error_cursor = current_sentence_element.cursor_number;
                    print_error_string(401, "Too many '(' in right part");
                }
                generate_operator(n_bract);
                brackets_k_level--;
            };
            state = GET_RPE;
            break;
        // The '=' sign is not allowed in the right part.
        case RPE9:
            // sign '=' in right part
            scanner.last_error_cursor = current_sentence_element.cursor_number;
            print_error_string(405, "Sign '=' in right part");
            state = GET_RPE;
            break;
        // The end of the sentence: n_eos; all brackets and calls must be closed (403, 401).
        case RPE10:
            // sentence end
            generate_operator(n_eos);
            if (brackets_k_level != 1)
            {
                scanner.last_error_cursor = current_sentence_element.cursor_number;
                print_error_string(403, "Too many signs '<' in right part");
            }
            if (brackets_count[brackets_k_level] != 0)
            {
                scanner.last_error_cursor = current_sentence_element.cursor_number;
                print_error_string(401, "Too many '(' in right part");
            }
#if defined mdebug
            fprintf(stderr, "compile_sentence: end, right part compiled\n");
#endif
            compile_output_switch_to_macrocode();
            return;
        // Error exit from the right part: the sentence is reported as not scanned (300).
        case RP_OSH300:
            scanner.last_error_cursor = current_sentence_element.cursor_number;
            print_error_string(300, "Sentence is't scanned");
            compile_output_switch_to_macrocode();
            return;
        //                      place of compiler's error
        // An impossible state: an error of the compiler itself.
        case ERROR:
            printf("Compiler's error\n");
            exit(1);
            return;
        }
}

//------------------------------------------------------------------
//   Transplantation of left part pieces into the right part.
//
//   The right part is read ahead (with the listing suppressed) into
//   rp_elements. A piece is a run of left part elements that equals
//   a run of right part elements; it is moved into the result by one
//   operator. For the current right element the compiler chooses between
//   "transplant one of the pieces" and "generate the element as usual"
//   by a search that minimizes the number of generated operators of
//   the rest of the right part (branch and bound with a node budget).
//
//   Brackets: a piece must be balanced, because bracket links keep
//   pointers to their pair. Balance is counted while the piece grows,
//   so the pairs of left part brackets are not needed.
//------------------------------------------------------------------
#define LOOKAHEAD_MAX UINT8_MAX
#define OPTIONS_MAX 24

static T_SENTENCE_ELEMENT rp_elements[LOOKAHEAD_MAX];
static uint16_t rp_count;

typedef struct transplant_option
{
    uint8_t first;     // first left part element of the piece
    uint8_t length;    // number of left part elements (used ones inside included)
    uint16_t consumed; // number of right part elements replaced by the piece
} T_TRANSPLANT_OPTION;

static bool rp_element_matches(const T_SENTENCE_ELEMENT *r, uint8_t j, const bool *in_chain)
{
    if (left_part_elements[j].type != r->type)
        return false;
    if (r->type == SC)
    {
        if (left_part_elements[j].code.tag != r->code.tag)
            return false;
        return left_part_elements[j].code.info.codef == r->code.info.codef;
    }
    if (r->type == S_V || r->type == W_V || r->type == E_V)
    {
        const uint8_t v = left_part_elements[j].variable_index;
        if (variables[v].identifier_length != r->identifier_length)
            return false;
        if (strncmp(variables[v].identifier, r->identifier, variables[v].identifier_length) != 0)
            return false;
        if (left_part_elements[j].v_variable != r->v_variable)
            return false;
        return in_chain[j];
    }
    return true;
}

// Keeps options sorted by consumed (descending); equal ones stay in the order of adding.
static void add_option(T_TRANSPLANT_OPTION *options, uint8_t *count, uint8_t first, uint8_t length, uint16_t consumed)
{
    uint8_t k = *count;
    if (k == OPTIONS_MAX)
    {
        if (consumed <= options[k - 1].consumed)
            return;
        k--;
    }
    else
        (*count)++;
    while (k > 0 && options[k - 1].consumed < consumed)
    {
        options[k] = options[k - 1];
        k--;
    }
    options[k].first = first;
    options[k].length = length;
    options[k].consumed = consumed;
}

// All balanced pieces that start at rp_elements[pos] (best OPTIONS_MAX of them).
static uint8_t collect_options(uint16_t pos, const bool *used, const bool *in_chain, T_TRANSPLANT_OPTION *options)
{
    uint8_t count = 0;
    for (uint8_t i = 1; i <= left_part_elements_count; i++)
    {
        if (used[i] || !rp_element_matches(&rp_elements[pos], i, in_chain))
            continue;
        uint16_t r = pos;
        uint16_t consumed = 0;
        uint8_t balance = 0;
        for (uint8_t j = i; j <= left_part_elements_count; j++)
        {
            // Used element inside - "garbage", it is moved together with the piece
            if (j != i && used[j])
                continue;
            if (r >= rp_count || !rp_element_matches(&rp_elements[r], j, in_chain))
                break;
            if (left_part_elements[j].type == LB)
                balance++;
            else if (left_part_elements[j].type == RB)
            {
                if (balance == 0)
                    break;
                balance--;
            }
            r++;
            consumed++;
            if (balance == 0)
                add_option(options, &count, i, (uint8_t)(j - i + 1), consumed);
        }
    }
    return count;
}

// Number of right part elements that the ordinary handlers generate by one operator.
static uint16_t copy_unit(uint16_t pos)
{
    const T_SENTENCE_ELEMENT *e = &rp_elements[pos];
    uint16_t unit = 1;
    if (e->type == SC && e->code.tag == TAGO)
    {
        // a run of symbols is generated by one operator
        while (pos + unit < rp_count && unit < UINT8_MAX && rp_elements[pos + unit].type == SC && rp_elements[pos + unit].code.tag == TAGO)
            unit++;
    }
    else if (e->type == LB || e->type == K)
    {
        // "()" and a function name after '<' are generated together with the bracket
        if (pos + 1 < rp_count)
        {
            if (e->type == LB && rp_elements[pos + 1].type == RB)
                unit = 2;
            else if (rp_elements[pos + 1].type == SC && rp_elements[pos + 1].code.tag == TAGF)
                unit = 2;
        }
    }
    return unit;
}

static void mark_option_used(bool *used, const T_TRANSPLANT_OPTION *option)
{
    for (uint8_t p = option->first; p < option->first + option->length; p++)
        used[p] = true;
}

// Number of operators for rp_elements[pos..] when the longest piece is always taken.
static uint16_t greedy_cost(uint16_t pos, bool *used, const bool *in_chain)
{
    uint16_t cost = 0;
    T_TRANSPLANT_OPTION options[OPTIONS_MAX];
    while (pos < rp_count)
    {
        cost++;
        if (collect_options(pos, used, in_chain, options) > 0)
        {
            mark_option_used(used, &options[0]);
            pos += options[0].consumed;
        }
        else
            pos += copy_unit(pos);
    }
    return cost;
}

// Minimal number of operators for rp_elements[pos..]. The result is exact only
// if it is less than limit; otherwise any value >= limit is returned.
static uint16_t solve(uint16_t pos, const bool *used, const bool *in_chain, uint16_t limit, int32_t *budget)
{
    uint16_t base = 0;
    uint8_t options_count = 0;
    T_TRANSPLANT_OPTION options[OPTIONS_MAX];
    while (pos < rp_count)
    {
        if (base >= limit)
            return base;
        options_count = collect_options(pos, used, in_chain, options);
        if (options_count > 0)
            break;
        base++;
        pos += copy_unit(pos);
    }
    if (pos >= rp_count)
        return base;
    if (base + 1 >= limit)
        return limit;
    bool next_used[UINT8_MAX];
    if ((*budget)-- <= 0)
    {
        memcpy(next_used, used, sizeof(next_used));
        return base + greedy_cost(pos, next_used, in_chain);
    }
    uint16_t best = limit;
    for (uint8_t k = 0; k < options_count; k++)
    {
        memcpy(next_used, used, sizeof(next_used));
        mark_option_used(next_used, &options[k]);
        const uint16_t cost = base + 1 + solve(pos + options[k].consumed, next_used, in_chain, best - base - 1, budget);
        if (cost < best)
            best = cost;
    }
    const uint16_t cost = base + 1 + solve(pos + copy_unit(pos), used, in_chain, best - base - 1, budget);
    if (cost < best)
        best = cost;
    return best;
}

// Tries to transplant a piece of the left part into the result instead of generating the
// current right part element: a piece that equals a run of right part elements is moved
// by one n_tpl* statement (copy of a part of the matched expression).
// The decision is made by a search over the rest of the right part (see solve): the
// transplant is taken only if the number of generated operators does not grow.
// Returns true when a piece was transplanted (the right part elements are consumed),
// false when the current element is to be generated as usual.
static bool try_transplant(void)
{
    bool used[UINT8_MAX];
    bool in_chain[UINT8_MAX];
    // Step 1: Quick check - is there any candidate for the current element
    for (uint8_t i = 0; i <= left_part_elements_count; i++)
    {
        used[i] = left_part_elements[i].used;
        in_chain[i] = true;
    }
    // in_chain: variable elements that are still in their variable chain
    for (uint8_t i = 1; i <= left_part_elements_count; i++)
        if (left_part_elements[i].type == S_V || left_part_elements[i].type == W_V || left_part_elements[i].type == E_V)
            in_chain[i] = false;
    for (uint8_t v = 0; v < variables_count; v++)
        for (uint8_t curr = variables[v].last_left_part_element; curr != 0; curr = left_part_elements[curr].next_variable)
            in_chain[curr] = true;
    rp_elements[0] = current_sentence_element;
    bool has_candidate = false;
    for (uint8_t i = 1; i <= left_part_elements_count; i++)
        if (!used[i] && rp_element_matches(&rp_elements[0], i, in_chain))
        {
            has_candidate = true;
            break;
        }
    if (!has_candidate)
    {
#if defined mdebug
        fprintf(stderr, "try_transplant: no candidate in the left part for ");
        debug_print_sentence_element(&current_sentence_element);
        fprintf(stderr, "\n");
#endif
        return false;
    }
    // Step 2: Read the rest of the right part ahead
    T_SAVE_SCANNER_STATE saved_state;
    save_scanner_state(&saved_state);
    suppress_listing = true;
    rp_count = 1;
    while (rp_count < LOOKAHEAD_MAX)
    {
        scan_sentence_element();
        if (current_sentence_element.type == END || current_sentence_element.type == NONE)
            break;
        rp_elements[rp_count++] = current_sentence_element;
    }
    restore_scanner_state(&saved_state);
#if defined mdebug
    fprintf(stderr, "try_transplant: read ahead %u right part elements, search_budget=%d\n", rp_count, (int)search_budget);
#endif
    // Step 3: Choose between transplantation of a piece and ordinary generation
    T_TRANSPLANT_OPTION options[OPTIONS_MAX];
    const uint8_t options_count = collect_options(0, used, in_chain, options);
#if defined mdebug
    fprintf(stderr, "try_transplant: %u option(s):\n", options_count);
    for (uint8_t k = 0; k < options_count; k++)
        fprintf(stderr, "    option %u: left part elements [%u..%u] (length %u) replace %u right part element(s)\n", k, options[k].first, options[k].first + options[k].length - 1, options[k].length, options[k].consumed);
#endif
    if (options_count == 0)
    {
        free_scanner_state(&saved_state);
        return false;
    }
    uint8_t best_option = 0;
    // When the budget of the sentence is exhausted the longest piece is taken
    if (search_budget > 0)
    {
        int32_t budget = search_budget < DECISION_SEARCH_BUDGET ? search_budget : DECISION_SEARCH_BUDGET;
        const int32_t budget_start = budget;
        uint16_t best_cost = UINT16_MAX;
        bool next_used[UINT8_MAX];
        for (uint8_t k = 0; k < options_count; k++)
        {
            memcpy(next_used, used, sizeof(next_used));
            mark_option_used(next_used, &options[k]);
            const uint16_t cost = 1 + solve(options[k].consumed, next_used, in_chain, best_cost - 1, &budget);
#if defined mdebug
            fprintf(stderr, "try_transplant: option %u cost=%u%s (budget left %d)\n", k, cost, cost < best_cost ? " [best so far]" : "", (int)budget);
#endif
            if (cost < best_cost)
            {
                best_cost = cost;
                best_option = k;
            }
        }
        // Ordinary generation is chosen only if it is strictly cheaper
        const bool copy_is_better = 1 + solve(copy_unit(0), used, in_chain, best_cost - 1, &budget) < best_cost;
        search_budget -= budget_start - budget;
#if defined mdebug
        fprintf(stderr, "try_transplant: best transplant cost=%u, ordinary generation is %s, nodes spent=%d, sentence budget left=%d\n", best_cost, copy_is_better ? "cheaper" : "not cheaper", (int)(budget_start - budget), (int)search_budget);
#endif
        if (copy_is_better)
        {
            free_scanner_state(&saved_state);
            return false;
        }
    }
    const uint8_t best_i = options[best_option].first;
    const uint8_t best_length = options[best_option].length;
#if defined mdebug
    if (search_budget <= 0)
        fprintf(stderr, "try_transplant: sentence budget exhausted, the first option is taken\n");
    fprintf(stderr, "try_transplant: transplant option %u, left part elements [%u..%u]\n", best_option, best_i, best_i + best_length - 1);
#endif
    // Step 6: Restore scanner state
    restore_scanner_state(&saved_state);
    // Re-read right part — LISTING IS OUTPUT
    for (uint8_t p = best_i + 1; p < best_i + best_length; p++)
    {
        if (left_part_elements[p].used)
            continue;
        scan_sentence_element();
    }
    // Determine numbers for generation
    const uint16_t N = (best_length == 1) ? left_part_elements[best_i].right_number_element : left_part_elements[best_i].left_number_element;
    const uint16_t M = left_part_elements[best_i + best_length - 1].right_number_element;
    // Determine if guaranteed non-empty element exists
    bool has_non_empty = false;
    for (uint8_t p = best_i; p < best_i + best_length; p++)
    {
        if (left_part_elements[p].type == SC || left_part_elements[p].type == LB || left_part_elements[p].type == RB || left_part_elements[p].type == S_V || left_part_elements[p].type == W_V || (left_part_elements[p].type == E_V && left_part_elements[p].v_variable))
        {
            has_non_empty = true;
            break;
        }
    }
    // Generate operator
    if (has_non_empty)
    {
        if (best_length == 1)
        {
            if (left_part_elements[best_i].type == S_V || left_part_elements[best_i].type == SC)
                generate_operator_n(n_tpls, (uint8_t)N);
            else if (left_part_elements[best_i].type == W_V || left_part_elements[best_i].type == E_V)
                generate_operator_n(n_tplv, (uint8_t)N);
        }
        else
            generate_operator_n_m(n_tplm, (uint8_t)N, (uint8_t)M);
    }
    else
    {
        if (best_length == 1)
            generate_operator_n(n_tple, (uint8_t)N);
        else
            generate_operator_n_m(n_tpl, (uint8_t)N, (uint8_t)M);
    }
    // Step 7: Update chains and mark used
    for (uint8_t p = best_i; p < best_i + best_length; p++)
    {
        left_part_elements[p].used = true;
        if (left_part_elements[p].type == S_V || left_part_elements[p].type == W_V || left_part_elements[p].type == E_V)
        {
            const uint8_t v = left_part_elements[p].variable_index;
            uint8_t prev = 0;
            uint8_t curr = variables[v].last_left_part_element;
            while (curr != 0)
            {
                if (curr == p)
                {
                    if (prev == 0)
                        variables[v].last_left_part_element = left_part_elements[curr].next_variable;
                    else
                        left_part_elements[prev].next_variable = left_part_elements[curr].next_variable;
                    break;
                }
                prev = curr;
                curr = left_part_elements[curr].next_variable;
            }
        }
    }
    // Step 8: Success
#if defined mdebug
    fprintf(stderr, "try_transplant: done, N=%u M=%u non_empty=%d\n", N, M, has_non_empty);
#endif
    free_scanner_state(&saved_state);
    return true;
}

// Looks for the variable named by the current sentence element in the variable table
// and sets variable_index. In the left part a missing variable is entered as a new one;
// in the right part false is returned (the variable is not defined, error 406).
static bool search_variable(bool left_part)
{
    for (variable_index = 0; variable_index < variables_count; variable_index++)
        if (variables[variable_index].identifier_length == current_sentence_element.identifier_length && strncmp(variables[variable_index].identifier, current_sentence_element.identifier, variables[variable_index].identifier_length) == 0)
            return true;
    if (!left_part)
        return false;
    variable_index = variables_count++;
#if defined mdebug
    fprintf(stderr, "search_variable: new variable [%u] %.*s\n", variable_index, current_sentence_element.identifier_length, current_sentence_element.identifier);
#endif
    strncpy(variables[variable_index].identifier, current_sentence_element.identifier, current_sentence_element.identifier_length);
    variables[variable_index].identifier_length = current_sentence_element.identifier_length;
    variables[variable_index].type = NEW;
    variables[variable_index].remains = 1;
    variables[variable_index].last_left_part_element = 0;
    variables[variable_index].v_variable = current_sentence_element.v_variable;
    return true;
}

// Loads the holes into the hole list: the first hole gets the number 1, the list ends with the
// terminating segment 128, the holes that appear later are numbered from count + 1.
static void load_class_holes(const uint8_t *left_boards, const uint8_t *right_boards, uint8_t count)
{
    for (uint8_t i = 0; i < count; i++)
    {
        hole_list[i + 1].left_board = left_boards[i];
        hole_list[i + 1].right_board = right_boards[i];
        hole_list[i + 1].next_hole = (i + 1 < count) ? (uint8_t)(i + 2) : 128;
    }
    hole_list[128].next_hole = 0;
    current_hole = 1;
    free_segment_hole_list = count + 1;
    return;
}

// The root of the set of the holes that contains the hole i (union-find with path halving).
static uint8_t class_root(uint8_t *parent, uint8_t i)
{
    while (parent[i] != i)
    {
        parent[i] = parent[parent[i]];
        i = parent[i];
    }
    return i;
}

// Decomposition of the hole list into classes of independent holes (S.A. Romanenko, "Realization
// of Refal-2", section 4.2, the step HDECOMP). Two holes are dependent when a variable that
// is not bound yet occurs in both of them; the classes are the sets of the holes that are linked by
// this relation (directly or through other holes). The matching of one class cannot be influenced
// by the choices made in another class, so the classes are compiled one after another, and when a
// class is compiled, the points that its searches have left on the jump stack are removed by n_eoe
// (see the state OCLASS): if a later class fails, it would be useless to look for other solutions
// of this class. Only the first class stays in the hole list, the others are put on the class stack.
// The decomposition is not done while some e-variable is marked (eoe_mark): such a mark was put when
// the search that is still open was started, and the marked variable closes it, so the elements
// that are marked and the searches they close must stay in the same class and the same order.
static void decompose_holes(void)
{
    uint8_t slots[130];
    uint8_t count = 0;
    for (uint8_t h = 1; hole_list[h].next_hole != 0; h = hole_list[h].next_hole)
        slots[count++] = h;
    if (count < 2)
        return;
    uint8_t parent[130];
    uint8_t owner[UINT8_MAX + 1]; // the first hole where a free variable occurs
    for (uint8_t i = 0; i < count; i++)
        parent[i] = i;
    for (uint16_t v = 0; v <= UINT8_MAX; v++)
        owner[v] = UINT8_MAX;
    for (uint8_t i = 0; i < count; i++)
    {
        for (uint8_t e = hole_list[slots[i]].left_board + 1; e < hole_list[slots[i]].right_board; e++)
        {
            if (left_part_elements[e].eoe_mark)
                return;
            const T_SENTENCE_ELEMENT_TYPES type = left_part_elements[e].type;
            if (type != S_V && type != W_V && type != E_V)
                continue;
            const uint8_t v = left_part_elements[e].variable_index;
            if (variables[v].last_left_part_element != 0)
                continue;
            if (owner[v] == UINT8_MAX)
                owner[v] = i;
            else
            {
                const uint8_t root_a = class_root(parent, owner[v]);
                const uint8_t root_b = class_root(parent, i);
                if (root_a != root_b)
                    parent[root_b] = root_a;
            }
        }
    }
    // The classes are numbered in the order of their first holes.
    uint8_t class_of[130];
    uint8_t root_class[130];
    uint8_t class_count = 0;
    for (uint8_t i = 0; i < count; i++)
        root_class[i] = UINT8_MAX;
    for (uint8_t i = 0; i < count; i++)
    {
        const uint8_t root = class_root(parent, i);
        if (root_class[root] == UINT8_MAX)
            root_class[root] = class_count++;
        class_of[i] = root_class[root];
    }
    if (class_count < 2)
        return;
#if defined mdebug
    fprintf(stderr, "decompose_holes: %u holes, %u classes, jump_stack_pointer=%u\n", count, class_count, jump_stack_pointer);
#endif
    // The classes from the last to the second are put on the stack, so the second one is on the top.
    for (uint8_t c = class_count - 1; c >= 1; c--)
    {
        class_stack[class_stack_count].first_hole = class_holes_count;
        class_stack[class_stack_count].jump_stack_pointer = jump_stack_pointer;
        uint8_t holes_in_class = 0;
        for (uint8_t i = 0; i < count; i++)
        {
            if (class_of[i] != c)
                continue;
            class_holes[class_holes_count].left_board = hole_list[slots[i]].left_board;
            class_holes[class_holes_count].right_board = hole_list[slots[i]].right_board;
            class_holes_count++;
            holes_in_class++;
        }
        class_stack[class_stack_count].hole_count = holes_in_class;
        class_stack_count++;
    }
    // The first class replaces the hole list. If the boards of the generated code are those of
    // one of its holes, that hole keeps being the current one and needs no n_sb1b2.
    uint8_t left_boards[130], right_boards[130];
    uint8_t first_count = 0;
    uint8_t new_hole_x = 0;
    for (uint8_t i = 0; i < count; i++)
    {
        if (class_of[i] != 0)
            continue;
        left_boards[first_count] = hole_list[slots[i]].left_board;
        right_boards[first_count] = hole_list[slots[i]].right_board;
        if (slots[i] == hole_x)
            new_hole_x = first_count + 1;
        first_count++;
    }
    load_class_holes(left_boards, right_boards, first_count);
    hole_x = new_hole_x;
    return;
}

// Generation of the statements for stopped brackets and setting of the boards.
// stoped_bracket_flag: 0 - no stopped bracket, 1 - left bracket, 2 - right bracket; for a
// hole that is not the one just opened n_sb1b2 sets the new boards of the hole.
static void generate_boards_stoping_brackets(void)
{
#if defined mdebug
    fprintf(stderr, "generate_boards_stoping_brackets: stoped_bracket_flag=%u boards (%u, %u)\n", stoped_bracket_flag, current_left_board, current_right_board);
#endif
    switch (stoped_bracket_flag)
    {
    case 0: // no stoped brackets
        if (current_hole != hole_x)
            break;
        return;
    case 1: // left stoped brackets
        if (current_hole == hole_x)
            generate_operator(n_lb);
        else if (current_hole == hole_y)
            generate_operator(n_lby);
        else
        {
            generate_operator(n_lb);
            break;
        };
        return;
    case 2: // right stoped brackets
        if (current_hole == hole_x)
            generate_operator(n_rb);
        else if (current_hole == hole_y)
            generate_operator(n_rby);
        else
        {
            generate_operator(n_rb);
            break;
        };
        return;
    };
    generate_operator_n_m(n_sb1b2, (uint8_t)left_part_elements[current_left_board].right_number_element, (uint8_t)left_part_elements[current_right_board].left_number_element);
    return;
}

// Attempt to extract the left support group: the search goes right from the left board
// and stops at the first free e-variable that is orthogonal to the elements before it
// (no other occurrence of its variables outside). Such a variable is marked (eoe_mark)
// as the end of the search level. The e-variable at the left board is then bound.
// Returns false when its specifier is checked at once (n_plespc, n_lespc).
static bool lsg_p(void)
{
    while (true)
    {
        current_left_part_element++;
        if (current_left_part_element == current_right_board)
            break;
        if (left_part_elements[current_left_part_element].type != LB)
        {
            if (left_part_elements[current_left_part_element].type != E_V)
                continue;
            variable_index = left_part_elements[current_left_part_element].variable_index;
            if (variable_index == temp_variable_index || variables[variable_index].last_left_part_element != 0)
                continue;
            if (left_part_elements[current_left_part_element].specifier.info.codef != NULL || variables[variable_index].remains != 1)
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
    variables[variable_index].remains--;
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

// Attempt to extract the right support group: the mirror image of lsg_p.
static bool rsg_p(void)
{
    while (true)
    {
        current_left_part_element--;
        if (current_left_part_element == current_left_board)
            break;
        if (left_part_elements[current_left_part_element].type != RB)
        {
            if (left_part_elements[current_left_part_element].type != E_V)
                continue;
            variable_index = left_part_elements[current_left_part_element].variable_index;
            if (variable_index == temp_variable_index || variables[variable_index].last_left_part_element != 0)
                continue;
            if (left_part_elements[current_left_part_element].specifier.info.codef != NULL || variables[variable_index].remains != 1)
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
    variables[variable_index].remains--;
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

// Checks the orthogonality of the elements between on1 and on2: every free variable
// in the range must have all its remaining occurrences inside the range. Then a failure
// to match the elements after the range does not require another expansion of the
// e-variable in front of it, so the search level can be closed (the n_eoe statement).
static bool ortogonality(uint8_t on1, uint8_t on2)
{
    uint8_t on = on1;
    uint8_t i;
    while (true)
    {
        on++;
        if (on == on2)
            break;
        if (left_part_elements[on].type <= RB)
            continue;
        i = left_part_elements[on].variable_index;
        if (variables[i].last_left_part_element != 0)
            continue;
        variables[i].remains--;
    }
    bool res = true;
    on = on1;
    while (true)
    {
        on++;
        if (on == on2)
            break;
        if (left_part_elements[on].type <= RB)
            continue;
        i = left_part_elements[on].variable_index;
        if (variables[i].last_left_part_element != 0)
            continue;
        if (variables[i].remains == 0)
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
        if (left_part_elements[on].type <= RB)
            continue;
        i = left_part_elements[on].variable_index;
        if (variables[i].last_left_part_element != 0)
            continue;
        variables[i].remains++;
    }
    return res;
}

// Generates the statement for an e-variable, or for a v-variable (which must not be empty)
// when not_nil is set.
static inline void generate_operator_e_v(uint8_t operator_e, uint8_t operator_v)
{
    if (not_nil)
        generate_operator(operator_v);
    else
        generate_operator(operator_e);
}

//----------  end of file compile_sentence.c  ----------
