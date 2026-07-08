// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-06-05
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file xbox.c  ---------
//       MO: Gtr, Rdr, Wtr, Swr, New
//----------------------------------

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "refalab.h"
#include "interface.h"

#define N_SWAP 0116

static bool legal_argument_box(bool empty_expression, T_LINKCB **head_box_pointer, T_LINKCB **first_argument_pointer)
{
    T_LINKCB *first_argument = refal.previous_argument->next;
    if (first_argument == refal.next_argument)
        return false;
    if (empty_expression && first_argument->next != refal.next_argument)
        return false;
    T_LINKCB *head_box;
    if (first_argument->tag == TAGR)
        head_box = first_argument->info.codep;
    else if (first_argument->tag == TAGF)
    {
        const uint8_t *swap = first_argument->info.codef;
        if (*swap != N_SWAP)
            return false;
        head_box = (T_LINKCB *)(swap + 1);
        if (head_box->previous == NULL)
        {
            head_box->next = head_box;
            head_box->previous = head_box->next;
            head_box->info.codep = refal.static_boxes;
            head_box->tag = TAGO;
            refal.static_boxes = head_box;
        }
    }
    else
        return false;
    *head_box_pointer = head_box;
    *first_argument_pointer = first_argument;
    return true;
}

static void gtr_(void)
{
    const bool empty_expression = true;
    T_LINKCB *head_box, *first_argument;
    if (!legal_argument_box(empty_expression, &head_box, &first_argument))
    {
        refal.upshot = 2;
        return;
    }; // FAIL
    transplantation(refal.previous_result, head_box, head_box);
    return;
}
char gtr_0[] = {Z3 'G', 'T', 'R', (char)3};
G_L_B uint8_t refalab_gtr = '\122';
void (*gtr_1)(void) = gtr_;

static void rdr_(void)
{
    const bool empty_expression = true;
    T_LINKCB *head_box, *first_argument;
    if (!legal_argument_box(empty_expression, &head_box, &first_argument))
    {
        refal.upshot = 2;
        return;
    }; // FAIL
    if (!copy_expression(refal.previous_result, head_box, head_box))
    {
        refal.upshot = 3;
        return;
    }; // LACK
    return;
}
char rdr_0[] = {Z3 'R', 'D', 'R', (char)3};
G_L_B uint8_t refalab_rdr = '\122';
void (*rdr_1)(void) = rdr_;

static void ptr_(void)
{
    const bool empty_expression = false;
    T_LINKCB *head_box, *first_argument;
    if (!legal_argument_box(empty_expression, &head_box, &first_argument))
    {
        refal.upshot = 2;
        return;
    }; // FAIL
    transplantation(head_box->previous, first_argument, refal.next_argument);
    return;
}
char ptr_0[] = {Z3 'P', 'T', 'R', (char)3};
G_L_B uint8_t refalab_ptr = '\122';
void (*ptr_1)(void) = ptr_;

static void wtr_(void)
{
    const bool empty_expression = false;
    T_LINKCB *head_box, *first_argument;
    if (!legal_argument_box(empty_expression, &head_box, &first_argument))
    {
        refal.upshot = 2;
        return;
    }; // FAIL
    insert_to_free_memory(head_box, head_box);
    transplantation(head_box, first_argument, refal.next_argument);
    return;
}
char wtr_0[] = {Z3 'W', 'T', 'R', (char)3};
G_L_B uint8_t refalab_wtr = '\122';
void (*wtr_1)(void) = wtr_;

static void swr_(void)
{
    const bool empty_expression = false;
    T_LINKCB *head_box, *first_argument;
    if (!legal_argument_box(empty_expression, &head_box, &first_argument))
    {
        refal.upshot = 2;
        return;
    }; // FAIL
    transplantation(refal.previous_result, head_box, head_box);
    transplantation(head_box, first_argument, refal.next_argument);
    return;
}
char swr_0[] = {Z3 'S', 'W', 'R', (char)3};
G_L_B uint8_t refalab_swr = '\122';
void (*swr_1)(void) = swr_;

static void new_(void)
{
    if (!extended_insert_from_free_memory(refal.previous_result, 1))
        return; // LACK
    T_LINKCB *result_symbol_reference = refal.previous_result->next;
    result_symbol_reference->info.codep = refal.previous_argument;
    result_symbol_reference->tag = TAGR;
    T_LINKCB *last_argument = refal.next_argument->previous;
    last_argument->next = refal.previous_argument;
    refal.previous_argument->previous = last_argument;
    refal.next_result->next = refal.next_argument;
    refal.next_argument->previous = refal.next_result;
    refal.previous_argument->info.codep = refal.dynamic_boxes;
    refal.previous_argument->tag = TAGO;
    refal.dynamic_boxes = refal.previous_argument;
    return;
}
char new_0[] = {Z3 'N', 'E', 'W', (char)3};
G_L_B uint8_t refalab_new = '\122';
void (*new_1)(void) = new_;

//-----------  end of file xbox.c  ----------
