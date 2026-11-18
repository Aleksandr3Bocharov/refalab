// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-11-17
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file xio.c  ----------
//           MO: input/output
//----------------------------------

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "refalab.h"
#include "interface.h"

// Card: <Card e.Prompt> - reads a line from the standard input and returns it as characters
// (without the line feed). A non-empty argument is printed first, as a prompt, without a line
// feed. When the input ends, the characters read so far are followed by the macrodigit 0.
//
// The functions of the printing family have these variants:
//   Print, Prout   - symbols in the form that Print uses (characters in apostrophes, ...)
//   ...s           - characters are printed without apostrophes
//   ...m           - the expression is printed in the form of a source program
//   Print..., Write... return their argument, Prout..., Wrout... return nothing
//   Print..., Prout... end the output with a line feed, Write..., Wrout... do not.
static void card_(void)
{
    if (refal.previous_argument->next != refal.next_argument)
        print_expression_s("", refal.previous_argument, refal.next_argument, false);
    T_LINKCB *current_result = refal.previous_result;
    int get_result = getchar();
    while (get_result != '\n')
    {
        if (!extended_insert_from_free_memory(current_result, 1))
            return;
        current_result = current_result->next;
        current_result->info.code = NULL;
        if (get_result == EOF)
        {
            current_result->tag = TAGN;
            return;
        }
        current_result->tag = TAGO;
        current_result->info.infoc = (char)get_result;
        get_result = getchar();
    }
    return;
}
char card_0[] = {Z4 'C', 'A', 'R', 'D', (char)4};
G_L_B uint8_t refalab_card = '\122';
void (*card_1)(void) = card_;

// Get_char: <Get_char> - one character from the standard input; the macrodigit 0 at the end
// of the input. FAIL when there is an argument.
static void get_char_(void)
{
    if (refal.previous_argument->next != refal.next_argument)
    {
        refal.upshot = 2;
        return;
    }
    const int get_result = getchar();
    refal.previous_argument->info.code = NULL;
    if (get_result == EOF)
        refal.previous_argument->tag = TAGN;
    else
    {
        refal.previous_argument->tag = TAGO;
        refal.previous_argument->info.infoc = (char)get_result;
    }
    transplantation(refal.previous_result, refal.previous_argument->previous, refal.previous_argument->next);
    return;
}
char get_char_0[] = {Z0 'G', 'E', 'T', '_', 'C', 'H', 'A', 'R', (char)8};
G_L_B uint8_t refalab_get_char = '\122';
void (*get_char_1)(void) = get_char_;

// Print: prints the expression with a line feed and returns it.
static void print_(void)
{
    print_expression("", refal.previous_argument, refal.next_argument, true);
    transplantation(refal.previous_result, refal.previous_argument, refal.next_argument);
    return;
}
char print_0[] = {Z5 'P', 'R', 'I', 'N', 'T', (char)5};
G_L_B uint8_t refalab_print = '\122';
void (*print_1)(void) = print_;

// Prints: like Print, but characters are not put into apostrophes; returns the expression.
static void prints_(void)
{
    print_expression_s("", refal.previous_argument, refal.next_argument, true);
    transplantation(refal.previous_result, refal.previous_argument, refal.next_argument);
    return;
}
char prints_0[] = {Z6 'P', 'R', 'I', 'N', 'T', 'S', (char)6};
G_L_B uint8_t refalab_prints = '\122';
void (*prints_1)(void) = prints_;

// Printm: like Print, but in the form of a source program; returns the expression.
static void printm_(void)
{
    print_expression_m("", refal.previous_argument, refal.next_argument, true);
    transplantation(refal.previous_result, refal.previous_argument, refal.next_argument);
    return;
}
char printm_0[] = {Z6 'P', 'R', 'I', 'N', 'T', 'M', (char)6};
G_L_B uint8_t refalab_printm = '\122';
void (*printm_1)(void) = printm_;

// Prout: like Print, but returns nothing.
static void prout_(void)
{
    print_expression("", refal.previous_argument, refal.next_argument, true);
    return;
}
char prout_0[] = {Z5 'P', 'R', 'O', 'U', 'T', (char)5};
G_L_B uint8_t refalab_prout = '\122';
void (*prout_1)(void) = prout_;

// Prouts: like Prints, but returns nothing.
static void prouts_(void)
{
    print_expression_s("", refal.previous_argument, refal.next_argument, true);
    return;
}
char prouts_0[] = {Z6 'P', 'R', 'O', 'U', 'T', 'S', (char)6};
G_L_B uint8_t refalab_prouts = '\122';
void (*prouts_1)(void) = prouts_;

// Proutm: like Printm, but returns nothing.
static void proutm_(void)
{
    print_expression_m("", refal.previous_argument, refal.next_argument, true);
    return;
}
char proutm_0[] = {Z6 'P', 'R', 'O', 'U', 'T', 'M', (char)6};
G_L_B uint8_t refalab_proutm = '\122';
void (*proutm_1)(void) = proutm_;

// Write: like Print, but without the line feed at the end.
static void write_(void)
{
    print_expression("", refal.previous_argument, refal.next_argument, false);
    transplantation(refal.previous_result, refal.previous_argument, refal.next_argument);
    return;
}
char write_0[] = {Z5 'W', 'R', 'I', 'T', 'E', (char)5};
G_L_B uint8_t refalab_write = '\122';
void (*write_1)(void) = write_;

// Writes: like Prints, but without the line feed at the end.
static void writes_(void)
{
    print_expression_s("", refal.previous_argument, refal.next_argument, false);
    transplantation(refal.previous_result, refal.previous_argument, refal.next_argument);
    return;
}
char writes_0[] = {Z6 'W', 'R', 'I', 'T', 'E', 'S', (char)6};
G_L_B uint8_t refalab_writes = '\122';
void (*writes_1)(void) = writes_;

// Writem: like Printm, but without the line feed at the end.
static void writem_(void)
{
    print_expression_m("", refal.previous_argument, refal.next_argument, false);
    transplantation(refal.previous_result, refal.previous_argument, refal.next_argument);
    return;
}
char writem_0[] = {Z6 'W', 'R', 'I', 'T', 'E', 'M', (char)6};
G_L_B uint8_t refalab_writem = '\122';
void (*writem_1)(void) = writem_;

// Wrout: like Prout, but without the line feed at the end.
static void wrout_(void)
{
    print_expression("", refal.previous_argument, refal.next_argument, false);
    return;
}
char wrout_0[] = {Z5 'W', 'R', 'O', 'U', 'T', (char)5};
G_L_B uint8_t refalab_wrout = '\122';
void (*wrout_1)(void) = wrout_;

// Wrouts: like Prouts, but without the line feed at the end.
static void wrouts_(void)
{
    print_expression_s("", refal.previous_argument, refal.next_argument, false);
    return;
}
char wrouts_0[] = {Z6 'W', 'R', 'O', 'U', 'T', 'S', (char)6};
G_L_B uint8_t refalab_wrouts = '\122';
void (*wrouts_1)(void) = wrouts_;

// Wroutm: like Proutm, but without the line feed at the end.
static void wroutm_(void)
{
    print_expression_m("", refal.previous_argument, refal.next_argument, false);
    return;
}
char wroutm_0[] = {Z6 'W', 'R', 'O', 'U', 'T', 'M', (char)6};
G_L_B uint8_t refalab_wroutm = '\122';
void (*wroutm_1)(void) = wroutm_;

//----------  end of file xio.c  ------------
