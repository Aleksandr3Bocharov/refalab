// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-10-05
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file function_pool.h  ----------
//    pool of functions for prefix optimization
//--------------------------------------------

#ifndef FUNCTION_POOL_H
#define FUNCTION_POOL_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "avl_identifiers.h"

// Labels in operator's buffer
typedef struct label_ref
{
    size_t offset;
    T_LABEL *label;
} T_LABEL_REF;

// Operator's buffer
typedef struct operator_buffer
{
    uint8_t *bytes;
    size_t length;
    size_t capacity;
    T_LABEL_REF *labels;
    size_t label_count;
    size_t label_capacity;
} T_OPERATOR_BUFFER;

// Sentence
typedef struct stored_sentence
{
    T_LABEL *sentence_label;      // Sentence label
    T_OPERATOR_BUFFER left_part;  // Left part operators
    T_OPERATOR_BUFFER right_part; // Right part operators
    struct stored_sentence *next;
} T_STORED_SENTENCE;

// Function
typedef struct stored_function
{
    T_LABEL *func_label; // Label
    char *func_name; // Name
    uint8_t func_name_length;
    T_STORED_SENTENCE *sentences; // Sentences
    T_STORED_SENTENCE *last_sentence; // Last sentence
    size_t sentence_count;
    struct stored_function *next;
} T_STORED_FUNCTION;

extern void function_pool_init(void);
extern void function_pool_clear(void);

extern T_STORED_FUNCTION *function_pool_get_current_function(void);
extern void function_pool_set_current_function(T_STORED_FUNCTION *func);

extern T_STORED_FUNCTION *function_pool_begin(T_LABEL *label);
extern void function_pool_set_name(T_STORED_FUNCTION *func, const char *name, uint8_t name_length);

extern T_STORED_SENTENCE *function_pool_add_sentence(T_STORED_FUNCTION *func, T_LABEL *sentence_label);

extern void sentence_append_byte_left(T_STORED_SENTENCE *sentence, uint8_t byte);
extern void sentence_append_address_left(T_STORED_SENTENCE *sentence, T_LABEL *label);
extern void sentence_append_byte_right(T_STORED_SENTENCE *sentence, uint8_t byte);
extern void sentence_append_address_right(T_STORED_SENTENCE *sentence, T_LABEL *label);

extern void function_pool_set_output_mode(struct stored_sentence *sentence, bool is_left_part);

extern void function_pool_finalize(void);

#endif

//----------  end of file function_pool.h  ----------
