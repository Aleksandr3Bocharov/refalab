// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-10-01
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

typedef struct pending_address
{
    size_t offset;
    T_LABEL *label;
} T_PENDING_ADDRESS;

// Uniqe specifier table
typedef struct unique_specifier
{
    uint8_t *bytes;               // bytes
    size_t length;                // length
    T_LABEL *label;               // specifier label
    T_PENDING_ADDRESS *addresses; // addresses array
    size_t address_count;         // addresses count
    struct unique_specifier *next;
} T_UNIQUE_SPECIFIER;

extern void specifier_pool_init(void);
extern void specifier_pool_clear(void);

extern void specifier_buffer_begin(void);
extern void specifier_buffer_append_byte(uint8_t byte);
extern void specifier_buffer_append_address(T_LABEL *label);
extern void specifier_buffer_append_symbol(const T_LINKTI *code);

extern T_LABEL *specifier_pool_find_or_create(void);
extern void specifier_pool_finalize(void);

extern bool specifier_pool_is_collecting(void);

#endif
