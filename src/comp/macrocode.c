// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-06-07
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file macrocode.c  ----------
//             generator of macrocode text
//----------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <unistd.h>
#include "refalab.h"
#include "macrocode.h"
#include "print_errors.h"
#include "compiler.h"
#include "avl_identifiers.h"

#define SZRLY sizeof(size_t) + sizeof(void *)

#define PRINT_ERROR_604 \
    print_error_three_strings("604 external label", idendifier_extern, idendifier_extern_length, " is already defined")

typedef struct entry
{ // entry table element
    struct entry *next;
    T_LABEL *label;
    char identifier_extern[MAX_IDENTIFIER_EXTERN_LENGTH];
    uint8_t identifier_extern_length;
} T_ENTRY;

typedef struct extrn
{ // external pointer table element
    struct extrn *next;
    T_LABEL *label;
    char identifier_extern[MAX_IDENTIFIER_EXTERN_LENGTH];
    uint8_t identifier_extern_length;
} T_EXTRN;

typedef struct relay
{
    T_LABEL *label;
    size_t delta;
} T_RELAY;

typedef struct stream_bytes_nodes
{
    size_t length;
    size_t current;
    uint8_t *buffer;
} T_STREAM_BYTES_NODES;

static T_STREAM_BYTES_NODES stream_bytes = {0, 0, NULL};
static T_STREAM_BYTES_NODES stream_nodes = {0, 0, NULL};

static T_ENTRY *entry, *entry2;
static T_EXTRN *extrn, *extrn2;

static T_ENTRY *first_entry;
static T_ENTRY *last_entry;
static T_EXTRN *first_extrn;
static T_EXTRN *last_extrn;
static size_t extrn_count;
static size_t current_address; // module generation files
static size_t module_length;
static T_RELAY relay;
static size_t delta;

static void error_no_memory(void)
{
    fputs("\nERROR: no memory!!!\n", stdout);
    exit(1);
    return;
}

static void stream_bytes_nodes_clear(T_STREAM_BYTES_NODES *stream_bytes_nodes)
{
    free(stream_bytes_nodes->buffer);
#if defined mdebug
    fprintf(stderr, "free(stream_bytes_nodes_clear): stream_bytes_nodes->buffer=%p\n", (void *)stream_bytes_nodes->buffer);
#endif
    stream_bytes_nodes->buffer = NULL;
    stream_bytes_nodes->length = 0;
    stream_bytes_nodes->current = 0;
    return;
}

static void stream_bytes_nodes_open_write(T_STREAM_BYTES_NODES *stream_bytes_nodes)
{
    stream_bytes_nodes_clear(stream_bytes_nodes);
    const size_t max_stream_bytes_nodes_length = 8192 * SZRLY;
    const size_t min_stream_bytes_nodes_length = 2 * SZRLY;
    size_t stream_bytes_nodes_length = max_stream_bytes_nodes_length;
    while (true)
    {
        stream_bytes_nodes->buffer = (uint8_t *)malloc(stream_bytes_nodes_length);
        if (stream_bytes_nodes->buffer != NULL)
        {
#if defined mdebug
            fprintf(stderr, "malloc(stream_bytes_nodes_open_write): stream_bytes_nodes->buffer=%p stream_bytes_nodes_length=%zu\n", (void *)stream_bytes_nodes->buffer, stream_bytes_nodes_length);
#endif
            break;
        }
        else
        {
            stream_bytes_nodes_length /= 2;
            if (stream_bytes_nodes_length < min_stream_bytes_nodes_length)
                error_no_memory();
        }
    } // while
    stream_bytes_nodes->length = stream_bytes_nodes_length;
    stream_bytes_nodes->current = 0;
    return;
}

static void stream_bytes_nodes_append(T_STREAM_BYTES_NODES *stream_bytes_nodes)
{
    if (stream_bytes_nodes->buffer == NULL)
        return;
    return;
}

static void stream_bytes_nodes_open_read(T_STREAM_BYTES_NODES *stream_bytes_nodes)
{
    stream_bytes_nodes->current = 0;
    return;
}

