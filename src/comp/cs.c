// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-04-07
// https://github.com/Aleksandr3Bocharov/refalab

//-------------  file  --  CS.C  ---------------
//         Work with identifier table
//----------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "refalab.h"
#include "cs.h"
#include "cerr.h"
#include "cj.h"
#include "cgop.h"
#include "clu.h"
#include "refal.h"
#include "ccst.h"

#define PRINT_ERROR_504 \
    print_error_three_strings("504 label", identifier, identifier_length, " is already defined")

#define PRINT_ERROR_500 \
    print_error_string("500 no statement label")

typedef struct array_info_labels
{
    struct array_info_labels *next;
    T_INFO_LABEL info_labels[16];
} T_ARRAY_INFO_LABELS;

static T_ARRAY_INFO_LABELS *first_array_info_labels = NULL;
static uint8_t number_info_label = 15;
static T_INFO_LABEL *fail_sentence = NULL; // sentence FAIL label
static T_INFO_LABEL *next_sentence = NULL; // next sentence label

static void func_end(void);
static void fnhead(const char *idp, size_t lid);

static T_INFO_LABEL *allocate_info_label(void)
{
    if (number_info_label == 15)
    {
        T_ARRAY_INFO_LABELS *new_array_info_labels = calloc(1, sizeof(T_ARRAY_INFO_LABELS));
#if defined mdebug
        fprintf(stderr, "calloc(cs)_lbl: new_array_info_labels=%p\n", (void *)new_array_info_labels);
#endif
        if (new_array_info_labels == NULL)
            uns_sto();
        else
        {
            new_array_info_labels->next = first_array_info_labels;
            first_array_info_labels = new_array_info_labels;
            number_info_label = 255;
        }
    }
    ++number_info_label;
    T_INFO_LABEL *info_label = &first_array_info_labels->info_labels[number_info_label];
    info_label->mode = 0;
    return info_label;
}

T_INFO_LABEL *generate_info_label(void)
{
    T_INFO_LABEL *info_label = allocate_info_label();
    return info_label;
}

void function_definition(const char *identifier, uint8_t identifier_length)
{
    if (identifier_length != 0)
    { // new function
        func_end();
        T_LABEL *label = lookup(identifier, identifier_length);
        next_sentence = allocate_info_label();
        label->type |= '\100';
        if ((label->mode) & '\020')
            PRINT_ERROR_504;
        else
        {
            fnhead(identifier, identifier_length);
            label->def = scanner.carriage_number;
            jit_label(label);
            generate_operator_l(n_sjump, (T_LABEL *)next_sentence);
        }
    }
    else
    { //  next sentence in function
        if (next_sentence != NULL)
            jit_label((T_LABEL *)next_sentence);
        else
            PRINT_ERROR_500;
        next_sentence = allocate_info_label();
        generate_operator_l(n_sjump, (T_LABEL *)next_sentence);
    };
    return;
}

static void func_end(void)
{
    if (next_sentence != NULL)
    {
        if (fail_sentence != NULL)
            jit_equ((T_LABEL *)next_sentence, (T_LABEL *)fail_sentence);
        else
        {
            fail_sentence = next_sentence;
            jit_label((T_LABEL *)next_sentence);
            jit_byte(n_fail);
        }
        next_sentence = NULL;
    }
    return;
}

void sempty(const char *idp, uint8_t lid)
{
    T_LABEL *p = lookup(idp, lid);
    p->type = (p->type) | '\100';
    if (p->mode & '\020')
        PRINT_ERROR_504;
    else
    {
        fnhead(idp, lid);
        p->def = scanner.carriage_number;
        jit_label(p);
        jit_byte(n_fail);
    }
    return;
}

void sswap(const char *idp, uint8_t lid)
{
    T_LABEL *p = lookup(idp, lid);
    p->type = p->type | '\100';
    if (p->mode & '\020')
        PRINT_ERROR_504;
    else
    { //  align box head on the 8-byte board
        size_t j0 = jit_where();
        size_t l0;
        if (options.full_name)
            l0 = 255 > scanner.module_name_length + lid + 1 ? scanner.module_name_length + lid + 1 : 255;
        else
            l0 = lid;
        j0 = (j0 + l0 + 2) % 8;
        if (j0 != 0)
            j0 = 8 - j0;
        for (size_t k0 = 1; k0 <= j0; k0++)
            jit_byte(' ');
        fnhead(idp, lid);
        p->def = scanner.carriage_number;
        jit_label(p);
        jit_byte(n_swap);
        const size_t kk = SMBL + LBLL * 2;
        for (size_t k0 = 1; k0 <= kk; k0++)
            jit_byte('\000');
    }
    return;
}

