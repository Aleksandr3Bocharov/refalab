// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-10-05
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file function_pool.c  ----------
//    pool of functions for prefix optimization
//--------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "refalab.h"
#include "function_pool.h"
#include "avl_identifiers.h"
#include "compile_output.h"
#include "print_errors.h"
#include "macrocode.h"
#include "generate_operators.h"
#include "compiler.h"

static T_STORED_FUNCTION *function_pool_head = NULL;
static T_STORED_FUNCTION *current_function = NULL;

void function_pool_init(void)
{
    function_pool_head = NULL;
    current_function = NULL;
    return;
}

static void clear_operator_buffer(T_OPERATOR_BUFFER *buf)
{
    if (buf->bytes != NULL)
    {
#if defined mdebug
        fprintf(stderr, "free(clear_operator_buffer): bytes=%p\n", (void *)buf->bytes);
#endif
        free(buf->bytes);
    }
    if (buf->labels != NULL)
    {
#if defined mdebug
        fprintf(stderr, "free(clear_operator_buffer): labels=%p\n", (void *)buf->labels);
#endif
        free(buf->labels);
    }
    buf->bytes = NULL;
    buf->length = 0;
    buf->capacity = 0;
    buf->labels = NULL;
    buf->label_count = 0;
    buf->label_capacity = 0;
    return;
}

void function_pool_clear(void)
{
    T_STORED_FUNCTION *func = function_pool_head;
    while (func != NULL)
    {
        T_STORED_FUNCTION *next_func = func->next;
        if (func->func_name != NULL)
        {
#if defined mdebug
            fprintf(stderr, "free(function_pool_clear): func_name=%p\n", (void *)func->func_name);
#endif
            free(func->func_name);
        }
        T_STORED_SENTENCE *sentence = func->sentences;
        while (sentence != NULL)
        {
            T_STORED_SENTENCE *next_sentence = sentence->next;
            clear_operator_buffer(&sentence->left_part);
            clear_operator_buffer(&sentence->right_part);
#if defined mdebug
            fprintf(stderr, "free(function_pool_clear): sentence=%p\n", (void *)sentence);
#endif
            free(sentence);
            sentence = next_sentence;
        }
#if defined mdebug
        fprintf(stderr, "free(function_pool_clear): func=%p\n", (void *)func);
#endif
        free(func);
        func = next_func;
    }
    function_pool_head = NULL;
    return;
}

T_STORED_FUNCTION *function_pool_get_current_function(void)
{
    return current_function;
}

void function_pool_set_current_function(T_STORED_FUNCTION *func)
{
    current_function = func;
    return;
}

T_STORED_FUNCTION *function_pool_begin(T_LABEL *label)
{
    T_STORED_FUNCTION *func = (T_STORED_FUNCTION *)calloc(1, sizeof(T_STORED_FUNCTION));
    if (func == NULL)
        error_no_memory();
#if defined mdebug
    fprintf(stderr, "calloc(function_pool_begin): func=%p label=%p\n", (void *)func, (void *)label);
#endif
    func->func_label = label;
    func->func_name = NULL;
    func->func_name_length = 0;
    func->sentences = NULL;
    func->last_sentence = NULL;
    func->sentence_count = 0;
    func->fail_label = NULL;
    func->next = function_pool_head;
    function_pool_head = func;
    return func;
}

void function_pool_set_name(T_STORED_FUNCTION *func, const char *name, uint8_t name_length)
{
    if (func->func_name != NULL)
    {
#if defined mdebug
        fprintf(stderr, "free(function_pool_set_name): func_name=%p\n", (void *)func->func_name);
#endif
        free(func->func_name);
        func->func_name = NULL;
    }
    if (!options.names)
    {
        func->func_name = NULL;
        func->func_name_length = 0;
        return;
    }
    size_t total_length;
    char *full_name;
    if (options.full_name)
    {
        // module_name + ':' + identifier
        total_length = (size_t)scanner.module_name_length + 1 + name_length;
        full_name = (char *)malloc(total_length);
        if (full_name == NULL)
            error_no_memory();
#if defined mdebug
        fprintf(stderr, "malloc(function_pool_set_name): full_name=%p length=%zu (full)\n", (void *)full_name, total_length);
#endif
        memcpy(full_name, scanner.module_name, scanner.module_name_length);
        full_name[scanner.module_name_length] = ':';
        memcpy(full_name + scanner.module_name_length + 1, name, name_length);
    }
    else
    {
        // only identifier
        total_length = name_length;
        full_name = (char *)malloc(total_length);
        if (full_name == NULL)
            error_no_memory();
#if defined mdebug
        fprintf(stderr, "malloc(function_pool_set_name): full_name=%p length=%zu\n", (void *)full_name, total_length);
#endif
        memcpy(full_name, name, name_length);
    }
    func->func_name = full_name;
    func->func_name_length = (uint8_t)(total_length > UINT8_MAX ? UINT8_MAX : total_length);
    return;
}

