// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-05-29
// https://github.com/Aleksandr3Bocharov/refalab

//-------------- file -- XIO.C ------------
//           MO: input/output
//-----------------------------------------

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "refalab.h"
#include "rfintf.h"

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

static void print_(void)
{
    print_expression("", refal.previous_argument, refal.next_argument, true);
    transplantation(refal.previous_result, refal.previous_argument, refal.next_argument);
    return;
}
char print_0[] = {Z5 'P', 'R', 'I', 'N', 'T', (char)5};
G_L_B uint8_t refalab_print = '\122';
void (*print_1)(void) = print_;

static void prints_(void)
{
    print_expression_s("", refal.previous_argument, refal.next_argument, true);
    transplantation(refal.previous_result, refal.previous_argument, refal.next_argument);
    return;
}
char prints_0[] = {Z6 'P', 'R', 'I', 'N', 'T', 'S', (char)6};
G_L_B uint8_t refalab_prints = '\122';
void (*prints_1)(void) = prints_;

static void printm_(void)
{
    print_expression_m("", refal.previous_argument, refal.next_argument, true);
    transplantation(refal.previous_result, refal.previous_argument, refal.next_argument);
    return;
}
char printm_0[] = {Z6 'P', 'R', 'I', 'N', 'T', 'M', (char)6};
G_L_B uint8_t refalab_printm = '\122';
void (*printm_1)(void) = printm_;

static void prout_(void)
{
    print_expression("", refal.previous_argument, refal.next_argument, true);
    return;
}
char prout_0[] = {Z5 'P', 'R', 'O', 'U', 'T', (char)5};
G_L_B uint8_t refalab_prout = '\122';
void (*prout_1)(void) = prout_;

static void prouts_(void)
{
    print_expression_s("", refal.previous_argument, refal.next_argument, true);
    return;
}
char prouts_0[] = {Z6 'P', 'R', 'O', 'U', 'T', 'S', (char)6};
G_L_B uint8_t refalab_prouts = '\122';
void (*prouts_1)(void) = prouts_;

static void proutm_(void)
{
    print_expression_m("", refal.previous_argument, refal.next_argument, true);
    return;
}
char proutm_0[] = {Z6 'P', 'R', 'O', 'U', 'T', 'M', (char)6};
G_L_B uint8_t refalab_proutm = '\122';
void (*proutm_1)(void) = proutm_;

static void write_(void)
{
    print_expression("", refal.previous_argument, refal.next_argument, false);
    transplantation(refal.previous_result, refal.previous_argument, refal.next_argument);
    return;
}
char write_0[] = {Z5 'W', 'R', 'I', 'T', 'E', (char)5};
G_L_B uint8_t refalab_write = '\122';
void (*write_1)(void) = write_;

static void writes_(void)
{
    print_expression_s("", refal.previous_argument, refal.next_argument, false);
    transplantation(refal.previous_result, refal.previous_argument, refal.next_argument);
    return;
}
char writes_0[] = {Z6 'W', 'R', 'I', 'T', 'E', 'S', (char)6};
G_L_B uint8_t refalab_writes = '\122';
void (*writes_1)(void) = writes_;

static void writem_(void)
{
    print_expression_m("", refal.previous_argument, refal.next_argument, false);
    transplantation(refal.previous_result, refal.previous_argument, refal.next_argument);
    return;
}
char writem_0[] = {Z6 'W', 'R', 'I', 'T', 'E', 'M', (char)6};
G_L_B uint8_t refalab_writem = '\122';
void (*writem_1)(void) = writem_;

static void wrout_(void)
{
    print_expression("", refal.previous_argument, refal.next_argument, false);
    return;
}
char wrout_0[] = {Z5 'W', 'R', 'O', 'U', 'T', (char)5};
G_L_B uint8_t refalab_wrout = '\122';
void (*wrout_1)(void) = wrout_;

static void wrouts_(void)
{
    print_expression_s("", refal.previous_argument, refal.next_argument, false);
    return;
}
char wrouts_0[] = {Z6 'W', 'R', 'O', 'U', 'T', 'S', (char)6};
G_L_B uint8_t refalab_wrouts = '\122';
void (*wrouts_1)(void) = wrouts_;

static void wroutm_(void)
{
    print_expression_m("", refal.previous_argument, refal.next_argument, false);
    return;
}
char wroutm_0[] = {Z6 'W', 'R', 'O', 'U', 'T', 'M', (char)6};
G_L_B uint8_t refalab_wroutm = '\122';
void (*wroutm_1)(void) = wroutm_;

//------------------ end of file  XIO.C ----------------
