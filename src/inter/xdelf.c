// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-05-29
// https://github.com/Aleksandr3Bocharov/refalab

//-------------- file -- XDELF.C ------------
//               MO: delf
//-------------------------------------------

#include <stddef.h>
#include <stdbool.h>
#include "refalab.h"
#include "interface.h"

static void delf_(void)
{
    if (refal.previous_argument->next != refal.next_argument)
    {
        refal.upshot = 2;
        return;
    } // FAIL
    const T_LINKCB *delete_dot = refal.next_argument;
    const T_LINKCB *temp_k;
    const T_LINKCB *delete_dot_next;
    while (true)
    {
        temp_k = delete_dot->info.codep;
        delete_dot = temp_k->info.codep;
        if (delete_dot == NULL)
            refal_abort_end("delf: sign '#' missing");
        delete_dot_next = delete_dot->next;
        if (delete_dot_next->info.infoc != '#')
            continue;
        break;
    }
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
    refal.next_result->info.codep = temp_dot;
    return;
}
char delf_0[] = {Z4 'D', 'E', 'L', 'F', (char)4};
G_L_B uint8_t refalab_delf = '\122';
void (*delf_1)(void) = delf_;

//-------------------- end of file  XDELF.C ---------------