T_STORED_SENTENCE *function_pool_add_sentence(T_STORED_FUNCTION *func, T_LABEL *sentence_label)
{
    if (func == NULL)
        return NULL;
    T_STORED_SENTENCE *sentence = (T_STORED_SENTENCE *)calloc(1, sizeof(T_STORED_SENTENCE));
    if (sentence == NULL)
        error_no_memory();
#if defined mdebug
    fprintf(stderr, "calloc(function_pool_add_sentence): sentence=%p label=%p\n", (void *)sentence, (void *)sentence_label);
#endif
    sentence->sentence_label = sentence_label;
    sentence->left_part.bytes = NULL;
    sentence->left_part.length = 0;
    sentence->left_part.capacity = 0;
    sentence->left_part.labels = NULL;
    sentence->left_part.label_count = 0;
    sentence->left_part.label_capacity = 0;
    sentence->right_part.bytes = NULL;
    sentence->right_part.length = 0;
    sentence->right_part.capacity = 0;
    sentence->right_part.labels = NULL;
    sentence->right_part.label_count = 0;
    sentence->right_part.label_capacity = 0;
    sentence->next = NULL;
    if (func->last_sentence != NULL)
        func->last_sentence->next = sentence;
    else
        func->sentences = sentence;
    func->last_sentence = sentence;
    func->sentence_count++;
    return sentence;
}

void function_pool_set_output_mode(T_STORED_SENTENCE *sentence, bool is_left_part)
{
    if (sentence == NULL)
        compile_output_set_mode(OUTPUT_MACROCODE, NULL);
    else
    {
        const T_OUTPUT_MODE mode = is_left_part ? OUTPUT_LEFT_PART : OUTPUT_RIGHT_PART;
        compile_output_set_mode(mode, sentence);
    }
    return;
}

static void ensure_capacity(T_OPERATOR_BUFFER *buf, size_t needed)
{
    if (buf->capacity < needed)
    {
        size_t new_capacity = buf->capacity == 0 ? 64 : buf->capacity * 2;
        while (new_capacity < needed)
            new_capacity *= 2;
        uint8_t *new_bytes = (uint8_t *)realloc(buf->bytes, new_capacity);
        if (new_bytes == NULL)
            error_no_memory();
#if defined mdebug
        fprintf(stderr, "realloc(ensure_capacity): bytes=%p capacity=%zu\n", (void *)new_bytes, new_capacity);
#endif
        buf->bytes = new_bytes;
        buf->capacity = new_capacity;
    }
    return;
}

static void ensure_label_capacity(T_OPERATOR_BUFFER *buf, size_t needed)
{
    if (buf->label_capacity < needed)
    {
        size_t new_capacity = buf->label_capacity == 0 ? 8 : buf->label_capacity * 2;
        while (new_capacity < needed)
            new_capacity *= 2;
        T_LABEL_REF *new_labels = (T_LABEL_REF *)realloc(buf->labels, new_capacity * sizeof(T_LABEL_REF));
        if (new_labels == NULL)
            error_no_memory();
#if defined mdebug
        fprintf(stderr, "realloc(ensure_label_capacity): labels=%p capacity=%zu\n", (void *)new_labels, new_capacity);
#endif
        buf->labels = new_labels;
        buf->label_capacity = new_capacity;
    }
    return;
}

