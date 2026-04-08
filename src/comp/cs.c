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

static void function_end(void);
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
        function_end();
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

static void function_end(void)
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

void set_empty(const char *identifier, uint8_t identifier_length)
{
    T_LABEL *label = lookup(identifier, identifier_length);
    label->type |= '\100';
    if (label->mode & '\020')
        PRINT_ERROR_504;
    else
    {
        fnhead(identifier, identifier_length);
        label->def = scanner.carriage_number;
        jit_label(label);
        jit_byte(n_fail);
    }
    return;
}

void set_swap(const char *identifier, uint8_t identifier_length)
{
    T_LABEL *label = lookup(identifier, identifier_length);
    label->type |= '\100';
    if (label->mode & '\020')
        PRINT_ERROR_504;
    else
    { //  align box head on the 8-byte board
        uint8_t name_length;
        if (options.full_name)
            name_length = 255 > scanner.module_name_length + identifier_length + 1 ? scanner.module_name_length + identifier_length + 1 : 255;
        else
            name_length = identifier_length;
        uint8_t align_bytes = (jit_where() + name_length + 2) % 8;
        if (align_bytes != 0)
            align_bytes = 8 - align_bytes;
        for (uint8_t i = 1; i <= align_bytes; i++)
            jit_byte(' ');
        fnhead(identifier, identifier_length);
        label->def = scanner.carriage_number;
        jit_label(label);
        jit_byte(n_swap);
        const uint8_t swap_length = SMBL + LBLL * 2;
        for (uint8_t i = 1; i <= swap_length; i++)
            jit_byte(0);
    }
    return;
}

void set_entry(const char *identifier, uint8_t identifier_length, const char *identifier_extern, uint8_t identifier_extern_length)
{
    T_LABEL *label = lookup(identifier, identifier_length);
    jit_entry(label, identifier_extern, identifier_extern_length);
    return;
}

void set_extrn(const char *identifier, uint8_t identifier_length, const char *identifier_extern, uint8_t identifier_extern_length)
// identifier internal name
// identifier_extern external name
{
    T_LABEL *label = lookup(identifier, identifier_length);
    if (label->mode & '\020')
        PRINT_ERROR_504;
    else
    {
        label->def = scanner.carriage_number;
        jit_extrn(label, identifier_extern, identifier_extern_length);
    }
    return;
}

T_LABEL *function_reference(const char *identifier, uint8_t identifier_length)
{
    T_LABEL *label = lookup(identifier, identifier_length);
    label->type |= '\100';
    return label;
}

T_LABEL *specifier_reference(const char *identifier, uint8_t identifier_length, char tail)
{
    T_LABEL *label = lookup(identifier, identifier_length);
    label->type |= '\200';
    if (tail != ')' && (label->mode & '\020') != '\020')
        print_error_three_strings("505 label", identifier, identifier_length, " is yet not defined");
    return label;
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

static void check_id(const T_LABEL *label) // check identifier attributes on confirmness
{
    const T_LABEL *not_equ_label = label;
    while ((not_equ_label->mode & '\300') == '\300')
        not_equ_label = not_equ_label->info.infop;
    if ((label->mode & '\300') == '\000')
        print_error_three_strings("512 label", label->id, label->l, " not defined");
    if ((label->mode & '\040') == '\040' && (not_equ_label->mode & '\300') == '\200')
        print_error_three_strings("511 label", label->id, label->l, " both extrn and entry");
    if ((not_equ_label->type & '\300') == '\300')
        print_error_three_strings("502 label", label->id, label->l, " boht specifier and function");
    return;
}

void s_end(void)
{
    function_end();
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
