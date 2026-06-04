// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-06-05
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file xtry.c  ---------
//                 MO: try
//----------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include "refalab.h"
#include "interface.h"
#include "interpreter.h"
#include "debugger_interface.h"

static void try_(void)
{
    T_STATUS_TABLE *new_status_table = NULL;
    size_t new_status_table_pointer_number = (size_t)&new_status_table;
    bool lack_memory = false;
    if ((new_status_table_pointer_number & 0xffff) < 200)
        refal_abort_end("try: stack overflow");
    else
    {
        new_status_table = malloc(sizeof(T_STATUS_TABLE));
        if (new_status_table == NULL)
            refal_abort_end("try: malloc error");
        else if (!create_status_table(new_status_table))
            lack_memory = true;
        else if (!extended_insert_from_free_memory(new_status_table->view, 2))
        {
            delete_status_table(new_status_table);
            lack_memory = true;
        }
    }
    if (lack_memory)
    {
        refal.upshot = 3;
        free(new_status_table);
        return;
    }
    T_LINKCB *k = new_status_table->view->next;
    T_LINKCB *dot = k->next;
    k->info.code = NULL;
    k->tag = TAGK;
    dot->info.codep = k;
    dot->tag = TAGD;
    new_status_table->dot = dot;
    transplantation(k, refal.previous_argument, refal.next_argument);
    T_STATUS_TABLE *current_status_table = refal.current_status_table;
    transplantation(new_status_table->store, current_status_table->store, current_status_table->store);
    new_status_table->step = ++current_status_table->step;
    new_status_table->stop = MAX_STOP;
#if defined mdebug
    const uint32_t step_stop = new_status_table->stop;
#endif
    do
    {
        if (status_table_debugger == NULL)
        {
#if defined mdebug
            if (new_status_table->step >= step_stop)
                new_status_table->step = 0;
            new_status_table->stop = new_status_table->step + 1;
            const T_LINKCB *current_k = new_status_table->dot->info.codep;
            const T_LINKCB *previous_current_k = current_k->previous;
            const T_LINKCB *next_current_dot = new_status_table->dot->next;
            printf(" Step: %" PRIu32 "\n", new_status_table->stop);
            print_expression_m(" Term: ", previous_current_k, next_current_dot, true);
            refal_run(new_status_table);
            if (new_status_table->state == 1)
                print_expression_m(" Result: ", previous_current_k, next_current_dot, true);
#else
            refal_run(new_status_table); // net prokrutki
            if (new_status_table->state == 1 && new_status_table->dot != NULL)
            {
                new_status_table->step = 0;
                continue;
            }
#endif
        }
        else
            (*status_table_debugger)(new_status_table); // prokrutka vkluchena
        if (new_status_table->state == 3)
            if (more_free_memory())
                new_status_table->state = 1;
    } while (new_status_table->state == 1 && new_status_table->dot != NULL);
    transplantation(current_status_table->store, new_status_table->store, new_status_table->store);
    current_status_table->step = --new_status_table->step;
#if defined mdebug
    if (status_table_debugger == NULL)
        current_status_table->stop = current_status_table->step + 1;
#endif
    transplantation(refal.previous_result, refal.previous_argument->previous, refal.previous_argument->next);
    T_LINKCB *symbol_char_result = refal.previous_result->next;
    symbol_char_result->tag = TAGO;
    symbol_char_result->info.code = NULL;
    switch (new_status_table->state)
    {
    case 1:
        symbol_char_result->info.infoc = 'N';
        transplantation(symbol_char_result, new_status_table->view, new_status_table->view);
        break;
    case 2:
        symbol_char_result->info.infoc = 'R';
        dot = new_status_table->dot;
        k = dot->info.codep;
        transplantation(symbol_char_result, k, dot);
        break;
    case 3:
        symbol_char_result->info.infoc = 'S';
        break;
    }
    delete_status_table(new_status_table);
    free(new_status_table);
    return;
}
char try_0[] = {Z3 'T', 'R', 'Y', (char)3};
G_L_B uint8_t refalab_try = '\122';
void (*try_1)(void) = try_;

//---------  end of file xtry.c  ---------
