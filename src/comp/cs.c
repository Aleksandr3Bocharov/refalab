// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-04-09
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
static void function_head(const char *identifier, uint8_t identifier_length);

static T_INFO_LABEL *allocate_info_label(void)
{
    if (number_info_label == 15)
    {
        T_ARRAY_INFO_LABELS *new_array_info_labels = calloc(1, sizeof(T_ARRAY_INFO_LABELS));
#if defined mdebug
        fprintf(stderr, "calloc(cs): new_array_info_labels=%p\n", (void *)new_array_info_labels);
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
        T_LABEL *label = lookup_label(identifier, identifier_length);
        next_sentence = allocate_info_label();
        label->type |= '\100';
        if ((label->mode) & '\020')
            PRINT_ERROR_504;
        else
        {
            function_head(identifier, identifier_length);
            label->carriage_number_defined = scanner.carriage_number;
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
    T_LABEL *label = lookup_label(identifier, identifier_length);
    label->type |= '\100';
    if (label->mode & '\020')
        PRINT_ERROR_504;
    else
    {
        function_head(identifier, identifier_length);
        label->carriage_number_defined = scanner.carriage_number;
        jit_label(label);
        jit_byte(n_fail);
    }
    return;
}

void set_swap(const char *identifier, uint8_t identifier_length)
{
    T_LABEL *label = lookup_label(identifier, identifier_length);
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
        function_head(identifier, identifier_length);
        label->carriage_number_defined = scanner.carriage_number;
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
    T_LABEL *label = lookup_label(identifier, identifier_length);
    jit_entry(label, identifier_extern, identifier_extern_length);
    return;
}

void set_extrn(const char *identifier, uint8_t identifier_length, const char *identifier_extern, uint8_t identifier_extern_length)
// identifier internal name
// identifier_extern external name
{
    T_LABEL *label = lookup_label(identifier, identifier_length);
    if (label->mode & '\020')
        PRINT_ERROR_504;
    else
    {
        label->carriage_number_defined = scanner.carriage_number;
        jit_extrn(label, identifier_extern, identifier_extern_length);
    }
    return;
}

T_LABEL *function_reference(const char *identifier, uint8_t identifier_length)
{
    T_LABEL *label = lookup_label(identifier, identifier_length);
    label->type |= '\100';
    return label;
}

T_LABEL *specifier_reference(const char *identifier, uint8_t identifier_length, char tail)
{
    T_LABEL *label = lookup_label(identifier, identifier_length);
    label->type |= '\200';
    if (tail != ')' && (label->mode & '\020') != '\020')
        print_error_three_strings("505 label", identifier, identifier_length, " is yet not defined");
    return label;
}

void specifier_definition(const char *identifier, uint8_t identifier_length)
{
    if (identifier_length == 0)
        PRINT_ERROR_500;
    else
    { // label exist
        T_LABEL *label = lookup_label(identifier, identifier_length);
        label->type |= '\200';
        if (label->mode & '\020')
            PRINT_ERROR_504;
        else
        {
            label->carriage_number_defined = scanner.carriage_number;
            jit_label(label);
        }
    }
    return;
}

void set_equ(const char *identifier1, uint8_t identifier1_length, const char *identifier2, uint8_t identifier2_length)
{
    if (identifier1_length == 0)
    {
        PRINT_ERROR_500;
        return;
    }
    T_LABEL *label1 = lookup_label(identifier1, identifier1_length);
    T_LABEL *label2 = lookup_label(identifier2, identifier2_length);
    if (label1 == label2)
        return;
    if ((label1->mode & '\300') == '\000')
    {
        label2->type |= label1->type;
        label1->carriage_number_defined = scanner.carriage_number;
        jit_equ(label1, label2);
    }
    else if ((label2->mode & '\300') == '\000')
    {
        label1->type |= label2->type;
        label2->carriage_number_defined = scanner.carriage_number;
        jit_equ(label2, label1);
    }
    else
        print_error_string("501 both labels already defined");
    return;
}

static void function_head(const char *identifier, uint8_t identifier_length)
{
    if (options.names)
    {
        uint8_t name_length_add;
        if (options.full_name)
        {
            for (uint8_t i = 0; i < scanner.module_name_length; i++)
                jit_byte((uint8_t)*(scanner.module_name + i));
            jit_byte(':');
            name_length_add = 1;
        }
        else
            name_length_add = 0;
        for (uint8_t i = 0; i < identifier_length; i++)
            jit_byte((uint8_t)*(identifier + i));
        jit_byte(255 < scanner.module_name_length + name_length_add + identifier_length ? 255 : scanner.module_name_length + name_length_add + identifier_length);
    }
    else
        jit_byte(0);
    return;
}

static void check_identifier(const T_LABEL *label) // check identifier attributes on confirmness
{
    const T_LABEL *not_equ_label = label;
    while ((not_equ_label->mode & '\300') == '\300')
        not_equ_label = not_equ_label->info.infop;
    if ((label->mode & '\300') == '\000')
        print_error_three_strings("512 label", label->identifier, label->identifier_length, " not defined");
    if ((label->mode & '\040') == '\040' && (not_equ_label->mode & '\300') == '\200')
        print_error_three_strings("511 label", label->identifier, label->identifier_length, " both extrn and entry");
    if ((not_equ_label->type & '\300') == '\300')
        print_error_three_strings("502 label", label->identifier, label->identifier_length, " boht specifier and function");
    return;
}

void module_end(void)
{
    function_end();
    through(check_identifier);
    return;
}

void module_init(void)
{ // module initiation
    first_array_info_labels = NULL;
    number_info_label = 15;
    fail_sentence = NULL;
    next_sentence = NULL;
    return;
}

void module_terminate(void)
{ // module termination
    T_ARRAY_INFO_LABELS *array_info_labels = first_array_info_labels;
    while (array_info_labels != NULL)
    {
        T_ARRAY_INFO_LABELS *temp_array_info_labels = array_info_labels->next;
#if defined mdebug
        fprintf(stderr, "free(cs): array_info_labels=%p\n", (void *)array_info_labels);
#endif
        free(array_info_labels);
        array_info_labels = temp_array_info_labels;
    }
    luterm();
    return;
}

//----------  end of file CS.C  -------------------
