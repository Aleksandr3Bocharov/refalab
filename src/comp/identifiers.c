// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-11-21
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file identifiers.c  ----------
//         Work with identifier table
//------------------------------------------

#if defined mdebug
#include <stdio.h>
#endif
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
#include "compile_output.h"
#include "function_pool.h"

// Error messages shared by several functions.
#define PRINT_ERROR_504(identifier, identifier_length) \
    print_error_three_strings(504, "Label", identifier, identifier_length, " is already defined")

#define PRINT_ERROR_501 \
    print_error_string(501, "Both labels in EQU-directive already defined")

// Sentence labels are allocated in blocks of 16 to avoid a malloc per label;
// the blocks are chained and released together in module_terminate.
typedef struct array_info_labels
{
    struct array_info_labels *next;
    T_INFO_LABEL info_labels[16];
} T_ARRAY_INFO_LABELS;

static T_ARRAY_INFO_LABELS *first_array_info_labels = NULL;
static uint8_t number_info_label = 15;
// The label of the sentence that is being compiled: it marks the place that follows the sentence
// in the code, where its failure continues (the entry of the next alternative). The label is
// defined by function_pool_finalize, wherever the next alternative begins.
static T_INFO_LABEL *next_sentence = NULL;

static void function_head(const char *identifier, uint8_t identifier_length);

// Takes the next free sentence label, allocating a new block when the current one is used up.
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

// Public wrapper of allocate_info_label.
T_INFO_LABEL *generate_info_label(void)
{
    T_INFO_LABEL *info_label = allocate_info_label();
    return info_label;
}

// Starts a function (when a label name was scanned) or the next sentence of the current one.
// For a new function the label must not be defined yet (error 504 otherwise);
// then a stored function is opened in the function pool and the output is switched
// to the left part of a new sentence, which is kept in memory for optimization.
void function_definition(void)
{
#if defined mdebug
    if (scanner.label_name_length != 0)
        fprintf(stderr, "function_definition: new function %.*s\n", scanner.label_name_length, scanner.label_name);
    else
        fprintf(stderr, "function_definition: next sentence of the current function\n");
#endif
    if (scanner.label_name_length != 0)
    { // new function
        T_LABEL *label = lookup_label(scanner.label_name, scanner.label_name_length, scanner.label_cursor_number);
        next_sentence = allocate_info_label();
        label->type |= 0100;
        if ((label->mode) & 0020)
        {
            scanner.last_error_cursor = scanner.label_cursor_number;
            PRINT_ERROR_504(scanner.label_name, scanner.label_name_length);
            function_pool_set_current_function(NULL);
        }
        else
        {
            label->cursor_number_defined = scanner.label_cursor_number;
            T_STORED_FUNCTION *func = function_pool_begin(label);
            function_pool_set_name(func, scanner.label_name, scanner.label_name_length);
            function_pool_set_current_function(func);
        }
        scanner.label_name_length = 0;
    }
    else
        next_sentence = allocate_info_label();
    T_STORED_FUNCTION *current_func = function_pool_get_current_function();
    if (current_func != NULL)
    {
        T_STORED_SENTENCE *sentence = function_pool_add_sentence(current_func, (T_LABEL *)next_sentence);
        if (sentence != NULL)
        {
            compile_output_set_current_sentence(sentence);
            compile_output_set_mode(OUTPUT_LEFT_PART, sentence);
        }
    }
    return;
}

// Ends the function: forgets its last sentence label and returns the output to the macrocode text.
void function_end(void)
{
#if defined mdebug
    fprintf(stderr, "function_end: next_sentence=%p\n", (void *)next_sentence);
#endif
    next_sentence = NULL;
    compile_output_switch_to_macrocode();
    return;
}

