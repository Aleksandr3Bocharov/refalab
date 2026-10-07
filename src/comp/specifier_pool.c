// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-10-07
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file specifier_pool.c  -----------
//    pool of unique specifiers optimization
//----------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include "refalab.h"
#include "specifier_pool.h"
#include "macrocode.h"
#include "generate_operators.h"
#include "print_errors.h"
#include "avl_identifiers.h"
#include "identifiers.h"

static T_UNIQUE_SPECIFIER *pool_head = NULL;

static uint8_t *spec_buffer = NULL;
static size_t spec_buffer_size = 0;
static size_t spec_buffer_capacity = 0;
static bool collecting = false;

static T_PENDING_ADDRESS *pending_addresses = NULL;
static size_t pending_count = 0;
static size_t pending_capacity = 0;

void specifier_pool_init(void)
{
    pool_head = NULL;
    spec_buffer = NULL;
    spec_buffer_size = 0;
    spec_buffer_capacity = 0;
    collecting = false;
    pending_addresses = NULL;
    pending_count = 0;
    pending_capacity = 0;
}

void specifier_pool_clear(void)
{
    T_UNIQUE_SPECIFIER *current = pool_head;
    while (current != NULL)
    {
        T_UNIQUE_SPECIFIER *next = current->next;
#if defined mdebug
        fprintf(stderr, "free(specifier_pool_clear): current=%p bytes=%p addresses=%p\n",
                (void *)current, (void *)current->bytes, (void *)current->addresses);
#endif
        free(current->bytes);
        if (current->addresses != NULL)
            free(current->addresses);
        free(current);
        current = next;
    }
    pool_head = NULL;
#if defined mdebug
    fprintf(stderr, "free(specifier_pool_clear): spec_buffer=%p pending_addresses=%p\n", (void *)spec_buffer, (void *)pending_addresses);
#endif
    free(spec_buffer);
    spec_buffer = NULL;
    spec_buffer_size = 0;
    spec_buffer_capacity = 0;
    free(pending_addresses);
    pending_addresses = NULL;
    pending_count = 0;
    pending_capacity = 0;
    collecting = false;
    return;
}

bool specifier_pool_is_collecting(void)
{
    return collecting;
}

void specifier_buffer_begin(void)
{
    collecting = true;
    spec_buffer_size = 0;
    pending_count = 0;
#if defined mdebug
    fprintf(stderr, "specifier_buffer_begin: collecting=true\n");
#endif
}

static void ensure_buffer_capacity(size_t needed)
{
    if (spec_buffer_capacity < needed)
    {
        size_t new_capacity = spec_buffer_capacity == 0 ? 64 : spec_buffer_capacity * 2;
        while (new_capacity < needed)
            new_capacity *= 2;
        uint8_t *new_buffer = realloc(spec_buffer, new_capacity);
        if (new_buffer == NULL)
            error_no_memory();
        spec_buffer = new_buffer;
        spec_buffer_capacity = new_capacity;
#if defined mdebug
        fprintf(stderr, "realloc(ensure_buffer_capacity): spec_buffer=%p spec_buffer_capacity=%zu\n", (void *)spec_buffer, spec_buffer_capacity);
#endif
    }
}

void specifier_buffer_append_byte(uint8_t byte)
{
    if (!collecting)
        return;
    ensure_buffer_capacity(spec_buffer_size + 1);
    spec_buffer[spec_buffer_size++] = byte;
#if defined mdebug
    fprintf(stderr, "specifier_buffer_append_byte: byte=0x%02X size=%zu\n", byte, spec_buffer_size);
#endif
}

void specifier_buffer_append_address(T_LABEL *label)
{
    if (!collecting)
        return;
    if (pending_count >= pending_capacity)
    {
        size_t new_capacity = pending_capacity == 0 ? 8 : pending_capacity * 2;
        T_PENDING_ADDRESS *new_pending = realloc(pending_addresses, new_capacity * sizeof(T_PENDING_ADDRESS));
        if (new_pending == NULL)
            error_no_memory();
        pending_addresses = new_pending;
        pending_capacity = new_capacity;
#if defined mdebug
        fprintf(stderr, "realloc(specifier_buffer_append_address): pending_addresses=%p pending_capacity=%zu\n", (void *)pending_addresses, pending_capacity);
#endif
    }
    pending_addresses[pending_count].offset = spec_buffer_size;
    pending_addresses[pending_count].label = label;
    pending_count++;
    ensure_buffer_capacity(spec_buffer_size + LBLL);
    memset(spec_buffer + spec_buffer_size, 0, LBLL);
    spec_buffer_size += LBLL;
#if defined mdebug
    fprintf(stderr, "specifier_buffer_append_address: label=%p offset=%zu size=%zu\n", (void *)label, spec_buffer_size - LBLL, spec_buffer_size);
#endif
}

