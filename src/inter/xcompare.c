// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-08-27
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file xcompare.c  ----------
//   MO: Nrel, Ltn, Len, Eqn, Nen, Gen, Gtn
//   Lrel, Ltl, Lel, Eql, Nel, Gel, Gtl
//---------------------------------------

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "refalab.h"
#include "interface.h"

#define Orel 1
#define Olt 2
#define Ole 3
#define Oeq 4
#define One 5
#define Oge 6
#define Ogt 7

extern uint8_t refalab_true, refalab_false;

static void nrel_operate(uint8_t operation);
static void lrel_operate(uint8_t operation);

static void nrel_(void)
{
    nrel_operate(Orel);
    return;
}
char nrel_0[] = {Z4 'N', 'R', 'E', 'L', (char)4};
G_L_B uint8_t refalab_nrel = '\122';
void (*nrel_1)(void) = nrel_;

static void ltn_(void)
{
    nrel_operate(Olt);
    return;
}
char ltn_0[] = {Z3 'L', 'T', 'N', (char)3};
G_L_B uint8_t refalab_ltn = '\122';
void (*ltn_1)(void) = ltn_;

static void len_(void)
{
    nrel_operate(Ole);
    return;
}
char len_0[] = {Z3 'L', 'E', 'N', (char)3};
G_L_B uint8_t refalab_len = '\122';
void (*len_1)(void) = len_;

static void eqn_(void)
{
    nrel_operate(Oeq);
    return;
}
char eqn_0[] = {Z3 'E', 'Q', 'N', (char)3};
G_L_B uint8_t refalab_eqn = '\122';
void (*eqn_1)(void) = eqn_;

static void nen_(void)
{
    nrel_operate(One);
    return;
}
char nen_0[] = {Z3 'N', 'E', 'N', (char)3};
G_L_B uint8_t refalab_nen = '\122';
void (*nen_1)(void) = nen_;

static void gen_(void)
{
    nrel_operate(Oge);
    return;
}
char gen_0[] = {Z3 'G', 'E', 'N', (char)3};
G_L_B uint8_t refalab_gen = '\122';
void (*gen_1)(void) = gen_;

static void gtn_(void)
{
    nrel_operate(Ogt);
    return;
}
char gtn_0[] = {Z3 'G', 'T', 'N', (char)3};
G_L_B uint8_t refalab_gtn = '\122';
void (*gtn_1)(void) = gtn_;

static void lrel_(void)
{
    lrel_operate(Orel);
    return;
}
char lrel_0[] = {Z4 'L', 'R', 'E', 'L', (char)4};
G_L_B uint8_t refalab_lrel = '\122';
void (*lrel_1)(void) = lrel_;

static void ltl_(void)
{
    lrel_operate(Olt);
    return;
}
char ltl_0[] = {Z3 'L', 'T', 'L', (char)3};
G_L_B uint8_t refalab_ltl = '\122';
void (*ltl_1)(void) = ltl_;

static void lel_(void)
{
    lrel_operate(Ole);
    return;
}
char lel_0[] = {Z3 'L', 'E', 'L', (char)3};
G_L_B uint8_t refalab_lel = '\122';
void (*lel_1)(void) = lel_;

static void eql_(void)
{
    lrel_operate(Oeq);
    return;
}
char eql_0[] = {Z3 'E', 'Q', 'L', (char)3};
G_L_B uint8_t refalab_eql = '\122';
void (*eql_1)(void) = eql_;

static void nel_(void)
{
    lrel_operate(One);
    return;
}
char nel_0[] = {Z3 'N', 'E', 'L', (char)3};
G_L_B uint8_t refalab_nel = '\122';
void (*nel_1)(void) = nel_;

static void gel_(void)
{
    lrel_operate(Oge);
    return;
}
char gel_0[] = {Z3 'G', 'E', 'L', (char)3};
G_L_B uint8_t refalab_gel = '\122';
void (*gel_1)(void) = gel_;

static void gtl_(void)
{
    lrel_operate(Ogt);
    return;
}
char gtl_0[] = {Z3 'G', 'T', 'L', (char)3};
G_L_B uint8_t refalab_gtl = '\122';
void (*gtl_1)(void) = gtl_;

static inline uint8_t *rel_operate(uint8_t operation, int8_t compare)
{
    uint8_t *rel_result = NULL;
    switch (operation)
    {
    case Olt:
        if (compare == -1)
            rel_result = &refalab_true;
        else
            rel_result = &refalab_false;
        break;
    case Ole:
        if (compare == 1)
            rel_result = &refalab_false;
        else
            rel_result = &refalab_true;
        break;
    case Oeq:
        if (compare == 0)
            rel_result = &refalab_true;
        else
            rel_result = &refalab_false;
        break;
    case One:
        if (compare == 0)
            rel_result = &refalab_false;
        else
            rel_result = &refalab_true;
        break;
    case Oge:
        if (compare == -1)
            rel_result = &refalab_false;
        else
            rel_result = &refalab_true;
        break;
    case Ogt:
        if (compare == 1)
            rel_result = &refalab_true;
        else
            rel_result = &refalab_false;
    }
    return rel_result;
}

static void nrel_operate(uint8_t operation)
{
    const T_LINKCB *x_current = refal.previous_argument->next;
    const T_LINKCB *y_current = x_current->info.codep;
    T_BIG_NUMBER X, Y;
    if (x_current->tag != TAGLB || !read_big_numbers_expression(&X, &Y, x_current, y_current, refal.next_argument))
    {
        refal.upshot = 2;
        return;
    }
    const int8_t compare = compare_big_numbers(&X, &Y);
    if (operation == Orel)
    {
        char compare_result = '=';
        if (compare == -1)
            compare_result = '<';
        else if (compare == 1)
            compare_result = '>';
        refal.previous_argument->tag = TAGO;
        refal.previous_argument->info.code = NULL;
        refal.previous_argument->info.infoc = compare_result;
    }
    else
        refal.previous_argument->info.codef = rel_operate(operation, compare);
    transplantation(refal.previous_result, refal.previous_argument->previous, refal.next_argument);
    return;
}

static void lrel_operate(uint8_t operation)
{
    const T_LINKCB *first_begin = refal.previous_argument->next;
    if (first_begin->tag != TAGLB)
    {
        refal.upshot = 2;
        return;
    }
    const T_LINKCB *second_begin = first_begin->info.codep;
    const int8_t compare = compare_expressions_lexicographic(first_begin, second_begin, refal.next_argument);
    if (operation == Orel)
    {
        char compare_result = '=';
        if (compare == 1)
            compare_result = '>';
        else if (compare == -1)
            compare_result = '<';
        refal.previous_argument->tag = TAGO;
        refal.previous_argument->info.code = NULL;
        refal.previous_argument->info.infoc = compare_result;
    }
    else
        refal.previous_argument->info.codef = rel_operate(operation, compare);
    transplantation(refal.previous_result, refal.previous_argument->previous, refal.next_argument);
    return;
}

//----------  end of file xcompare.c  ----------
