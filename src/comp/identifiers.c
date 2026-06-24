// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-06-20
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file identifiers.c  ----------
//         Work with identifier table
//------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "refalab.h"
#include "identifiers.h"
#include "print_errors.h"
#include "macrocode.h"
#include "generate_operators.h"
#include "avl_identifiers.h"
#include "compiler.h"
#include "compile_sentence.h"

#define PRINT_ERROR_504 \
    print_error_three_strings(504, "Label", identifier, identifier_length, " is already defined")

#define PRINT_ERROR_500 \
    print_error_string("500 no statement label")

#define PRINT_ERROR_501 \
    print_error_string(501, "Both labels in EQU-directive already defined")

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
            error_no_memory_labels();
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

void function_definition(const char *identifier, uint8_t identifier_length, size_t identifier_cursor_number)
{
    if (identifier_length != 0)
    { // new function
        function_end();
        T_LABEL *label = lookup_label(identifier, identifier_length, identifier_cursor_number);
        next_sentence = allocate_info_label();
        label->type |= 0100;
        if ((label->mode) & 0020)
        {
            scanner.last_error_cursor = identifier_cursor_number;
            PRINT_ERROR_504;
        }
        else
        {
            function_head(identifier, identifier_length);
            label->cursor_number_defined = identifier_cursor_number;
            macrocode_label(label);
            generate_operator_l(n_sjump, (T_LABEL *)next_sentence);
        }
    }
    else
    { //  next sentence in function
        if (next_sentence != NULL)
            macrocode_label((T_LABEL *)next_sentence);
        else
        !!!    PRINT_ERROR_500;
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
            macrocode_equ((T_LABEL *)next_sentence, (T_LABEL *)fail_sentence);
        else
        {
            fail_sentence = next_sentence;
            macrocode_label((T_LABEL *)next_sentence);
            macrocode_byte(n_fail);
        }
        next_sentence = NULL;
    }
    return;
}

void set_empty(const char *identifier, uint8_t identifier_length, size_t identifier_cursor_number)
{
    T_LABEL *label = lookup_label(identifier, identifier_length, identifier_cursor_number);
    label->type |= 0100;
    if (label->mode & 0020)
    {
        scanner.last_error_cursor = identifier_cursor_number;
        PRINT_ERROR_504;
    }
    else
    {
        function_head(identifier, identifier_length);
        label->cursor_number_defined = identifier_cursor_number;
        macrocode_label(label);
        macrocode_byte(n_fail);
    }
    return;
}

void set_swap(const char *identifier, uint8_t identifier_length, size_t identifier_cursor_number)
{
    T_LABEL *label = lookup_label(identifier, identifier_length, identifier_cursor_number);
    label->type |= 0100;
    if (label->mode & 0020)
    {
        scanner.last_error_cursor = identifier_cursor_number;
        PRINT_ERROR_504;
    }
    else
    { //  align box head on the 8-byte board
        uint8_t name_length;
        if (options.full_name)
        {
            const int full_length = (int)scanner.module_name_length + (int)identifier_length + 1;
            name_length = (uint8_t)(full_length > 255 ? 255 : full_length);
        }
        else
            name_length = identifier_length;
        uint8_t align_bytes = (macrocode_where() + name_length + 2) % 8;
        if (align_bytes != 0)
            align_bytes = 8 - align_bytes;
        for (uint8_t i = 1; i <= align_bytes; i++)
            macrocode_byte(' ');
        function_head(identifier, identifier_length);
        label->cursor_number_defined = identifier_cursor_number;
        macrocode_label(label);
        macrocode_byte(n_swap);
        const uint8_t swap_length = SMBL + LBLL * 2;
        for (uint8_t i = 1; i <= swap_length; i++)
            macrocode_byte(0);
    }
    return;
}

void set_entry(const char *identifier, uint8_t identifier_length, size_t identifier_cursor_number, const char *identifier_extern, uint8_t identifier_extern_length, size_t identifier_extern_cursor_number)
{
    T_LABEL *label = lookup_label(identifier, identifier_length, identifier_cursor_number);
    macrocode_entry(label, identifier_extern, identifier_extern_length, identifier_extern_cursor_number);
    return;
}