static void stream_nodes_write(void)
{
    while (true)
    {
        const size_t residual = stream_nodes.length - stream_nodes.current;
        if (residual >= SZRLY)
        {
            memcpy(stream_nodes.buffer + stream_nodes.current, &relay, SZRLY);
            stream_nodes.current += SZRLY;
            return;
        }
        stream_bytes_nodes_append(&stream_nodes);
    } // while
} // stream_nodes_write

static void stream_bytes_read(uint8_t *buffer, size_t count)
{
    while (true)
    {
        const size_t residual = stream_bytes.length - stream_bytes.current;
        if (residual >= count)
        {
            memcpy(buffer, stream_bytes.buffer + stream_bytes.current, count);
            stream_bytes.current += count;
            return;
        }
        memcpy(buffer, stream_bytes.buffer + stream_bytes.current, residual);
        stream_bytes.current = 0;
        count -= residual;
        buffer += residual;
    } // while
} // stream_bytes_read

static void stream_nodes_read(void)
{
    while (true)
    {
        const size_t residual = stream_nodes.length - stream_nodes.current;
        if (residual >= SZRLY)
        {
            memcpy(&relay, stream_nodes.buffer + stream_nodes.current, SZRLY);
            stream_nodes.current += SZRLY;
            return;
        }
        stream_nodes.current = 0;
    } // while
} // stream_nodes_read

void macrocode_start(void)
{
    delta = 0;
    stream_bytes_nodes_open_write(&stream_bytes);
    stream_bytes_nodes_open_write(&stream_nodes);
    first_entry = (T_ENTRY *)malloc(sizeof(T_ENTRY));
    if (first_entry == NULL)
        error_no_memory();
#if defined mdebug
    fprintf(stderr, "malloc(macrocode_start): first_entry=%p\n", (void *)first_entry);
#endif
    last_entry = first_entry;
    first_entry->next = NULL;
    first_extrn = (T_EXTRN *)malloc(sizeof(T_EXTRN));
    if (first_extrn == NULL)
        error_no_memory();
#if defined mdebug
    fprintf(stderr, "malloc(macrocode_start): first_extrn=%p\n", (void *)first_extrn);
#endif
    last_extrn = first_extrn;
    first_extrn->next = NULL;
    current_address = 0;
    extrn_count = 1;
    return;
} // macrocode_start

size_t macrocode_where(void)
{
    return current_address;
}

void macrocode_byte(uint8_t byte)
{
    if (stream_bytes.current != stream_bytes.length)
    {
        *(stream_bytes.buffer + stream_bytes.current) = byte;
        stream_bytes.current++;
    }
    else
    {
        stream_bytes_nodes_append(&stream_bytes);
        *(stream_bytes.buffer + stream_bytes.current) = byte;
        stream_bytes.current++;
    }
    delta++;
    current_address++;
    return;
} // macrocode_byte

void macrocode_address(T_LABEL *label)
{
    relay.label = label;
    relay.delta = delta;
    delta = 0;
    stream_nodes_write();
    current_address += LBLL;
    return;
}

void macrocode_entry(T_LABEL *label, const char *idendifier_extern, uint8_t idendifier_extern_length)
// idendifier_extern label
{
    // idendifier_extern_length label length
    extrn2 = first_extrn;
    while (extrn2 != last_extrn)
    {
        extrn2 = extrn2->next;
        if (extrn2->identifier_extern_length == idendifier_extern_length && strncmp(extrn2->identifier_extern, idendifier_extern, idendifier_extern_length) == 0)
        {
            PRINT_ERROR_604;
            return;
        }
    }
    entry2 = first_entry;
    while (entry2 != last_entry)
    {
        entry2 = entry2->next;
        if (entry2->identifier_extern_length == idendifier_extern_length && strncmp(entry2->identifier_extern, idendifier_extern, idendifier_extern_length) == 0)
        {
            PRINT_ERROR_604;
            return;
        }
    }
    entry2 = (T_ENTRY *)malloc(sizeof(T_ENTRY));
    if (entry2 == NULL)
        error_no_memory();
#if defined mdebug
    fprintf(stderr, "malloc(macrocode_entry): entry2=%p\n", (void *)entry2);
#endif
    last_entry->next = entry2;
    last_entry = entry2;
    entry2->label = label;
    entry2->next = NULL;
    entry2->identifier_extern_length = idendifier_extern_length;
    strncpy(entry2->identifier_extern, idendifier_extern, entry2->identifier_extern_length);
    label->mode |= 0040;
    return;
} // macrocode_entry

