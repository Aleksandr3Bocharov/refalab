// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-11-17
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file xdelf.c  ----------
//               MO: Delf
//------------------------------------

#include <stddef.h>
#include <stdbool.h>
#include "refalab.h"
#include "interface.h"

// Delf: <Delf> - deletes a chain of function terms from the view field: from the term
// that becomes the leading one after this call up to the first term directly followed
// by the symbol '#'; the '#' is deleted too. The freed cells go back to free memory.
// The argument must be empty (FAIL otherwise); aborts when no such '#' exists.
static void delf_(void)
{
    if (refal.previous_argument->next != refal.next_argument)
    {
        refal.upshot = 2;
        return;
    } // FAIL
    // Walk the list of call brackets in evaluation order until a call followed by '#' is found.
    const T_LINKCB *delete_dot = refal.next_argument;
    const T_LINKCB *temp_k;
    const T_LINKCB *delete_dot_next;
    while (true)
    {
        temp_k = delete_dot->info.codep;
        delete_dot = temp_k->info.codep;
        if (delete_dot == NULL)
            refal_abort_end("Delf: sign '#' missing");
        delete_dot_next = delete_dot->next;
        if (delete_dot_next->info.infoc != '#')
            continue;
        break;
    }
    // Free the cells of every call of the chain except the last one.
    const T_LINKCB *current_delete_dot = refal.next_result->info.codep;
    T_LINKCB *temp_dot;
    while (current_delete_dot != delete_dot)
    {
        temp_k = current_delete_dot->info.codep;
        temp_dot = temp_k->info.codep;
        insert_to_free_memory(temp_k->previous, current_delete_dot->next);
        current_delete_dot = temp_dot;
    }
    temp_k = current_delete_dot->info.codep;
    temp_dot = temp_k->info.codep;
    insert_to_free_memory(temp_k->previous, delete_dot_next->next);
    // The next leading call is the one that followed the deleted chain.
    refal.next_result->info.codep = temp_dot;
    return;
}
char delf_0[] = {Z4 'D', 'E', 'L', 'F', (char)4};
G_L_B uint8_t refalab_delf = '\122';
void (*delf_1)(void) = delf_;

//-----------  end of file xdelf.c  ----------
