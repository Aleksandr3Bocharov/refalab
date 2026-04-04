// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-04-03
// https://github.com/Aleksandr3Bocharov/refalab

//-----------------  file  --  cj.C  -------------------
//             generator of assembler text
//------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include "refalab.h"
#include "cj.h"
#include "cerr.h"
#include "refal.h"
#include "clu.h"

typedef struct entry
{ // entry table element
    struct entry *next;
    T_U *node;
    char identifier_extern[MAX_IDENTIFIER_EXTERN_LENGTH];
    uint8_t identifier_extern_length;
} T_ENTRY;

typedef struct extrn
{ // external pointer table element
    struct extrn *next;
    T_U *node;
    char identifier_extern[MAX_IDENTIFIER_EXTERN_LENGTH];
    uint8_t identifier_extern_length;
} T_EXTRN;

typedef struct relay
{
    T_U *node;
    uint16_t delta;
} T_RELAY;

typedef struct stream_bytes_nodes
{
    size_t length;
    size_t current;
    char *file_name;
    uint8_t *buffer;
    FILE *file;
} T_STREAM_BYTES_NODES;

static T_STREAM_BYTES_NODES stream_bytes = {0, 0, NULL, NULL, NULL};
static T_STREAM_BYTES_NODES stream_nodes = {0, 0, NULL, NULL, NULL};

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
static uint16_t delta;

static void error_no_memory(void)
{
    fputs("\nERROR: no memory!!!\n", stdout);
    exit(1);
    return;
}

static void stream_bytes_nodes_open_write(const char *file_name, T_STREAM_BYTES_NODES *stream_bytes_nodes)
{
    if (stream_bytes_nodes->file_name != NULL)
    {
        free(stream_bytes_nodes->file_name);
#if defined mdebug
        fprintf(stderr, "free(cj) stream_bytes_nodes->file_name=%p\n", (void *)stream_bytes_nodes->file_name);
#endif
    }
    stream_bytes_nodes->file_name = (char *)malloc(strlen(file_name) + 1);
    if (stream_bytes_nodes->file_name == NULL)
        error_no_memory();
#if defined mdebug
    fprintf(stderr, "malloc(cj): stream_bytes_nodes->file_name=%p\n", (void *)stream_bytes_nodes->file_name);
#endif
    strcpy(stream_bytes_nodes->file_name, file_name);
    size_t stream_bytes_nodes_length = 0;
    if (stream_bytes_nodes->buffer == NULL)
    {
        size_t max_stream_bytes_nodes_length;
        if (stream_bytes_nodes == &stream_nodes)
        {
            if (LBLL == 4)
                max_stream_bytes_nodes_length = 49152; // 8192*6
            else
                max_stream_bytes_nodes_length = 81920; // 8192*10
        }
        else
        {
            if (LBLL == 4)
                max_stream_bytes_nodes_length = 65528; // 65536-8
            else
                max_stream_bytes_nodes_length = 98292; // 98304-12
        }
        stream_bytes_nodes_length = max_stream_bytes_nodes_length;
        size_t min_stream_bytes_nodes_length;
        if (LBLL == 4)
            min_stream_bytes_nodes_length = 16;
        else
            min_stream_bytes_nodes_length = 24;
        while (true)
        {
            stream_bytes_nodes->buffer = (uint8_t *)malloc(stream_bytes_nodes_length);
            if (stream_bytes_nodes->buffer != NULL)
            {
#if defined mdebug
                fprintf(stderr, "malloc(cj): stream_bytes_nodes->buffer=%p stream_bytes_nodes_length=%zu\n", (void *)stream_bytes_nodes->buffer, stream_bytes_nodes_length);
#endif
                break;
            }
            else
            {
                if (stream_bytes_nodes == &stream_nodes)
                    stream_bytes_nodes_length /= 2;
                else
                    stream_bytes_nodes_length = (stream_bytes_nodes_length + LBLL + 4) / 2 - LBLL - 4;
                if (stream_bytes_nodes_length < min_stream_bytes_nodes_length)
                    error_no_memory();
            }
        } // while
    }
    stream_bytes_nodes->current = 0;
    stream_bytes_nodes->length = stream_bytes_nodes_length;
    stream_bytes_nodes->file = NULL;
    return;
}

