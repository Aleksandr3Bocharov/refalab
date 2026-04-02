// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-03-31
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

static uint8_t byte;

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

static void file_bytes_nodes_open_write(const char *file_name, T_STREAM_BYTES_NODES *stream_bytes_nodes)
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
    size_t file_bytes_nodes_length = 0;
    if (stream_bytes_nodes->buffer == NULL)
    {
        size_t max_file_bytes_nodes_length;
        if (stream_bytes_nodes == &stream_nodes)
        {
            if (LBLL == 4)
                max_file_bytes_nodes_length = 49152; // 8192*6
            else
                max_file_bytes_nodes_length = 81920; // 8192*10
        }
        else
        {
            if (LBLL == 4)
                max_file_bytes_nodes_length = 65528; // 65536-8
            else
                max_file_bytes_nodes_length = 98292; // 98304-12
        }
        file_bytes_nodes_length = max_file_bytes_nodes_length;
        size_t min_file_bytes_nodes_length;
        if (LBLL == 4)
            min_file_bytes_nodes_length = 16;
        else
            min_file_bytes_nodes_length= 24;
        while (true)
        {
            stream_bytes_nodes->buffer = (uint8_t *)malloc(file_bytes_nodes_length);
            if (stream_bytes_nodes->buffer != NULL)
            {
#if defined mdebug
                fprintf(stderr, "malloc(cj): stream_bytes_nodes->buffer=%p file_bytes_nodes_length=%zu\n", (void *)stream_bytes_nodes->buffer, file_bytes_nodes_length);
#endif
                break;
            }
            else
            {
                if (stream_bytes_nodes == &stream_nodes)
                    file_bytes_nodes_length /= 2;
                else
                    file_bytes_nodes_length = (file_bytes_nodes_length + LBLL + 4) / 2 - LBLL - 4;
                if (file_bytes_nodes_length < min_file_bytes_nodes_length)
                    error_no_memory();
            }
        } // while
    }
    stream_bytes_nodes->current = 0;
    stream_bytes_nodes->length = file_bytes_nodes_length;
    stream_bytes_nodes->file = NULL;
    return;
}

static void sfop_r(T_STREAM_BYTES_NODES *stream_bytes_nodes)
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

static void sfcl(const T_STREAM_BYTES_NODES *stream_bytes_nodes)
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

static void sfclr(T_STREAM_BYTES_NODES *stream_bytes_nodes)
{
    if (stream_bytes_nodes->file != NULL)
        unlink(stream_bytes_nodes->file_name);
    free(stream_bytes_nodes->file_name);
    free(stream_bytes_nodes->buffer);
#if defined mdebug
    fprintf(stderr, "free(sfclr) stream_bytes_nodes->file_name(c 0)=%p\n", (void *)stream_bytes_nodes->file_name);
    fprintf(stderr, "            stream_bytes_nodes->buffer(c 0)=%p\n", (void *)stream_bytes_nodes->buffer);
#endif
    stream_bytes_nodes->file_name = NULL;
    stream_bytes_nodes->buffer = NULL;
    return;
}

