// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-04-28
// https://github.com/Aleksandr3Bocharov/refalab

//------------ file -- XCF.C ---------------
//   MO: ftochar, functab, chartof
//------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include "refalab.h"
#include "rfintf.h"

static size_t functions_count = 0;
static uint8_t **functions_table = NULL;

static void ftochar_(void)
{
    const T_LINKCB *symbol_label = refal.previous_argument->next;
    if (symbol_label->next != refal.next_argument || symbol_label->tag != TAGF)
    {
        refal.upshot = 2;
        return;
    }
    const uint8_t *label_length = symbol_label->info.codef - 1;
    const char *label = (char *)label_length - *label_length;
    if (*label_length > 2)
        if (!extended_insert_from_free_memory_list(refal.next_result, *label_length - 2))
            return;
    T_LINKCB *current_result = refal.next_result;
    for (uint8_t i = 0; i < *label_length; i++)
    {
        current_result = current_result->next;
        current_result->tag = TAGO;
        current_result->info.code = NULL;
        current_result->info.infoc = *(label + i);
    }
    transplantation(refal.previous_result, refal.next_result, current_result->next);
    return;
}
char ftochar_0[] = {Z7 'F', 'T', 'O', 'C', 'H', 'A', 'R', (char)7};
G_L_B uint8_t refalab_ftochar = '\122';
void (*ftochar_1)(void) = ftochar_;

static void functab_(void)
{
    const T_LINKCB *symbol_label = refal.previous_argument->next;
    if (symbol_label->next != refal.next_argument || symbol_label->tag != TAGF)
    {
        refal.upshot = 2;
        return;
    }
    uint8_t *function = symbol_label->info.codef;
    for (size_t i = 0; i < functions_count; i++)
        if (function == functions_table[i])
            return;
    uint8_t **temp_functions_table = NULL;
    if (functions_count == 0)
        temp_functions_table = (uint8_t **)malloc(sizeof(uint8_t *));
    else
        temp_functions_table = (uint8_t **)realloc(functions_table, (functions_count + 1) * sizeof(uint8_t *));
    if (temp_functions_table == NULL)
        refal_abort_end("functab: malloc or realloc error");
    functions_table = temp_functions_table;
    functions_table[functions_count] = function;
    functions_count++;
    return;
}
char functab_0[] = {Z7 'F', 'U', 'N', 'C', 'T', 'A', 'B', (char)7};
G_L_B uint8_t refalab_functab = '\122';
void (*functab_1)(void) = functab_;

static void chartof_(void)
{
    T_LINKCB *p = refal.previous_argument->next;
    size_t i;
    bool heot = false;
    if (p == refal.next_argument)
        heot = true;
    else
    {
        for (i = 0; p != refal.next_argument; i++, p = p->next)
            if (p->tag != TAGO)
            {
                heot = true;
                break;
            }
        if (i > 255)
            heot = true;
    }
    if (heot)
    {
        refal.upshot = 2;
        return;
    }
    p = refal.previous_argument->next;
    char *u = (char *)malloc(i + 2);
    if (u == NULL)
        refal_abort_end("chartof: malloc error");
    for (i = 0; p != refal.next_argument; i++, p = p->next)
        u[i] = (char)toupper(p->info.infoc);
    u[i] = (char)i;
    ++i;
    u[i] = 2; // HEOT
    uint8_t *j = (uint8_t *)(u + i);
    uint8_t l;
    const uint8_t *lp;
    for (size_t k = 0; k < functions_count; k++)
    {
        lp = functions_table[k] - 1;
        l = *lp;
        if (i == (size_t)l + 1 && strncmp(u, (char *)lp - l, l) == 0)
        {
            // identificator iz tablicy ne preobr. w zaglawnye!!!
            // poetomu w m.o. imja d.b. napisano zaglawnymi!
            p = refal.previous_argument->next;
            p->tag = TAGF;
            p->info.codef = functions_table[k];
            if (p->next != refal.next_argument)
                insert_to_free_memory_list(p, refal.next_argument);
            transplantation(refal.previous_result, p->previous, p->next);
            free(u);
            return;
        }
    }
    uint8_t *temp_func_f = NULL;
    if (functions_count == 0)
        temp_func_f = (uint8_t *)malloc(sizeof(uint8_t));
    else
        temp_func_f = (uint8_t *)realloc(functions_table, (functions_count + 1) * sizeof(uint8_t));
    if (temp_func_f == NULL)
        refal_abort_end("chartof: malloc or realloc error");
    functions_table = temp_func_f;
    functions_table[functions_count] = j;
    functions_count++;
    p = refal.previous_argument->next;
    p->tag = TAGF;
    p->info.codef = j;
    if (p->next != refal.next_argument)
        insert_to_free_memory_list(p, refal.next_argument);
    transplantation(refal.previous_result, p->previous, p->next);
    return;
}
char chartof_0[] = {Z7 'C', 'H', 'A', 'R', 'T', 'O', 'F', (char)7};
G_L_B uint8_t refalab_chartof = '\122';
void (*chartof_1)(void) = chartof_;

//------------------ end of file  XCF.C ----------------
