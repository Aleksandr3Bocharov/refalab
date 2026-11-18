// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-11-17
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file specifier_pool.h  -----------
//    pool of unique specifiers optimization
//----------------------------------------------

#ifndef SPECIFIER_POOL_H
#define SPECIFIER_POOL_H

#include <stdint.h>
#include <stdbool.h>
#include "avl_identifiers.h"
#include "generate_operators.h"

// An address slot inside the specifier text: its offset and the label (a function or another specifier)
// that is to be put there.
typedef struct pending_address
{
    size_t offset;
    T_LABEL *label;
} T_PENDING_ADDRESS;

// One specifier of the pool; the pool is a singly linked list. Equal specifiers (the same bytes and
// the same labels at the same offsets) are kept once.
// Uniqe specifier table
typedef struct unique_specifier
{
    uint8_t *bytes;               // bytes
    size_t length;                // length
    T_LABEL *label;               // specifier label
    T_PENDING_ADDRESS *addresses; // addresses array
    size_t address_count;         // addresses count
    bool named;                   // named specifier ($SPEC): the label is the user's one
    bool symbol_only;             // used only by variables of type s: brackets are never checked
    struct unique_specifier *next;
} T_UNIQUE_SPECIFIER;

// The pool of the unique specifiers of a module (see specifier_pool.c): start and end.
extern void specifier_pool_init(void);
extern void specifier_pool_clear(void);

// Collecting of the text of a specifier.
extern void specifier_buffer_begin(void);
extern void specifier_buffer_append_byte(uint8_t byte);
extern void specifier_buffer_append_address(T_LABEL *label);
extern void specifier_buffer_append_symbol(const T_LINKTI *code);

// Ending of the specifier: optimization and entering into the pool. The named specifiers ($SPEC) are
// entered by specifier_pool_add_named, the abandoned text of a specifier is dropped by
// specifier_buffer_abort, and specifier_pool_finalize writes the pool to the module image.
extern T_LABEL *specifier_pool_find_or_create(bool symbol_only);
extern void specifier_pool_add_named(T_LABEL *label);
extern void specifier_buffer_abort(void);
extern void specifier_pool_finalize(void);

// True while the text of a specifier is being collected.
extern bool specifier_pool_is_collecting(void);

#endif