static void sfwr2(void)
{
    while (true)
    {
        const size_t ost = stream_nodes.length - stream_nodes.current;
        if (ost >= SMBL)
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
} // sfwr2

static void sfrd1(uint8_t *c, size_t n)
{
    while (true)
    {
        const size_t ost = stream_bytes.length - stream_bytes.current;
        if (ost >= n)
        {
            memcpy(c, stream_bytes.buffer + stream_bytes.current, n);
            stream_bytes.current += n;
            return;
        }
        memcpy(c, stream_bytes.buffer + stream_bytes.current, ost);
        stream_bytes.current = 0;
        n -= ost;
        c += ost;
    } // while
} // sfrd1

static void sfrd2(void)
{
    while (true)
    {
        const size_t ost = stream_nodes.length - stream_nodes.current;
        if (ost >= SMBL)
        {
            memcpy(&relay, stream_nodes.buffer + stream_nodes.current, SMBL);
            stream_nodes.current += SMBL;
            return;
        }
        stream_nodes.current = 0;
    } // while
} // sfrd2

void jstart(void)
{
    delta = 0;
    file_bytes_nodes_open_write("sysut1.rf", &stream_bytes);
    file_bytes_nodes_open_write("sysut2.rf", &stream_nodes);
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
} // jstart

size_t jwhere(void)
{
    size_t max_addr;
    if (LBLL == 4)
        max_addr = 65535;
    else
        max_addr = 98303;
    if (current_address > max_addr)
    {
        printf("Module too long\n");
        exit(1);
    }
    return current_address;
}

void jbyte(uint8_t bb)
{
    if (stream_bytes.current != stream_bytes.length)
    {
        *(stream_bytes.buffer + stream_bytes.current) = bb;
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
        *stream_bytes.buffer = bb;
        stream_bytes.current = 1;
    }
    delta++;
    current_address++;
    return;
} // jbyte

void j3addr(T_U *pp)
{
    relay.node = pp;
    relay.delta = delta;
    delta = 0;
    sfwr2();
    current_address += LBLL;
    return;
}

void jentry(T_U *pp, const char *ee, size_t ll)
// ee label
{
    entry2 = first_entry;
    while (entry2 != last_entry)
    {
        entry2 = entry2->next;
        if (entry2->identifier_extern_length == ll && strncmp(entry2->identifier_extern, ee, ll < entry2->identifier_extern_length ? ll : entry2->identifier_extern_length) == 0)
        // !!!!! this is error
            return;
    }
    entry2 = (T_ENTRY *)malloc(sizeof(T_ENTRY));
    if (entry2 == NULL)
        error_no_memory();
#if defined mdebug
    fprintf(stderr, "malloc(cj): entry2()=%p\n", (void *)entry2);
#endif
    last_entry->next = entry2;
    last_entry = entry2;
    entry2->node = pp;
    entry2->next = NULL;
    entry2->identifier_extern_length = MAX_IDENTIFIER_EXTERN_LENGTH < ll ? MAX_IDENTIFIER_EXTERN_LENGTH : ll;
    strncpy(entry2->identifier_extern, ee, entry2->identifier_extern_length);
    pp->mode |= '\040';
    return;
} // jentry

void jextrn(T_U *pp, const char *ee, size_t ll)
// ee label
{
    //  label length
    extrn2 = (T_EXTRN *)malloc(sizeof(T_EXTRN));
    if (extrn2 == NULL)
        error_no_memory();
#if defined mdebug
    fprintf(stderr, "malloc(cj): extrn2=%p\n", (void *)extrn2);
#endif
    last_extrn->next = extrn2;
    last_extrn = extrn2;
    extrn2->node = pp;
    extrn2->next = NULL;
    extrn2->identifier_extern_length = MAX_IDENTIFIER_EXTERN_LENGTH < ll ? MAX_IDENTIFIER_EXTERN_LENGTH : ll;
    strncpy(extrn2->identifier_extern, ee, extrn2->identifier_extern_length);
    pp->mode |= '\220';
    extrn_count++;
    pp->info.infon = extrn_count;
    return;
} // jextrn

void jlabel(T_U *pp)
{
    pp->mode |= '\120';
    pp->info.infon = current_address;
    return;
}

void jequ(T_U *pp, T_U *qq)
{
    pp->info.infop = qq;
    pp->mode |= '\320';
    return;
}

static void zakon(void)
{
    relay.delta = delta;
    relay.node = NULL;
    sfwr2();
    sfcl(&stream_bytes);
    sfcl(&stream_nodes);
    module_length = current_address;
    return;
} // zakon

static void write_asm(int put, FILE *f)
{
    if (put == EOF)
        if (feof(f) != 0 || ferror(f) != 0)
        {
            printf("Write i/o error in asm file\n");
            exit(8);
        }
}

void jend(void)
{
    zakon();
    byte = 0;
    // heading generating
    write_asm(fputs(".data\n", assembler_source), assembler_source);
    char bufs[81];
    sprintf(bufs, "_d%d$:\n", scanner.module_number);
    write_asm(fputs(bufs, assembler_source), assembler_source);
    //  empty module test
    if (module_length != 0)
    {
        // text generating
        sfop_r(&stream_bytes);
        sfop_r(&stream_nodes);
        while (true)
        {
            sfrd2();
            delta = relay.delta;
            for (uint16_t k = 0; k < delta; k++)
            {
                sfrd1(&byte, 1);
                if (k % 60 == 0)
                {
                    if (k != 0)
                        write_asm(fputc('\n', assembler_source), assembler_source);
                    write_asm(fputs("\t.byte\t", assembler_source), assembler_source);
                }
                sprintf(bufs, "%d", byte);
                write_asm(fputs(bufs, assembler_source), assembler_source);
                if (k % 60 != 59 && k != delta - 1)
                    write_asm(fputc(',', assembler_source), assembler_source);
            }
            write_asm(fputc('\n', assembler_source), assembler_source);
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
                    write_asm(fputs(bufs, assembler_source), assembler_source);
                }
                else
                {
//     external   label
#if defined POSIX
                    // begin name without underlining _
                    if (LBLL == 4)
                        write_asm(fputs("\t.long\trefalab_", assembler_source), assembler_source);
                    else
                        write_asm(fputs("\t.quad\trefalab_", assembler_source), assembler_source);
#else // Windows - with underlining _ in x86
                    if (LBLL == 4)
                        write_asm(fputs("\t.long\t_refalab_", assembler_source), assembler_source);
                    else
                        write_asm(fputs("\t.quad\trefalab_", assembler_source), assembler_source);
#endif
                    extrn = first_extrn;
                    for (size_t i = 1; i < p->info.infon; i++)
                        extrn = extrn->next;
                    for (size_t i = 0; i < extrn->identifier_extern_length; i++)
                        write_asm(fputc(tolower(*(extrn->identifier_extern + i)), assembler_source), assembler_source);
                    write_asm(fputs("\n", assembler_source), assembler_source);
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
            write_asm(fputs("\t.extern\trefalab_", assembler_source), assembler_source);
#else // Windows
            if (LBLL == 4)
                write_asm(fputs("\t.extern\t_refalab_", assembler_source), assembler_source);
            else
                write_asm(fputs("\t.extern\trefalab_", assembler_source), assembler_source);
#endif
            for (size_t i = 0; i < extrn->identifier_extern_length; i++)
                write_asm(fputc(tolower(*(extrn->identifier_extern + i)), assembler_source), assembler_source);
            write_asm(fputs(":byte\n", assembler_source), assembler_source);
            extrn = extrn->next;
        } // while
        write_asm(fputs(".data\n", assembler_source), assembler_source);
        // entry label generating
        entry = first_entry->next;
        while (entry != NULL)
        {
#if !defined POSIX
            // begin name with underlining _ in x86
            if (LBLL == 4)
                write_asm(fputc('_', assembler_source), assembler_source);
#endif
            write_asm(fputs("refalab_", assembler_source), assembler_source);
            for (size_t i = 0; i < entry->identifier_extern_length; i++)
                // translate name to lower case
                write_asm(fputc(tolower(*(entry->identifier_extern + i)), assembler_source), assembler_source);
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
            write_asm(fputs(bufs, assembler_source), assembler_source);
            for (size_t i = 0; i < entry->identifier_extern_length; i++)
                // translate name to lower case
                write_asm(fputc(tolower(*(entry->identifier_extern + i)), assembler_source), assembler_source);
            write_asm(fputc('\n', assembler_source), assembler_source);
            entry = entry->next;
        };
#if defined POSIX
        write_asm(fputs(".section\t.note.GNU-stack,\"\",\%progbits\n", assembler_source), assembler_source);
#endif
    }
    // termination
    sfclr(&stream_bytes);
    sfclr(&stream_nodes);
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
} // jend

//----------  end of file CJ.C  -------------------