static void stream_bytes_nodes_open_read(T_STREAM_BYTES_NODES *stream_bytes_nodes)
{
    if (stream_bytes_nodes->file != NULL)
    {
        stream_bytes_nodes->file = fopen(stream_bytes_nodes->file_name, Rbin);
        if (stream_bytes_nodes->file == NULL)
        {
            printf("Can't open for read %s\n", stream_bytes_nodes->file_name);
            exit(8);
        }
        if (fread(stream_bytes_nodes->buffer, stream_bytes_nodes->length, 1, stream_bytes_nodes->file) == 0)
        {
            printf("Read i/o error in %s\n", stream_bytes_nodes->file_name);
            exit(8);
        }
    }
    stream_bytes_nodes->current = 0;
    return;
}

static void stream_bytes_nodes_close(const T_STREAM_BYTES_NODES *stream_bytes_nodes)
{
    if (stream_bytes_nodes->file != NULL)
    {
        if (fwrite(stream_bytes_nodes->buffer, stream_bytes_nodes->current, 1, stream_bytes_nodes->file) == 0)
        {
            printf("Write i/o error in %s\n", stream_bytes_nodes->file_name);
            exit(8);
        }
        fclose(stream_bytes_nodes->file);
    }
    return;
}

static void stream_bytes_nodes_clear(T_STREAM_BYTES_NODES *stream_bytes_nodes)
{
    if (stream_bytes_nodes->file != NULL)
        unlink(stream_bytes_nodes->file_name);
    free(stream_bytes_nodes->file_name);
    free(stream_bytes_nodes->buffer);
#if defined mdebug
    fprintf(stderr, "free(cj)    stream_bytes_nodes->file_name=%p\n", (void *)stream_bytes_nodes->file_name);
    fprintf(stderr, "            stream_bytes_nodes->buffer=%p\n", (void *)stream_bytes_nodes->buffer);
#endif
    stream_bytes_nodes->file_name = NULL;
    stream_bytes_nodes->buffer = NULL;
    return;
}

static void stream_nodes_write(void)
{
    while (true)
    {
        const size_t residual = stream_nodes.length - stream_nodes.current;
        if (residual >= SMBL)
        {
            memcpy(stream_nodes.buffer + stream_nodes.current, &relay, SMBL);
            stream_nodes.current += SMBL;
            return;
        }
        if (stream_nodes.file == NULL)
        {
            stream_nodes.file = fopen(stream_nodes.file_name, Wbin);
            if (stream_nodes.file == NULL)
            {
                printf("Can't open for write sysut2\n");
                exit(8);
            }
        }
        if (fwrite(stream_nodes.buffer, stream_nodes.length, 1, stream_nodes.file) == 0)
        {
            printf("Write i/o error in sysut2\n");
            exit(8);
        }
        stream_nodes.current = 0;
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
        if (residual >= SMBL)
        {
            memcpy(&relay, stream_nodes.buffer + stream_nodes.current, SMBL);
            stream_nodes.current += SMBL;
            return;
        }
        stream_nodes.current = 0;
    } // while
} // stream_nodes_read

void jit_start(void)
{
    delta = 0;
    stream_bytes_nodes_open_write("sysut1.rf", &stream_bytes);
    stream_bytes_nodes_open_write("sysut2.rf", &stream_nodes);
    first_entry = (T_ENTRY *)malloc(sizeof(T_ENTRY));
    if (first_entry == NULL)
        error_no_memory();
#if defined mdebug
    fprintf(stderr, "malloc(cj): first_entry=%p\n", (void *)first_entry);
#endif
    last_entry = first_entry;
    first_entry->next = NULL;
    first_extrn = (T_EXTRN *)malloc(sizeof(T_EXTRN));
    if (first_extrn == NULL)
        error_no_memory();
#if defined mdebug
    fprintf(stderr, "malloc(cj): first_extrn=%p\n", (void *)first_extrn);
#endif
    last_extrn = first_extrn;
    first_extrn->next = NULL;
    current_address = 0;
    extrn_count = 1;
    return;
} // jit_start

