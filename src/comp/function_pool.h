// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-11-21
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
// A reference to a label inside an operator buffer: offset of the place where the
// address of the label is to be put.
typedef struct label_ref
{
    size_t offset;
    T_LABEL *label;
} T_LABEL_REF;

// Operator's buffer
// Growing buffer with the code of one part of a sentence: the bytes of the operators,
// where every address slot (LBLL bytes) is listed in labels[] with its label.
// For the left part the offset of the first byte of every statement is also kept
// in statements[], so that the left part can be split into statements without
// knowing the format of each one (statement_count of them).
typedef struct operator_buffer
{
    uint8_t *bytes;
    size_t length;
    size_t capacity;
    T_LABEL_REF *labels;
    size_t label_count;
    size_t label_capacity;
    size_t *statements;
    size_t statement_count;
    size_t statement_capacity;
} T_OPERATOR_BUFFER;

// Sentence
// A sentence kept in memory: the left part (the pattern) and the right part
// (the result) are generated separately. sentence_label is the label placed after
// the sentence, where control goes when the pattern does not match.
typedef struct stored_sentence
{
    T_LABEL *sentence_label;      // Sentence label
    T_OPERATOR_BUFFER left_part;  // Left part operators
    T_OPERATOR_BUFFER right_part; // Right part operators
    struct stored_sentence *next;
} T_STORED_SENTENCE;

// Function
// A function kept in memory as a list of its sentences. The code of a function is
// written to the macrocode only when the module ends (function_pool_finalize),
// so the sentences can be optimized before that.
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

// The pool holds all functions of the module in the order of definition.
extern void function_pool_init(void);
extern void function_pool_clear(void);

extern T_STORED_FUNCTION *function_pool_get_current_function(void);
extern void function_pool_set_current_function(T_STORED_FUNCTION *func);

extern T_STORED_FUNCTION *function_pool_begin(T_LABEL *label);
extern void function_pool_set_name(T_STORED_FUNCTION *func, const char *name, uint8_t name_length);

extern T_STORED_SENTENCE *function_pool_add_sentence(T_STORED_FUNCTION *func, T_LABEL *sentence_label);

// Remember that a new statement starts at the end of the left part of a sentence.
extern void sentence_mark_statement_left(T_STORED_SENTENCE *sentence);
// Append a byte or an address to the left or to the right part of a sentence.
extern void sentence_append_byte_left(T_STORED_SENTENCE *sentence, uint8_t byte);
extern void sentence_append_address_left(T_STORED_SENTENCE *sentence, T_LABEL *label);
extern void sentence_append_byte_right(T_STORED_SENTENCE *sentence, uint8_t byte);
extern void sentence_append_address_right(T_STORED_SENTENCE *sentence, T_LABEL *label);

// Directs the generated code to the left or to the right part of the sentence
// (to the macrocode when the sentence is NULL).
extern void function_pool_set_output_mode(T_STORED_SENTENCE *sentence, bool is_left_part);

// Writes all stored functions to the macrocode. The common beginnings of the left
// parts of neighbouring sentences are merged first (see the comment in function_pool.c).
extern void function_pool_finalize(void);

#endif

//----------  end of file function_pool.h  ----------