void sentence_append_byte_left(T_STORED_SENTENCE *sentence, uint8_t byte)
{
    T_OPERATOR_BUFFER *buf = &sentence->left_part;
    ensure_capacity(buf, buf->length + 1);
    buf->bytes[buf->length++] = byte;
    return;
}

void sentence_append_byte_right(T_STORED_SENTENCE *sentence, uint8_t byte)
{
    T_OPERATOR_BUFFER *buf = &sentence->right_part;
    ensure_capacity(buf, buf->length + 1);
    buf->bytes[buf->length++] = byte;
    return;
}

void sentence_append_address_left(T_STORED_SENTENCE *sentence, T_LABEL *label)
{
    T_OPERATOR_BUFFER *buf = &sentence->left_part;
    ensure_capacity(buf, buf->length + LBLL);
    ensure_label_capacity(buf, buf->label_count + 1);
    buf->labels[buf->label_count].offset = buf->length;
    buf->labels[buf->label_count].label = label;
    buf->label_count++;
    memset(buf->bytes + buf->length, 0, LBLL);
    buf->length += LBLL;
    return;
}

void sentence_append_address_right(T_STORED_SENTENCE *sentence, T_LABEL *label)
{
    T_OPERATOR_BUFFER *buf = &sentence->right_part;
    ensure_capacity(buf, buf->length + LBLL);
    ensure_label_capacity(buf, buf->label_count + 1);
    buf->labels[buf->label_count].offset = buf->length;
    buf->labels[buf->label_count].label = label;
    buf->label_count++;
    memset(buf->bytes + buf->length, 0, LBLL);
    buf->length += LBLL;
    return;
}

static void write_buffer_to_macrocode(T_OPERATOR_BUFFER *buf)
{
    size_t pos = 0;
    size_t label_idx = 0;
    while (pos < buf->length)
    {
        if (label_idx < buf->label_count && buf->labels[label_idx].offset == pos)
        {
            macrocode_address(buf->labels[label_idx].label);
            pos += LBLL;
            label_idx++;
        }
        else
        {
            macrocode_byte(buf->bytes[pos]);
            pos++;
        }
    }
    return;
}

static void write_function_name(T_STORED_FUNCTION *func)
{
    if (func->func_name != NULL)
    {
        for (uint8_t i = 0; i < func->func_name_length; i++)
            macrocode_byte((uint8_t)func->func_name[i]);
        macrocode_byte(func->func_name_length);
    }
    else
        macrocode_byte(0);
    return;
}

void function_pool_finalize(void)
{
#if defined mdebug
    fprintf(stderr, "function_pool_finalize: begin\n");
#endif
    compile_output_set_mode(OUTPUT_MACROCODE, NULL);
    T_STORED_FUNCTION *func = function_pool_head;
    while (func != NULL)
    {
#if defined mdebug
        fprintf(stderr, "  function: label=%p sentences=%zu\n", (void *)func->func_label, func->sentence_count);
#endif
        write_function_name(func);
        macrocode_label(func->func_label);
        T_STORED_SENTENCE *sentence = func->sentences;
        if (sentence != NULL)
            generate_operator_l(n_sjump, sentence->sentence_label);
        while (sentence != NULL)
        {
#if defined mdebug
            fprintf(stderr, "    sentence: label=%p left=%zu right=%zu\n", (void *)sentence->sentence_label, sentence->left_part.length, sentence->right_part.length);
#endif
            write_buffer_to_macrocode(&sentence->left_part);
            write_buffer_to_macrocode(&sentence->right_part);
            bool is_alias = ((sentence->sentence_label->mode & 0300) == 0300);
            if (!is_alias)
            {
                macrocode_label(sentence->sentence_label);
                if (sentence->next != NULL)
                    generate_operator_l(n_sjump, sentence->next->sentence_label);
                else
                    macrocode_byte(n_fail);
            }
#if defined mdebug
            else
                fprintf(stderr, "      SKIPPED label (alias to %p)\n", (void *)sentence->sentence_label->info.infop);
#endif
            sentence = sentence->next;
        }
        func = func->next;
    }
#if defined mdebug
    fprintf(stderr, "function_pool_finalize: end\n");
#endif
    return;
}

//----------  end of file function_pool.c  ----------