size_t jit_where(void)
{
    size_t max_address;
    if (LBLL == 4)
        max_address = 65535;
    else
        max_address = 98303;
    if (current_address > max_address)
    {
        printf("Module too long\n");
        exit(1);
    }
    return current_address;
}

void jit_byte(uint8_t byte)
{
    if (stream_bytes.current != stream_bytes.length)
    {
        *(stream_bytes.buffer + stream_bytes.current) = byte;
        stream_bytes.current++;
    }
    else
    {
        if (stream_bytes.file == NULL)
        {
            stream_bytes.file = fopen(stream_bytes.file_name, Wbin);
            if (stream_bytes.file == NULL)
            {
                printf("Can't open for write sysut1\n");
                exit(8);
            }
        }
        if (fwrite(stream_bytes.buffer, stream_bytes.length, 1, stream_bytes.file) == 0)
        {
            printf("Write i/o error in sysut1\n");
            exit(8);
        }
        *stream_bytes.buffer = byte;
        stream_bytes.current = 1;
    }
    delta++;
    current_address++;
    return;
} // jit_byte

void jit_address(T_U *node)
{
    relay.node = node;
    relay.delta = delta;
    delta = 0;
    stream_nodes_write();
    current_address += LBLL;
    return;
}

void jit_entry(T_U *node, const char *idendifier_extern, uint8_t idendifier_extern_length)
// idendifier_extern label
{
    // idendifier_extern_length label length
    entry2 = first_entry;
    while (entry2 != last_entry)
    {
        entry2 = entry2->next;
        if (entry2->identifier_extern_length == idendifier_extern_length && strncmp(entry2->identifier_extern, idendifier_extern, idendifier_extern_length) == 0)
        {
            print_error_three_strings("604 external label", idendifier_extern, idendifier_extern_length, " is already defined");
            return;
        }
    }
    entry2 = (T_ENTRY *)malloc(sizeof(T_ENTRY));
    if (entry2 == NULL)
        error_no_memory();
#if defined mdebug
    fprintf(stderr, "malloc(cj): entry2=%p\n", (void *)entry2);
#endif
    last_entry->next = entry2;
    last_entry = entry2;
    entry2->node = node;
    entry2->next = NULL;
    entry2->identifier_extern_length = idendifier_extern_length;
    strncpy(entry2->identifier_extern, idendifier_extern, entry2->identifier_extern_length);
    node->mode |= '\040';
    return;
} // jit_entry

void jit_extrn(T_U *node, const char *idendifier_extern, uint8_t idendifier_extern_length)
// idendifier_extern label
{
    // idendifier_extern_length label length
    extrn2 = first_extrn;
    while (extrn2 != last_extrn)
    {
        extrn2 = extrn2->next;
        if (extrn2->identifier_extern_length == idendifier_extern_length && strncmp(extrn2->identifier_extern, idendifier_extern, idendifier_extern_length) == 0)
        {
            node->info.infon = extrn2->node->info.infon;
            node->mode |= '\220';
            return;
        }
    }
    extrn2 = (T_EXTRN *)malloc(sizeof(T_EXTRN));
    if (extrn2 == NULL)
        error_no_memory();
#if defined mdebug
    fprintf(stderr, "malloc(cj): extrn2=%p\n", (void *)extrn2);
#endif
    last_extrn->next = extrn2;
    last_extrn = extrn2;
    extrn2->node = node;
    extrn2->next = NULL;
    extrn2->identifier_extern_length = idendifier_extern_length;
    strncpy(extrn2->identifier_extern, idendifier_extern, extrn2->identifier_extern_length);
    node->mode |= '\220';
    extrn_count++;
    node->info.infon = extrn_count;
    return;
} // jit_extrn