void set_extrn(const char *identifier, uint8_t identifier_length, size_t identifier_cursor_number, const char *identifier_extern, uint8_t identifier_extern_length, size_t identifier_extern_cursor_number)
// identifier internal name
// identifier_extern external name
{
    T_LABEL *label = lookup_label(identifier, identifier_length, identifier_cursor_number);
    if (label->mode & 0020)
    {
        scanner.last_error_cursor = identifier_cursor_number;
        PRINT_ERROR_504;
    }
    else
    {
        label->cursor_number_defined = identifier_cursor_number;
        macrocode_extrn(label, identifier_extern, identifier_extern_length, identifier_extern_cursor_number);
    }
    return;
}

T_LABEL *function_reference(const char *identifier, uint8_t identifier_length, size_t identifier_cursor_number)
{
    T_LABEL *label = lookup_label(identifier, identifier_length, identifier_cursor_number);
    label->type |= 0100;
    return label;
}

T_LABEL *specifier_reference(const char *identifier, uint8_t identifier_length, size_t identifier_cursor_number, char tail)
{
    T_LABEL *label = lookup_label(identifier, identifier_length, identifier_cursor_number);
    label->type |= 0200;
    if (tail != ')' && (label->mode & 0020) != 0020)
    {
        scanner.last_error_cursor = identifier_cursor_number;
        print_error_three_strings(505, "Label", identifier, identifier_length, " is yet not defined");
    } 
    return label;
}

void specifier_definition(const char *identifier, uint8_t identifier_length, size_t identifier_cursor_number)
{
    T_LABEL *label = lookup_label(identifier, identifier_length, identifier_cursor_number);
    label->type |= 0200;
    if (label->mode & 0020)
    {
        scanner.last_error_cursor = identifier_cursor_number;
        PRINT_ERROR_504;
    }
    else
    {
        label->cursor_number_defined = identifier_cursor_number;
        macrocode_label(label);
    }
    return;
}

void set_equ(const char *identifier1, uint8_t identifier1_length, size_t identifier1_cursor_number, const char *identifier2, uint8_t identifier2_length, size_t identifier2_cursor_number)
{
    T_LABEL *label1 = lookup_label(identifier1, identifier1_length, identifier1_cursor_number);
    T_LABEL *label2 = lookup_label(identifier2, identifier2_length, identifier2_cursor_number);
    if (label1 == label2)
        return;
    if ((label1->mode & 0300) == 0)
    {
        label2->type |= label1->type;
        label1->cursor_number_defined = identifier1_cursor_number;
        macrocode_equ(label1, label2);
    }
    else if ((label2->mode & 0300) == 0)
    {
        label1->type |= label2->type;
        label2->cursor_number_defined = identifier2_cursor_number;
        macrocode_equ(label2, label1);
    }
    else
    {
        scanner.last_error_cursor = identifier1_cursor_number;
        PRINT_ERROR_501;
        scanner.last_error_cursor = identifier2_cursor_number;
        PRINT_ERROR_501;
    }
    return;
}

static void function_head(const char *identifier, uint8_t identifier_length)
{
    if (options.names)
    {
        int name_length_add;
        if (options.full_name)
        {
            for (uint8_t i = 0; i < scanner.module_name_length; i++)
                macrocode_byte((uint8_t)*(scanner.module_name + i));
            macrocode_byte(':');
            name_length_add = (int)scanner.module_name_length + 1;
        }
        else
            name_length_add = 0;
        for (uint8_t i = 0; i < identifier_length; i++)
            macrocode_byte((uint8_t)*(identifier + i));
        const int full_length = name_length_add + identifier_length;
        macrocode_byte((uint8_t)(full_length > 255 ? 255 : full_length));
    }
    else
        macrocode_byte(0);
    return;
}

static void check_identifier(const T_LABEL *label) // check identifier attributes on confirmness
{
    const T_LABEL *not_equ_label = label;
    while ((not_equ_label->mode & 0300) == 0300)
        not_equ_label = not_equ_label->info.infop;
    if ((label->mode & 0300) == 0)
        print_error_three_strings("512 label", label->identifier, label->identifier_length, " not defined");
    if ((label->mode & 0040) == 0040 && (not_equ_label->mode & 0300) == 0200)
        print_error_three_strings("511 label", label->identifier, label->identifier_length, " both extrn and entry");
    if ((not_equ_label->type & 0300) == 0300)
        print_error_three_strings("502 label", label->identifier, label->identifier_length, " boht specifier and function");
    return;
}

void module_end(void)
{
    function_end();
    through_labels(check_identifier);
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
        fprintf(stderr, "free(module_terminate): array_info_labels=%p\n", (void *)array_info_labels);
#endif
        free(array_info_labels);
        array_info_labels = temp_array_info_labels;
    }
    labels_terminate();
    return;
}

//----------  end of file identifiers.c  ----------