// $EMPTY: defines a function that consists of the FAIL statement only.
void set_empty(const char *identifier, uint8_t identifier_length, size_t identifier_cursor_number)
{
#if defined mdebug
    fprintf(stderr, "set_empty: %.*s\n", identifier_length, identifier);
#endif
    T_LABEL *label = lookup_label(identifier, identifier_length, identifier_cursor_number);
    label->type |= 0100;
    if (label->mode & 0020)
    {
        scanner.last_error_cursor = identifier_cursor_number;
        PRINT_ERROR_504(identifier, identifier_length);
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

// $SWAP: defines a box. Its head is padded with spaces so that it is aligned to
// 8 bytes, and the box body is filled with zeros.
void set_swap(const char *identifier, uint8_t identifier_length, size_t identifier_cursor_number)
{
#if defined mdebug
    fprintf(stderr, "set_swap: %.*s\n", identifier_length, identifier);
#endif
    T_LABEL *label = lookup_label(identifier, identifier_length, identifier_cursor_number);
    label->type |= 0100;
    if (label->mode & 0020)
    {
        scanner.last_error_cursor = identifier_cursor_number;
        PRINT_ERROR_504(identifier, identifier_length);
    }
    else
    { //  align box head on the 8-byte board
        uint8_t name_length;
        if (options.full_name)
        {
            const int full_length = (int)scanner.module_name_length + (int)identifier_length + 1;
            name_length = (uint8_t)(full_length > UINT8_MAX ? UINT8_MAX : full_length);
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

// $ENTRY: the label becomes visible to other modules under the external name.
void set_entry(const char *identifier, uint8_t identifier_length, size_t identifier_cursor_number, const char *identifier_extern, uint8_t identifier_extern_length, size_t identifier_extern_cursor_number)
{
#if defined mdebug
    fprintf(stderr, "set_entry: %.*s external name=%.*s\n", identifier_length, identifier, identifier_extern_length, identifier_extern);
#endif
    T_LABEL *label = lookup_label(identifier, identifier_length, identifier_cursor_number);
    macrocode_entry(label, identifier_extern, identifier_extern_length, identifier_extern_cursor_number);
    return;
}

// $EXTRN: the label is defined in another module under the external name.
void set_extrn(const char *identifier, uint8_t identifier_length, size_t identifier_cursor_number, const char *identifier_extern, uint8_t identifier_extern_length, size_t identifier_extern_cursor_number)
// identifier internal name
// identifier_extern external name
{
#if defined mdebug
    fprintf(stderr, "set_extrn: %.*s external name=%.*s\n", identifier_length, identifier, identifier_extern_length, identifier_extern);
#endif
    T_LABEL *label = lookup_label(identifier, identifier_length, identifier_cursor_number);
    if (label->mode & 0020)
    {
        scanner.last_error_cursor = identifier_cursor_number;
        PRINT_ERROR_504(identifier, identifier_length);
    }
    else
    {
        label->cursor_number_defined = identifier_cursor_number;
        macrocode_extrn(label, identifier_extern, identifier_extern_length, identifier_extern_cursor_number);
    }
    return;
}

// A use of a function name: finds or creates the label and marks it as a function.
T_LABEL *function_reference(const char *identifier, uint8_t identifier_length, size_t identifier_cursor_number)
{
    T_LABEL *label = lookup_label(identifier, identifier_length, identifier_cursor_number);
    label->type |= 0100;
    return label;
}

// A use of a specifier name: marks the label as a specifier. A specifier used before
// its definition is an error (505) unless it is the tail of a bracketed definition.
T_LABEL *specifier_reference(const char *identifier, uint8_t identifier_length, size_t identifier_cursor_number, char tail)
{
    T_LABEL *label = lookup_label(identifier, identifier_length, identifier_cursor_number);
    label->type |= 0200;
    if (tail != ')' && (label->mode & 0020) != 0020)
    {
        scanner.last_error_cursor = identifier_cursor_number;
        print_error_three_strings(505, "Label", identifier, identifier_length, " is not yet defined");
    }
    return label;
}

// A definition of a specifier name; its address is set when the module ends.
T_LABEL *specifier_definition(void)
{
    T_LABEL *label = lookup_label(scanner.label_name, scanner.label_name_length, scanner.label_cursor_number);
    label->type |= 0200;
    if (label->mode & 0020)
    {
        scanner.last_error_cursor = scanner.label_cursor_number;
        PRINT_ERROR_504(scanner.label_name, scanner.label_name_length);
    }
    else
    {
        label->cursor_number_defined = scanner.label_cursor_number;
        label->mode |= 0120; // the address is set at the end of the module (specifier_pool_finalize)
    }
    return label;
}

// $EQU: makes one label an alias of the other. Error 501 when both are defined already.
void set_equ(const char *identifier1, uint8_t identifier1_length, size_t identifier1_cursor_number, const char *identifier2, uint8_t identifier2_length, size_t identifier2_cursor_number)
{
#if defined mdebug
    fprintf(stderr, "set_equ: %.*s = %.*s\n", identifier1_length, identifier1, identifier2_length, identifier2);
#endif
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

// Writes the head of a function: the name (with the module name when the full-name
// option is set) and its length byte, or a zero byte when names are switched off.
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
        macrocode_byte((uint8_t)(full_length > UINT8_MAX ? UINT8_MAX : full_length));
    }
    else
        macrocode_byte(0);
    return;
}

// Checks the attributes of a label at the end of the module:
// it is defined (512), is not both extern and entry (511) and
// is not used both as a function and as a specifier (502).
static void check_identifier(T_LABEL *label) // check identifier attributes on confirmness
{
    T_LABEL *not_equ_label = resolve_label_alias(label);
    if ((label->mode & 0300) == 0)
    {
        scanner.last_error_cursor = label->usage_list.cursor_numbers[0];
        print_error_three_strings(512, "Label", label->identifier, label->identifier_length, " not defined");
    }
    if ((label->mode & 0040) == 0040 && (not_equ_label->mode & 0300) == 0200)
    {
        scanner.last_error_cursor = label->usage_list.cursor_numbers[0];
        print_error_three_strings(511, "Label", label->identifier, label->identifier_length, " both extrn and entry");
    }
    if ((not_equ_label->type & 0300) == 0300)
    {
        scanner.last_error_cursor = label->usage_list.cursor_numbers[0];
        print_error_three_strings(502, "Label", label->identifier, label->identifier_length, " both specifier and function");
    }
    return;
}

// Checks every label of the module.
void module_end(void)
{
    through_labels(check_identifier);
    return;
}

// Initializes the label allocator and the sentence label chain.
void module_init(void)
{ // module initiation
    first_array_info_labels = NULL;
    number_info_label = 15;
    next_sentence = NULL;
    return;
}

// Releases the sentence label blocks and the identifier table.
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