void jit_label(T_U *node)
{
    node->mode |= '\120';
    node->info.infon = current_address;
    return;
}

void jit_equ(T_U *equ_node, T_U *node)
{
    equ_node->info.infop = node;
    equ_node->mode |= '\320';
    return;
}

static void ending(void)
{
    relay.delta = delta;
    relay.node = NULL;
    stream_nodes_write();
    stream_bytes_nodes_close(&stream_bytes);
    stream_bytes_nodes_close(&stream_nodes);
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

void jit_end(void)
{
    ending();
    uint8_t byte = 0;
    // heading generating
    write_assembler_source(fputs(".data\n", assembler_source));
    char bufs[81];
    sprintf(bufs, "_d%d$:\n", scanner.module_number);
    write_assembler_source(fputs(bufs, assembler_source));
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
            for (uint16_t k = 0; k < delta; k++)
            {
                stream_bytes_read(&byte, 1);
                if (k % 60 == 0)
                {
                    if (k != 0)
                        write_assembler_source(fputc('\n', assembler_source));
                    write_assembler_source(fputs("\t.byte\t", assembler_source));
                }
                sprintf(bufs, "%d", byte);
                write_assembler_source(fputs(bufs, assembler_source));
                if (k % 60 != 59 && k != delta - 1)
                    write_assembler_source(fputc(',', assembler_source));
            }
            write_assembler_source(fputc('\n', assembler_source));
            const T_U *p = relay.node;
            if (p != NULL)
            {
                while ((p->mode & '\300') == '\300')
                    p = p->info.infop;
                if ((p->mode & '\300') != '\200')
                {
                    //    nonexternal label
                    if (LBLL == 4)
                        sprintf(bufs, "\t.long\t_d%d$+%zu\n", scanner.module_number, p->info.infon);
                    else
                        sprintf(bufs, "\t.quad\t_d%d$+%zu\n", scanner.module_number, p->info.infon);
                    write_assembler_source(fputs(bufs, assembler_source));
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
                    for (size_t i = 1; i < p->info.infon; i++)
                        extrn = extrn->next;
                    for (size_t i = 0; i < extrn->identifier_extern_length; i++)
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
            for (size_t i = 0; i < extrn->identifier_extern_length; i++)
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
            for (size_t i = 0; i < entry->identifier_extern_length; i++)
                // translate name to lower case
                write_assembler_source(fputc(tolower(*(entry->identifier_extern + i)), assembler_source));
            const T_U *pp = entry->node;
            while ((pp->mode & '\300') == '\300')
                pp = pp->info.infop;
#if defined POSIX
            // begin name without underlining _
            sprintf(bufs, "\t=_d%d$+%zu\n\t.globl\trefalab_", scanner.module_number, pp->info.infon);
#else // Windows
            if (LBLL == 4)
                sprintf(bufs, "\t=_d%d$+%zu\n\t.globl\t_refalab_", scanner.module_number, pp->info.infon);
            else
                sprintf(bufs, "\t=_d%d$+%zu\n\t.globl\trefalab_", scanner.module_number, pp->info.infon);
#endif
            write_assembler_source(fputs(bufs, assembler_source));
            for (size_t i = 0; i < entry->identifier_extern_length; i++)
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
        fprintf(stderr, "free(cj) entry=%p\n", (void *)entry);
#endif
        free(entry);
        entry = entry2;
    }
    extrn = first_extrn;
    while (extrn != NULL)
    {
        extrn2 = extrn->next;
#if defined mdebug
        fprintf(stderr, "free(cj) extrn=%p\n", (void *)extrn);
#endif
        free(extrn);
        extrn = extrn2;
    }
    return;
} // jit_end

//----------  end of file CJ.C  -------------------
