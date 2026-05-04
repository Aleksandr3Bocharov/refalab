// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-05-03
// https://github.com/Aleksandr3Bocharov/refalab

//-------------- file -- XSTOR.C -----------
//       MO: br, dg, dgall, rp, cp
//------------------------------------------

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "refalab.h"
#include "rfintf.h"

static void br_(void)
{
    const T_STATUS_TABLE *status_table = refal.current_status_table;
    const T_LINKCB *current_argument = refal.previous_argument;
    while (current_argument->tag != TAGO || current_argument->info.infoc != '=')
    {
        current_argument = current_argument->next;
        if (current_argument == refal.next_argument)
        {
            refal.upshot = 2;
            return;
        }; // FAIL
    }
    if (!extended_insert_from_free_memory(status_table->store, 2))
        return; // LACK
    T_LINKCB *left_bracket = status_table->store->next;
    T_LINKCB *right_bracket = left_bracket->next;
    left_bracket->info.codep = right_bracket;
    left_bracket->tag = TAGLB;
    right_bracket->info.codep = left_bracket;
    right_bracket->tag = TAGRB;
    transplantation(left_bracket, refal.previous_argument, refal.next_argument);
    return;
}
char br_0[] = {Z2 'B', 'R', (char)2};
G_L_B uint8_t refalab_br = '\122';
void (*br_1)(void) = br_;

static void dg_(void)
{
    const T_STATUS_TABLE *status_table = refal.current_status_table;
    T_LINKCB *right_bracket = status_table->store;
    T_LINKCB *after_duplicate;
    const T_LINKCB *left_bracket;
    while (true)
    {
        left_bracket = right_bracket->next;
        if (left_bracket == status_table->store)
            return;
        if (left_bracket->tag != TAGLB)
        {
            refal.upshot = 2;
            return;
        }; // FAIL
        right_bracket = left_bracket->info.codep;
        after_duplicate = find_duplicate(refal.previous_argument, refal.next_argument, left_bracket);
        if (after_duplicate == NULL)
            continue;
        if (after_duplicate->tag != TAGO || after_duplicate->info.infoc != '=')
            continue;
        break;
    }
    transplantation(refal.previous_result, after_duplicate, right_bracket);
    insert_to_free_memory(left_bracket->previous, right_bracket->next);
    return;
}
char dg_0[] = {Z2 'D', 'G', (char)2};
G_L_B uint8_t refalab_dg = '\122';
void (*dg_1)(void) = dg_;

static void dgall_(void)
{
    const T_STATUS_TABLE *status_table = refal.current_status_table;
    if (refal.previous_argument->next != refal.next_argument)
        refal.upshot = 2; // FAIL
    else
        transplantation(refal.previous_result, status_table->store, status_table->store);
    return;
}
char dgall_0[] = {Z5 'D', 'G', 'A', 'L', 'L', (char)5};
G_L_B uint8_t refalab_dgall = '\122';
void (*dgall_1)(void) = dgall_;

static void rp_(void)
{
    const T_STATUS_TABLE *status_table = refal.current_status_table;
    T_LINKCB *p = refal.previous_argument;
    bool fail = false;
    while (p->tag != TAGO || p->info.infoc != '=')
    {
        p = p->next;
        if (p == refal.next_argument)
        {
            fail = true;
            break;
        }
    };
    if (!fail)
    {
        T_LINKCB *right_bracket = status_table->store;
        while (true)
        {
            T_LINKCB *left_bracket = right_bracket->next;
            if (left_bracket == status_table->store)
            {
                if (!insert_from_free_memory(status_table->store, 2))
                {
                    refal.upshot = 3;
                    return;
                }; // LACK
                left_bracket = status_table->store->next;
                right_bracket = left_bracket->next;
                left_bracket->info.codep = right_bracket;
                left_bracket->tag = TAGLB;
                right_bracket->info.codep = left_bracket;
                right_bracket->tag = TAGRB;
                transplantation(left_bracket, refal.previous_argument, refal.next_argument);
            }
            else
            {
                if (left_bracket->tag != TAGLB)
                    break;
                right_bracket = left_bracket->info.codep;
                T_LINKCB *after_duplicate = find_duplicate(refal.previous_argument, p, left_bracket);
                if (after_duplicate == NULL)
                    continue;
                if (after_duplicate->tag != TAGO || after_duplicate->info.infoc != '=')
                    continue;
                insert_to_free_memory(after_duplicate, right_bracket);
                transplantation(after_duplicate, p, refal.next_argument);
            }
            return;
        }
    }
    refal.upshot = 2;
    return;
}
char rp_0[] = {Z2 'R', 'P', (char)2};
G_L_B uint8_t refalab_rp = '\122';
void (*rp_1)(void) = rp_;

static void cp_(void)
{
    const T_STATUS_TABLE *status_table = refal.current_status_table;
    T_LINKCB *right_bracket = status_table->store;
    T_LINKCB *after_duplicate;
    while (true)
    {
        const T_LINKCB *left_bracket = right_bracket->next;
        if (left_bracket == status_table->store)
            return;
        if (left_bracket->tag != TAGLB)
        {
            refal.upshot = 2;
            return;
        }; // FAIL
        right_bracket = left_bracket->info.codep;
        after_duplicate = find_duplicate(refal.previous_argument, refal.next_argument, left_bracket);
        if (after_duplicate == NULL)
            continue;
        if (after_duplicate->tag != TAGO || after_duplicate->info.infoc != '=')
            continue;
        break;
    }
    if (!copy_expression(refal.previous_result, after_duplicate, right_bracket))
        refal.upshot = 3; // LACK
    return;
}
char cp_0[] = {Z2 'C', 'P', (char)2};
G_L_B uint8_t refalab_cp = '\122';
void (*cp_1)(void) = cp_;

//----------------- end of file  XSTOR.C ---------------
