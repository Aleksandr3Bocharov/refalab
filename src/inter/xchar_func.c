// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-07-10
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file xchar_func.c  -----------
//   MO: Ftochar, Functab, Chartof
//------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include "refalab.h"
#include "interface.h"

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
        if (!extended_insert_from_free_memory(refal.next_result, *label_length - 2))
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
    T_LINKCB *current_symbol_char = refal.previous_argument->next;
    size_t i;
    bool impossible = false;
    if (current_symbol_char == refal.next_argument)
        impossible = true;
    else
    {
        for (i = 0; current_symbol_char != refal.next_argument; i++, current_symbol_char = current_symbol_char->next)
            if (current_symbol_char->tag != TAGO)
            {
                impossible = true;
                break;
            }
        if (i > UINT8_MAX)
            impossible = true;
    }
    if (impossible)
    {
        refal.upshot = 2;
        return;
    }
    current_symbol_char = refal.previous_argument->next;
    char *label = (char *)malloc(i + 2);
    if (label == NULL)
        refal_abort_end("chartof: malloc error");
    for (i = 0; current_symbol_char != refal.next_argument; i++, current_symbol_char = current_symbol_char->next)
        label[i] = (char)toupper((unsigned char)current_symbol_char->info.infoc);
    label[i] = (char)i;
    i++;
    label[i] = '\002'; // EMPTY
    for (size_t k = 0; k < functions_count; k++)
    {
        const uint8_t *label_length = functions_table[k] - 1;
        if (i == (size_t)*label_length + 1 && strncmp(label, (char *)label_length - *label_length, *label_length) == 0)
        {
            // identificator iz tablicy ne preobr. w zaglawnye!!!
            // poetomu w m.o. imja d.b. napisano zaglawnymi!
            refal.previous_argument->info.codef = functions_table[k];
            transplantation(refal.previous_result, refal.previous_argument->previous, refal.previous_argument->next);
            free(label);
            return;
        }
    }
    uint8_t **temp_functions_table = NULL;
    if (functions_count == 0)
        temp_functions_table = (uint8_t **)malloc(sizeof(uint8_t *));
    else
        temp_functions_table = (uint8_t **)realloc(functions_table, (functions_count + 1) * sizeof(uint8_t *));
    if (temp_functions_table == NULL)
        refal_abort_end("chartof: malloc or realloc error");
    functions_table = temp_functions_table;
    uint8_t *function = (uint8_t *)(label + i);
    functions_table[functions_count] = function;
    functions_count++;
    refal.previous_argument->info.codef = function;
    transplantation(refal.previous_result, refal.previous_argument->previous, refal.previous_argument->next);
    return;
}
char chartof_0[] = {Z7 'C', 'H', 'A', 'R', 'T', 'O', 'F', (char)7};
G_L_B uint8_t refalab_chartof = '\122';
void (*chartof_1)(void) = chartof_;

//----------  end of file xchar_func.c  -----------