void specifier_buffer_append_symbol(const T_LINKTI *code)
{
    if (!collecting)
        return;
    const uint8_t *tag_bytes = (const uint8_t *)&(code->tag);
    ensure_buffer_capacity(spec_buffer_size + ZBLL);
    memcpy(spec_buffer + spec_buffer_size, tag_bytes, ZBLL);
    spec_buffer_size += ZBLL;
    if (code->tag == TAGF)
    {
#if defined mdebug
        fprintf(stderr, "specifier_buffer_append_symbol: TAGF codef=%p\n", (void *)code->info.codef);
#endif
        specifier_buffer_append_address(code->info.codef);
        return;
    }
    ensure_buffer_capacity(spec_buffer_size + LBLL);
    if (code->tag == TAGO)
    {
        spec_buffer[spec_buffer_size] = code->info.infoc;
        memset(spec_buffer + spec_buffer_size + 1, 0, LBLL - 1);
#if defined mdebug
        fprintf(stderr, "specifier_buffer_append_symbol: TAGO infoc=0x%02X\n", (unsigned char)code->info.infoc);
#endif
    }
    else
    {
        const uint8_t *info_bytes = (const uint8_t *)&(code->info.coden);
        memcpy(spec_buffer + spec_buffer_size, info_bytes, ZBLL);
        memset(spec_buffer + spec_buffer_size + ZBLL, 0, LBLL - ZBLL);
#if defined mdebug
        fprintf(stderr, "specifier_buffer_append_symbol: TAGN coden=%" PRIu32 "\n", code->info.coden);
#endif
    }
    spec_buffer_size += LBLL;
}

T_LABEL *specifier_pool_find_or_create(void)
{
    collecting = false;
#if defined mdebug
    fprintf(stderr, "specifier_pool_find_or_create: searching, buffer_size=%zu\n", spec_buffer_size);
    fprintf(stderr, "  buffer content: ");
    for (size_t i = 0; i < spec_buffer_size && i < 48; i++)
        fprintf(stderr, "%02X ", spec_buffer[i]);
    if (spec_buffer_size > 48)
        fprintf(stderr, "...");
    fprintf(stderr, "\n");
#endif
    T_UNIQUE_SPECIFIER *current = pool_head;
    while (current != NULL)
    {
        if (current->length != spec_buffer_size)
        {
#if defined mdebug
            fprintf(stderr, "  skip spec at %p: length mismatch (%zu != %zu)\n", (void *)current, current->length, spec_buffer_size);
#endif
            current = current->next;
            continue;
        }
        if (current->address_count != pending_count)
        {
#if defined mdebug
            fprintf(stderr, "  skip spec at %p: address_count mismatch (%zu != %zu)\n", (void *)current, current->address_count, pending_count);
#endif
            current = current->next;
            continue;
        }
        if (memcmp(current->bytes, spec_buffer, spec_buffer_size) != 0)
        {
#if defined mdebug
            fprintf(stderr, "  skip spec at %p: bytes mismatch\n", (void *)current);
#endif
            current = current->next;
            continue;
        }
        bool addresses_match = true;
        for (size_t i = 0; i < pending_count; i++)
        {
            if (current->addresses[i].offset != pending_addresses[i].offset)
            {
                addresses_match = false;
                break;
            }
            if (current->addresses[i].label != pending_addresses[i].label)
            {
                addresses_match = false;
                break;
            }
        }
        if (addresses_match)
        {
#if defined mdebug
            fprintf(stderr, "  FOUND existing label=%p\n", (void *)current->label);
#endif
            return current->label;
        }
#if defined mdebug
        fprintf(stderr, "  skip spec at %p: addresses mismatch\n", (void *)current);
#endif
        current = current->next;
    }
    T_UNIQUE_SPECIFIER *new_spec = malloc(sizeof(T_UNIQUE_SPECIFIER));
    if (new_spec == NULL)
        error_no_memory();
#if defined mdebug
    fprintf(stderr, "malloc(specifier_pool_find_or_create): new_spec=%p\n", (void *)new_spec);
#endif
    new_spec->bytes = malloc(spec_buffer_size);
    if (new_spec->bytes == NULL)
        error_no_memory();
#if defined mdebug
    fprintf(stderr, "malloc(specifier_pool_find_or_create): new_spec->bytes=%p length=%zu\n", (void *)new_spec->bytes, spec_buffer_size);
#endif
    memcpy(new_spec->bytes, spec_buffer, spec_buffer_size);
    new_spec->length = spec_buffer_size;
    new_spec->label = (T_LABEL *)generate_info_label();
    new_spec->address_count = pending_count;
    if (pending_count > 0)
    {
        new_spec->addresses = malloc(pending_count * sizeof(T_PENDING_ADDRESS));
        if (new_spec->addresses == NULL)
            error_no_memory();
#if defined mdebug
        fprintf(stderr, "malloc(specifier_pool_find_or_create): new_spec->addresses=%p count=%zu\n", (void *)new_spec->addresses, pending_count);
#endif
        memcpy(new_spec->addresses, pending_addresses, pending_count * sizeof(T_PENDING_ADDRESS));
    }
    else
        new_spec->addresses = NULL;
    new_spec->next = pool_head;
    pool_head = new_spec;
#if defined mdebug
    fprintf(stderr, "  CREATED new label=%p\n", (void *)new_spec->label);
#endif
    return new_spec->label;
}

