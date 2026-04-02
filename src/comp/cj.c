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

typedef struct file_bytes_nodes
{
    size_t length;
    size_t current;
    char *name;
    uint8_t *buffer;
    FILE *file;
} T_FILE_BYTES_NODES;

static T_FILE_BYTES_NODES file_bytes = {0, 0, NULL, NULL, NULL};
static T_FILE_BYTES_NODES file_nodes = {0, 0, NULL, NULL, NULL};

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

static void oshex(void)
{
    fputs("\nOSHEX: no memory!!!\n", stdout);
    exit(1);
    return;
}

static void sfop_w(const char *s, T_FILE_BYTES_NODES *b)
{
    if (b->name != NULL)
    {
        free(b->name);
#if defined mdebug
        fprintf(stderr, "free(cj) b->name=%p\n", (void *)b->name);
#endif
    }
    b->name = (char *)malloc(strlen(s) + 1);
    if (b->name == NULL)
        oshex();
#if defined mdebug
    fprintf(stderr, "malloc(cj): b->name=%p\n", (void *)b->name);
#endif
    strcpy(b->name, s);
    size_t un = 0;
    if (b->buffer == NULL)
    {
        if (b == &file_nodes)
        {
            if (LBLL == 4)
                un = 49152; // 8192*6
            else
                un = 81920; // 8192*10
        }
        else
        {
            if (LBLL == 4)
                un = 65528; // 65536-8
            else
                un = 98292; // 98304-12
        }
        size_t lon;
        size_t min_oshex;
        if (LBLL == 4)
            min_oshex = 16;
        else
            min_oshex = 24;
        while (true)
        {
            b->buffer = (uint8_t *)malloc(un);
            if (b->buffer != NULL)
            {
#if defined mdebug
                fprintf(stderr, "malloc(cj): b->buffer=%p un=%zu\n", (void *)b->buffer, un);
#endif
                break;
            }
            else
            {
                lon = un;
                if (b == &file_nodes)
                    lon /= 2;
                else
                    lon = (lon + LBLL + 4) / 2 - LBLL - 4;
                un = lon;
                if (un < min_oshex)
                    oshex();
            }
        } // while
    }
    b->current = 0;
    b->length = un;
    b->file = NULL;
    return;
}

static void sfop_r(T_FILE_BYTES_NODES *b)
{
    if (b->file != NULL)
    {
        b->file = fopen(b->name, Rbin);
        if (b->file == NULL)
        {
            printf("Can't open for read %s\n", b->name);
            exit(8);
        }
        if (fread(b->buffer, b->length, 1, b->file) == 0)
        {
            printf("Read i/o error in %s\n", b->name);
            exit(8);
        }
    }
    b->current = 0;
    return;
}

static void sfcl(const T_FILE_BYTES_NODES *b)
{
    if (b->file != NULL)
    {
        if (fwrite(b->buffer, b->current, 1, b->file) == 0)
        {
            printf("Write i/o error in %s\n", b->name);
            exit(8);
        }
        fclose(b->file);
    }
    return;
}

static void sfclr(T_FILE_BYTES_NODES *b)
{
    if (b->file != NULL)
        unlink(b->name);
    free(b->name);
    free(b->buffer);
#if defined mdebug
    fprintf(stderr, "free(sfclr) b->name(c 0)=%p\n", (void *)b->name);
    fprintf(stderr, "            b->buffer(c 0)=%p\n", (void *)b->buffer);
#endif
    b->name = NULL;
    b->buffer = NULL;
    return;
}

static void sfwr2(void)
{
    while (true)
    {
        const size_t ost = file_nodes.length - file_nodes.current;
        if (ost >= SMBL)
        {
            memcpy(file_nodes.buffer + file_nodes.current, &relay, SMBL);
            file_nodes.current += SMBL;
            return;
        }
        if (file_nodes.file == NULL)
        {
            file_nodes.file = fopen(file_nodes.name, Wbin);
            if (file_nodes.file == NULL)
            {
                printf("Can't open for write sysut2\n");
                exit(8);
            }
        }
        if (fwrite(file_nodes.buffer, file_nodes.length, 1, file_nodes.file) == 0)
        {
            printf("Write i/o error in sysut2\n");
            exit(8);
        }
        file_nodes.current = 0;
    } // while
} // sfwr2

static void sfrd1(uint8_t *c, size_t n)
{
    while (true)
    {
        const size_t ost = file_bytes.length - file_bytes.current;
        if (ost >= n)
        {
            memcpy(c, file_bytes.buffer + file_bytes.current, n);
            file_bytes.current += n;
            return;
        }
        memcpy(c, file_bytes.buffer + file_bytes.current, ost);
        file_bytes.current = 0;
        n -= ost;
        c += ost;
    } // while
} // sfrd1

static void sfrd2(void)
{
    while (true)
    {
        const size_t ost = file_nodes.length - file_nodes.current;
        if (ost >= SMBL)
        {
            memcpy(&relay, file_nodes.buffer + file_nodes.current, SMBL);
            file_nodes.current += SMBL;
            return;
        }
        file_nodes.current = 0;
    } // while
} // sfrd2

void jstart(void)
{
    delta = 0;
    sfop_w("sysut1.rf", &file_bytes);
    sfop_w("sysut2.rf", &file_nodes);
    first_entry = (T_ENTRY *)malloc(sizeof(T_ENTRY));
    if (first_entry == NULL)
        oshex();
#if defined mdebug
    fprintf(stderr, "malloc(cj): first_entry=%p\n", (void *)first_entry);
#endif
    last_entry = first_entry;
    first_entry->next = NULL;
    first_extrn = (T_EXTRN *)malloc(sizeof(T_EXTRN));
    if (first_extrn == NULL)
        oshex();
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
    if (file_bytes.current != file_bytes.length)
    {
        *(file_bytes.buffer + file_bytes.current) = bb;
        file_bytes.current++;
    }
    else
    {
        if (file_bytes.file == NULL)
        {
            file_bytes.file = fopen(file_bytes.name, Wbin);
            if (file_bytes.file == NULL)
            {
                printf("Can't open for write sysut1\n");
                exit(8);
            }
        }
        if (fwrite(file_bytes.buffer, file_bytes.length, 1, file_bytes.file) == 0)
        {
            printf("Write i/o error in sysut1\n");
            exit(8);
        }
        *file_bytes.buffer = bb;
        file_bytes.current = 1;
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
        oshex();
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
        oshex();
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
    sfcl(&file_bytes);
    sfcl(&file_nodes);
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
        sfop_r(&file_bytes);
        sfop_r(&file_nodes);
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
    sfclr(&file_bytes);
    sfclr(&file_nodes);
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