void sentry(const char *idp, uint8_t lidp, const char *ide, uint8_t lide)
{
    T_LABEL *p = lookup(idp, lidp);
    jit_entry(p, ide, lide);
    return;
}

void sextrn(const char *idp, uint8_t lidp, const char *ide, uint8_t lide)
// idp internal name
// ide external name
{
    T_LABEL *p = lookup(idp, lidp);
    if (p->mode & '\020')
        PRINT_ERROR_504;
    else
    {
        p->def = scanner.carriage_number;
        jit_extrn(p, ide, lide);
    }
    return;
}

T_LABEL *fnref(const char *idp, size_t lid)
{
    T_LABEL *p = lookup(idp, lid);
    p->type |= '\100';
    return p;
}

T_LABEL *spref(const char *idp, size_t lid, char d)
{
    T_LABEL *p = lookup(idp, lid);
    p->type |= '\200';
    if (d != ')' && (p->mode & '\020') != '\020')
        print_error_three_strings("505 label", idp, lid, " is yet not defined");
    return p;
}

void spdef(const char *idp, size_t lid)
{
    if (lid == 0)
        PRINT_ERROR_500;
    else
    { // label exist
        T_LABEL *p = lookup(idp, lid);
        p->type |= '\200';
        if (p->mode & '\020')
            PRINT_ERROR_504;
        else
        {
            p->def = scanner.carriage_number;
            jit_label(p);
        }
    }
    return;
}

void sequ(const char *id1, size_t lid1, const char *id0, size_t lid0)
{
    T_LABEL *p0 = lookup(id0, lid0);
    if (lid1 == 0)
    {
        PRINT_ERROR_500;
        return;
    }
    T_LABEL *p1 = lookup(id1, lid1);
    if (p0 == p1)
        return;
    if ((p1->mode & '\300') == '\000')
    {
        p0->type |= p1->type;
        p1->def = scanner.carriage_number;
        jit_equ(p1, p0);
    }
    else if ((p0->mode & '\300') == '\000')
    {
        p1->type |= p0->type;
        p0->def = scanner.carriage_number;
        jit_equ(p0, p1);
    }
    else
        print_error_string("501 both labels already defined");
    return;
}

static void fnhead(const char *idp, size_t lid)
{
    if (options.names)
    {
        size_t k0, l0, ll;
        if (options.full_name)
        {
            const char *idpm = scanner.module_name;
            l0 = scanner.module_name_length;
            for (k0 = 0; k0 < l0; k0++)
                jit_byte((uint8_t)*(idpm + k0));
            jit_byte(':');
            ll = k0 + 1;
        }
        else
            ll = 0;
        l0 = lid;
        for (k0 = 0; k0 < l0; k0++)
            jit_byte((uint8_t)*(idp + k0));
        jit_byte((uint8_t)(255 < ll + l0 ? 255 : ll + l0));
    }
    else
        jit_byte(0);
    return;
}

static void check_id(const T_LABEL *pp) // check identifier attributes on confirmness
{
    const T_LABEL *q = pp;
    while ((q->mode & '\300') == '\300')
        q = q->info.infop;
    if ((pp->mode & '\300') == '\000')
        print_error_three_strings("512 label", pp->id, pp->l, " not defined");
    if ((pp->mode & '\040') == '\040' && (q->mode & '\300') == '\200')
        print_error_three_strings("511 label", pp->id, pp->l, " both extrn and entry");
    if ((q->type & '\300') == '\300')
        print_error_three_strings("502 label", pp->id, pp->l, " boht specifier and function");
    return;
}

void s_end(void)
{
    func_end();
    through(check_id);
    return;
}

void s_init(void)
{ // module initiation
    first_array_info_labels = NULL;
    number_info_label = 15;
    fail_sentence = NULL;
    next_sentence = NULL;
    return;
}

void s_term(void)
{ // module termination
    T_ARRAY_INFO_LABELS *p = first_array_info_labels;
    while (p != NULL)
    {
        T_ARRAY_INFO_LABELS *p1 = p->next;
#if defined mdebug
        fprintf(stderr, "free(cs): p=%p\n", (void *)p);
#endif
        free(p);
        p = p1;
    }
    luterm();
    return;
}

//----------  end of file CS.C  -------------------