static bool specifiers_equal(const T_UNIQUE_SPECIFIER *a, const T_UNIQUE_SPECIFIER *b)
{
    if (a->length != b->length)
        return false;
    size_t pos = 0;
    size_t addr_idx_a = 0;
    size_t addr_idx_b = 0;
    while (pos < a->length)
    {
        bool a_has_addr = (addr_idx_a < a->address_count && a->addresses[addr_idx_a].offset == pos);
        bool b_has_addr = (addr_idx_b < b->address_count && b->addresses[addr_idx_b].offset == pos);
        if (a_has_addr && b_has_addr)
        {
            if (a->addresses[addr_idx_a].label != b->addresses[addr_idx_b].label)
                return false;
            pos += LBLL;
            addr_idx_a++;
            addr_idx_b++;
        }
        else if (a_has_addr != b_has_addr)
            return false;
        else
        {
            if (a->bytes[pos] != b->bytes[pos])
                return false;
            pos++;
        }
    }
    return true;
}

void specifier_pool_finalize(void)
{
#if defined mdebug
    fprintf(stderr, "specifier_pool_finalize: begin\n");
#endif
    // === PASS 1 ===
    T_UNIQUE_SPECIFIER *current = pool_head;
    while (current != NULL)
    {
        for (size_t i = 0; i < current->address_count; i++)
        {
            T_LABEL *resolved = resolve_label_alias(current->addresses[i].label);
            current->addresses[i].label = resolved;
        }
        current = current->next;
    }
    // === PASS 2 ===
    current = pool_head;
    while (current != NULL)
    {
        T_UNIQUE_SPECIFIER **prev_next = &current->next;
        T_UNIQUE_SPECIFIER *other = current->next;
        while (other != NULL)
        {
            if (specifiers_equal(current, other))
            {
#if defined mdebug
                fprintf(stderr, "  MERGE: label=%p -> label=%p\n", (void *)other->label, (void *)current->label);
#endif
                macrocode_equ((T_LABEL *)other->label, (T_LABEL *)current->label);
                *prev_next = other->next;
                free(other->bytes);
                if (other->addresses != NULL)
                    free(other->addresses);
                free(other);
                other = *prev_next;
            }
            else
            {
                prev_next = &other->next;
                other = other->next;
            }
        }
        current = current->next;
    }
    // === PASS 3 ===
    current = pool_head;
    while (current != NULL)
    {
        macrocode_label(current->label);
        size_t pos = 0;
        size_t addr_index = 0;
        while (pos < current->length)
        {
            bool is_address = false;
            if (addr_index < current->address_count && current->addresses[addr_index].offset == pos)
            {
                macrocode_address(current->addresses[addr_index].label);
                pos += LBLL;
                addr_index++;
                is_address = true;
            }
            if (!is_address)
            {
                macrocode_byte(current->bytes[pos]);
                pos++;
            }
        }
#if defined mdebug
        fprintf(stderr, "  WRITE: label=%p length=%zu\n", (void *)current->label, current->length);
#endif
        current = current->next;
    }
#if defined mdebug
    fprintf(stderr, "specifier_pool_finalize: end\n");
#endif
}

//----------  end of file specifier_pool.c  ----------