void macrocode_extrn(T_LABEL *label, const char *idendifier_extern, uint8_t idendifier_extern_length)
// idendifier_extern label
{
    // idendifier_extern_length label length
    entry2 = first_entry;
    while (entry2 != last_entry)
    {
        entry2 = entry2->next;
        if (entry2->identifier_extern_length == idendifier_extern_length && strncmp(entry2->identifier_extern, idendifier_extern, idendifier_extern_length) == 0)
        {
            PRINT_ERROR_604;
            return;
        }
    }
    extrn2 = first_extrn;
    while (extrn2 != last_extrn)
    {
        extrn2 = extrn2->next;
        if (extrn2->identifier_extern_length == idendifier_extern_length && strncmp(extrn2->identifier_extern, idendifier_extern, idendifier_extern_length) == 0)
        {
            label->info.infon = extrn2->label->info.infon;
            label->mode |= 0220;
            return;
        }
    }
    extrn2 = (T_EXTRN *)malloc(sizeof(T_EXTRN));
    if (extrn2 == NULL)
        error_no_memory();
#if defined mdebug
    fprintf(stderr, "malloc(macrocode_extrn): extrn2=%p\n", (void *)extrn2);
#endif
    last_extrn->next = extrn2;
    last_extrn = extrn2;
    extrn2->label = label;
    extrn2->next = NULL;
    extrn2->identifier_extern_length = idendifier_extern_length;
    strncpy(extrn2->identifier_extern, idendifier_extern, extrn2->identifier_extern_length);
    label->mode |= 0220;
    extrn_count++;
    label->info.infon = extrn_count;
    return;
} // macrocode_extrn

void macrocode_label(T_LABEL *label)
{
    label->mode |= 0120;
    label->info.infon = current_address;
    return;
}

void macrocode_equ(T_LABEL *equ_label, T_LABEL *label)
{
    equ_label->info.infop = label;
    equ_label->mode |= 0320;
    return;
}

static void ending(void)
{
    relay.delta = delta;
    relay.label = NULL;
    stream_nodes_write();
    module_length = current_address;
    return;
} // ending

static void write_assembler_source(int put_result)
{
    if (put_result == EOF)
        if (feof(assembler_source) != 0 || ferror(assembler_source) != 0)
        {
            printf("Write i/o error in assembler source file\n");
            exit(8);
        }
}

