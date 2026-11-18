// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-11-17
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file xstore.c  ----------
//       MO: Br, Dg, Dgall, Rp, Cp
//-------------------------------------

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "refalab.h"
#include "interface.h"

// Checks the argument of a store function. The store holds the terms (e.Name = e.Value).
// With only_expression_name the argument is a name alone, otherwise it is a name followed by
// '=' and a value ('=' is looked for on the zero level). Returns the place after the name.
static bool legal_argument_store(bool only_expression_name, T_LINKCB **after_expression_name_pointer)
{
    T_LINKCB *current_expression_name = refal.previous_argument->next;
    while (true)
    {
        if (current_expression_name == refal.next_argument)
        {
            if (!only_expression_name)
                return false;
            else
                break;
        }
        if (current_expression_name->tag == TAGO && current_expression_name->info.infoc == '=')
        {
            if (only_expression_name)
                return false;
            else
                break;
        }
        if (current_expression_name->tag == TAGLB)
            current_expression_name = current_expression_name->info.codep;
        else
            current_expression_name = current_expression_name->next;
    }
    *after_expression_name_pointer = current_expression_name;
    return true;
}

// Br: <Br e.Name = e.Value> - puts the term (e.Name = e.Value) to the beginning of the store;
// the result is empty. Equal names may be stored several times.
static void br_(void)
{
    const bool only_expression_name = false;
    T_LINKCB *after_expression_name;
    if (!legal_argument_store(only_expression_name, &after_expression_name))
    {
        refal.upshot = 2;
        return;
    }; // FAIL
    if (!extended_insert_from_free_memory(refal.next_result, 1))
        return; // LACK
    T_LINKCB *left_bracket = refal.next_result->next;
    T_LINKCB *right_bracket = refal.previous_argument;
    left_bracket->info.codep = right_bracket;
    left_bracket->tag = TAGLB;
    right_bracket->info.codep = left_bracket;
    right_bracket->tag = TAGRB;
    transplantation(left_bracket, refal.previous_argument, refal.next_argument);
    transplantation(refal.current_status_table->store, refal.next_result, right_bracket->next);
    return;
}
char br_0[] = {Z2 'B', 'R', (char)2};
G_L_B uint8_t refalab_br = '\122';
void (*br_1)(void) = br_;

// Dg: <Dg e.Name> - takes the leftmost (the last stored) value of the name out of the store
// and returns it. The result is empty when there is no such name.
static void dg_(void)
{
    const bool only_expression_name = true;
    T_LINKCB *after_expression_name;
    if (!legal_argument_store(only_expression_name, &after_expression_name))
    {
        refal.upshot = 2;
        return;
    }; // FAIL
    const T_STATUS_TABLE *status_table = refal.current_status_table;
    T_LINKCB *right_bracket = status_table->store;
    T_LINKCB *after_duplicate;
    const T_LINKCB *left_bracket;
    while (true)
    {
        left_bracket = right_bracket->next;
        if (left_bracket == status_table->store)
            return;
        right_bracket = left_bracket->info.codep;
        after_duplicate = find_duplicate(refal.previous_argument, after_expression_name, left_bracket);
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

// Dgall: <Dgall> - all the contents of the store are the result and the store is left empty.
// FAIL when there is an argument.
static void dgall_(void)
{
    if (refal.previous_argument->next != refal.next_argument)
        refal.upshot = 2; // FAIL
    else
        transplantation(refal.previous_result, refal.current_status_table->store, refal.current_status_table->store);
    return;
}
char dgall_0[] = {Z5 'D', 'G', 'A', 'L', 'L', (char)5};
G_L_B uint8_t refalab_dgall = '\122';
void (*dgall_1)(void) = dgall_;

// Rp: <Rp e.Name = e.Value> - replaces the value in the leftmost term with this name in place;
// the term is added like Br when there is none. The result is empty.
static void rp_(void)
{
    const bool only_expression_name = false;
    T_LINKCB *after_expression_name;
    if (!legal_argument_store(only_expression_name, &after_expression_name))
    {
        refal.upshot = 2;
        return;
    }; // FAIL
    const T_STATUS_TABLE *status_table = refal.current_status_table;
    T_LINKCB *right_bracket = status_table->store;
    while (true)
    {
        T_LINKCB *left_bracket = right_bracket->next;
        if (left_bracket == status_table->store)
        {
            if (!extended_insert_from_free_memory(refal.next_result, 1))
                return; // LACK
            left_bracket = refal.next_result->next;
            right_bracket = refal.previous_argument;
            left_bracket->info.codep = right_bracket;
            left_bracket->tag = TAGLB;
            right_bracket->info.codep = left_bracket;
            right_bracket->tag = TAGRB;
            transplantation(left_bracket, refal.previous_argument, refal.next_argument);
            transplantation(status_table->store, refal.next_result, right_bracket->next);
        }
        else
        {
            right_bracket = left_bracket->info.codep;
            T_LINKCB *after_duplicate = find_duplicate(refal.previous_argument, after_expression_name, left_bracket);
            if (after_duplicate == NULL)
                continue;
            if (after_duplicate->tag != TAGO || after_duplicate->info.infoc != '=')
                continue;
            insert_to_free_memory(after_duplicate, right_bracket);
            transplantation(after_duplicate, after_expression_name, refal.next_argument);
        }
        break;
    }
    return;
}
char rp_0[] = {Z2 'R', 'P', (char)2};
G_L_B uint8_t refalab_rp = '\122';
void (*rp_1)(void) = rp_;

// Cp: <Cp e.Name> - a copy of the leftmost value of the name; the store is not changed.
// The result is empty when there is no such name.
static void cp_(void)
{
    const bool only_expression_name = true;
    T_LINKCB *after_expression_name;
    if (!legal_argument_store(only_expression_name, &after_expression_name))
    {
        refal.upshot = 2;
        return;
    }; // FAIL
    const T_STATUS_TABLE *status_table = refal.current_status_table;
    T_LINKCB *right_bracket = status_table->store;
    T_LINKCB *after_duplicate;
    while (true)
    {
        const T_LINKCB *left_bracket = right_bracket->next;
        if (left_bracket == status_table->store)
            return;
        right_bracket = left_bracket->info.codep;
        after_duplicate = find_duplicate(refal.previous_argument, after_expression_name, left_bracket);
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

//----------  end of file xstore.c  ----------