void macrocode_end(void)
{
    ending();
    uint8_t byte = 0;
    // heading generating
    write_assembler_source(fputs(".data\n", assembler_source));
    char buffer_string[81];
    sprintf(buffer_string, "_d%" PRIu32 "$:\n", scanner.module_number);
    write_assembler_source(fputs(buffer_string, assembler_source));
    //  empty module test
    if (module_length != 0)
    {
        // text generating
        stream_bytes_nodes_open_read(&stream_bytes);
        stream_bytes_nodes_open_read(&stream_nodes);
        while (true)
        {
            stream_nodes_read();
            delta = relay.delta;
            for (size_t k = 0; k < delta; k++)
            {
                stream_bytes_read(&byte, 1);
                if (k % 60 == 0)
                {
                    if (k != 0)
                        write_assembler_source(fputc('\n', assembler_source));
                    write_assembler_source(fputs("\t.byte\t", assembler_source));
                }
                sprintf(buffer_string, "%" PRIu8, byte);
                write_assembler_source(fputs(buffer_string, assembler_source));
                if (k % 60 != 59 && k != delta - 1)
                    write_assembler_source(fputc(',', assembler_source));
            }
            write_assembler_source(fputc('\n', assembler_source));
            const T_LABEL *label = relay.label;
            if (label != NULL)
            {
                while ((label->mode & 0300) == 0300)
                    label = label->info.infop;
                if ((label->mode & 0300) != 0200)
                {
                    //    nonexternal label
                    if (LBLL == 4)
                        sprintf(buffer_string, "\t.long\t_d%" PRIu32 "$+%zu\n", scanner.module_number, label->info.infon);
                    else
                        sprintf(buffer_string, "\t.quad\t_d%" PRIu32 "$+%zu\n", scanner.module_number, label->info.infon);
                    write_assembler_source(fputs(buffer_string, assembler_source));
                }
                else
                {
//     external   label
#if defined POSIX
                    // begin name without underlining _
                    if (LBLL == 4)
                        write_assembler_source(fputs("\t.long\trefalab_", assembler_source));
                    else
                        write_assembler_source(fputs("\t.quad\trefalab_", assembler_source));
#else // Windows - with underlining _ in x86
                    if (LBLL == 4)
                        write_assembler_source(fputs("\t.long\t_refalab_", assembler_source));
                    else
                        write_assembler_source(fputs("\t.quad\trefalab_", assembler_source));
#endif
                    extrn = first_extrn;
                    for (size_t i = 1; i < label->info.infon; i++)
                        extrn = extrn->next;
                    for (uint8_t i = 0; i < extrn->identifier_extern_length; i++)
                        write_assembler_source(fputc(tolower(*(extrn->identifier_extern + i)), assembler_source));
                    write_assembler_source(fputs("\n", assembler_source));
                }
                continue;
            } // if
            break;
        }
        //   external label generating
        extrn = first_extrn->next;
        while (extrn != NULL)
        {
//
#if defined POSIX
            // begin name without underlining _
            write_assembler_source(fputs("\t.extern\trefalab_", assembler_source));
#else // Windows
            if (LBLL == 4)
                write_assembler_source(fputs("\t.extern\t_refalab_", assembler_source));
            else
                write_assembler_source(fputs("\t.extern\trefalab_", assembler_source));
#endif
            for (uint8_t i = 0; i < extrn->identifier_extern_length; i++)
                write_assembler_source(fputc(tolower(*(extrn->identifier_extern + i)), assembler_source));
            write_assembler_source(fputs(":byte\n", assembler_source));
            extrn = extrn->next;
        } // while
        write_assembler_source(fputs(".data\n", assembler_source));
        // entry label generating
        entry = first_entry->next;
        while (entry != NULL)
        {
#if !defined POSIX
            // begin name with underlining _ in x86
            if (LBLL == 4)
                write_assembler_source(fputc('_', assembler_source));
#endif
            write_assembler_source(fputs("refalab_", assembler_source));
            for (uint8_t i = 0; i < entry->identifier_extern_length; i++)
                // translate name to lower case
                write_assembler_source(fputc(tolower(*(entry->identifier_extern + i)), assembler_source));
            const T_LABEL *label = entry->label;
            while ((label->mode & 0300) == 0300)
                label = label->info.infop;
#if defined POSIX
            // begin name without underlining _
            sprintf(buffer_string, "\t=_d%" PRIu32 "$+%zu\n\t.globl\trefalab_", scanner.module_number, label->info.infon);
#else // Windows
            if (LBLL == 4)
                sprintf(buffer_string, "\t=_d%" PRIu32 "$+%zu\n\t.globl\t_refalab_", scanner.module_number, label->info.infon);
            else
                sprintf(buffer_string, "\t=_d%" PRIu32 "$+%zu\n\t.globl\trefalab_", scanner.module_number, label->info.infon);
#endif
            write_assembler_source(fputs(buffer_string, assembler_source));
            for (uint8_t i = 0; i < entry->identifier_extern_length; i++)
                // translate name to lower case
                write_assembler_source(fputc(tolower(*(entry->identifier_extern + i)), assembler_source));
            write_assembler_source(fputc('\n', assembler_source));
            entry = entry->next;
        };
#if defined POSIX
        write_assembler_source(fputs(".section\t.note.GNU-stack,\"\",\%progbits\n", assembler_source));
#endif
    }
    // termination
    stream_bytes_nodes_clear(&stream_bytes);
    stream_bytes_nodes_clear(&stream_nodes);
    entry = first_entry;
    while (entry != NULL)
    {
        entry2 = entry->next;
#if defined mdebug
        fprintf(stderr, "free(macrocode_end) entry=%p\n", (void *)entry);
#endif
        free(entry);
        entry = entry2;
    }
    extrn = first_extrn;
    while (extrn != NULL)
    {
        extrn2 = extrn->next;
#if defined mdebug
        fprintf(stderr, "free(macrocode_end) extrn=%p\n", (void *)extrn);
#endif
        free(extrn);
        extrn = extrn2;
    }
    return;
} // macrocode_end

//----------  end of file macrocode.c  ----------
