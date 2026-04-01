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

static T_ENTRY *q, *r;
static T_EXTRN *qx, *rx;

static T_ENTRY *first_ent;
static T_ENTRY *last_ent;
static size_t mod_length;
static T_EXTRN *first_ext;
static T_EXTRN *last_ext;
static size_t curr_addr; // module generation files
static size_t n_ext;
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
    if (b->nam != NULL)
    {
        free(b->nam);
#if defined mdebug
        fprintf(stderr, "free(cj) b->nam=%p\n", (void *)b->nam);
#endif
    }
    b->nam = (char *)malloc(strlen(s) + 1);
    if (b->nam == NULL)
        oshex();
#if defined mdebug
    fprintf(stderr, "malloc(cj): b->nam=%p\n", (void *)b->nam);
#endif
    strcpy(b->nam, s);
    size_t un = 0;
    if (b->buf == NULL)
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
            b->buf = (uint8_t *)malloc(un);
            if (b->buf != NULL)
            {
#if defined mdebug
                fprintf(stderr, "malloc(cj): b->buf=%p un=%zu\n", (void *)b->buf, un);
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
    b->tek = 0;
    b->len = un;
    b->fil = NULL;
    return;
}

static void sfop_r(T_FILE_BYTES_NODES *b)
{
    if (b->fil != NULL)
    {
        b->fil = fopen(b->nam, Rbin);
        if (b->fil == NULL)
        {
            printf("Can't open for read %s\n", b->nam);
            exit(8);
        }
        if (fread(b->buf, b->len, 1, b->fil) == 0)
        {
            printf("Read i/o error in %s\n", b->nam);
            exit(8);
        }
    }
    b->tek = 0;
    return;
}

static void sfcl(const T_FILE_BYTES_NODES *b)
{
    if (b->fil != NULL)
    {
        if (fwrite(b->buf, b->tek, 1, b->fil) == 0)
        {
            printf("Write i/o error in %s\n", b->nam);
            exit(8);
        }
        fclose(b->fil);
    }
    return;
}

static void sfclr(T_FILE_BYTES_NODES *b)
{
    if (b->fil != NULL)
        unlink(b->nam);
    free(b->nam);
    free(b->buf);
#if defined mdebug
    fprintf(stderr, "free(sfclr) b->nam(c 0)=%p\n", (void *)b->nam);
    fprintf(stderr, "            b->buf(c 0)=%p\n", (void *)b->buf);
#endif
    b->nam = NULL;
    b->buf = NULL;
    return;
}

static void sfwr2(void)
{
    while (true)
    {
        const size_t ost = file_nodes.len - file_nodes.tek;
        if (ost >= SMBL)
        {
            memcpy(file_nodes.buf + file_nodes.tek, &relay, SMBL);
            file_nodes.tek += SMBL;
            return;
        }
        if (file_nodes.fil == NULL)
        {
            file_nodes.fil = fopen(file_nodes.nam, Wbin);
            if (file_nodes.fil == NULL)
            {
                printf("Can't open for write sysut2\n");
                exit(8);
            }
        }
        if (fwrite(file_nodes.buf, file_nodes.len, 1, file_nodes.fil) == 0)
        {
            printf("Write i/o error in sysut2\n");
            exit(8);
        }
        file_nodes.tek = 0;
    } // while
} // sfwr2

static void sfrd1(uint8_t *c, size_t n)
{
    while (true)
    {
        const size_t ost = file_bytes.len - file_bytes.tek;
        if (ost >= n)
        {
            memcpy(c, file_bytes.buf + file_bytes.tek, n);
            file_bytes.tek += n;
            return;
        }
        memcpy(c, file_bytes.buf + file_bytes.tek, ost);
        file_bytes.tek = 0;
        n -= ost;
        c += ost;
    } // while
} // sfrd1

static void sfrd2(void)
{
    while (true)
    {
        const size_t ost = file_nodes.len - file_nodes.tek;
        if (ost >= SMBL)
        {
            memcpy(&relay, file_nodes.buf + file_nodes.tek, SMBL);
            file_nodes.tek += SMBL;
            return;
        }
        file_nodes.tek = 0;
    } // while
} // sfrd2

void jstart(void)
{
    delta = 0;
    sfop_w("sysut1.rf", &file_bytes);
    sfop_w("sysut2.rf", &file_nodes);
    first_ent = (T_ENTRY *)malloc(sizeof(T_ENTRY));
    if (first_ent == NULL)
        oshex();
#if defined mdebug
    fprintf(stderr, "malloc(cj): first_ent=%p\n", (void *)first_ent);
#endif
    last_ent = first_ent;
    first_ent->next = NULL;
    first_ext = (T_EXTRN *)malloc(sizeof(T_EXTRN));
    if (first_ext == NULL)
        oshex();
#if defined mdebug
    fprintf(stderr, "malloc(cj): first_ext=%p\n", (void *)first_ext);
#endif
    last_ext = first_ext;
    first_ext->next = NULL;
    curr_addr = 0;
    n_ext = 1;
    return;
} // jstart

size_t jwhere(void)
{
    size_t max_addr;
    if (LBLL == 4)
        max_addr = 65535;
    else
        max_addr = 98303;
    if (curr_addr > max_addr)
    {
        printf("Module too long\n");
        exit(1);
    }
    return curr_addr;
}

void jbyte(uint8_t bb)
{
    if (file_bytes.tek != file_bytes.len)
    {
        *(file_bytes.buf + file_bytes.tek) = bb;
        file_bytes.tek++;
    }
    else
    {
        if (file_bytes.fil == NULL)
        {
            file_bytes.fil = fopen(file_bytes.nam, Wbin);
            if (file_bytes.fil == NULL)
            {
                printf("Can't open for write sysut1\n");
                exit(8);
            }
        }
        if (fwrite(file_bytes.buf, file_bytes.len, 1, file_bytes.fil) == 0)
        {
            printf("Write i/o error in sysut1\n");
            exit(8);
        }
        *file_bytes.buf = bb;
        file_bytes.tek = 1;
    }
    delta++;
    curr_addr++;
    return;
} // jbyte

void j3addr(T_U *pp)
{
    relay.node = pp;
    relay.delta = delta;
    delta = 0;
    sfwr2();
    curr_addr += LBLL;
    return;
}

void jentry(T_U *pp, const char *ee, size_t ll)
// ee label
{
    r = first_ent;
    while (r != last_ent)
    {
        r = r->next;
        if (r->identifier_extern_length == ll && strncmp(r->identifier_extern, ee, ll < r->identifier_extern_length ? ll : r->identifier_extern_length) == 0)
        // !!!!! this is error
            return;
    }
    r = (T_ENTRY *)malloc(sizeof(T_ENTRY));
    if (r == NULL)
        oshex();
#if defined mdebug
    fprintf(stderr, "malloc(cj): r(ent)=%p\n", (void *)r);
#endif
    last_ent->next = r;
    last_ent = r;
    r->node = pp;
    r->next = NULL;
    r->identifier_extern_length = MAX_IDENTIFIER_EXTERN_LENGTH < ll ? MAX_IDENTIFIER_EXTERN_LENGTH : ll;
    strncpy(r->identifier_extern, ee, r->identifier_extern_length);
    pp->mode |= '\040';
    return;
} // jentry

void jextrn(T_U *pp, const char *ee, size_t ll)
// ee label
{
    //  label length
    rx = (T_EXTRN *)malloc(sizeof(T_EXTRN));
    if (rx == NULL)
        oshex();
#if defined mdebug
    fprintf(stderr, "malloc(cj): rx(ext)=%p\n", (void *)rx);
#endif
    last_ext->next = rx;
    last_ext = rx;
    rx->node = pp;
    rx->next = NULL;
    rx->identifier_extern_length = MAX_IDENTIFIER_EXTERN_LENGTH < ll ? MAX_IDENTIFIER_EXTERN_LENGTH : ll;
    strncpy(rx->identifier_extern, ee, rx->identifier_extern_length);
    pp->mode |= '\220';
    n_ext++;
    pp->info.infon = n_ext;
    return;
} // jextrn

void jlabel(T_U *pp)
{
    pp->mode |= '\120';
    pp->info.infon = curr_addr;
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
    mod_length = curr_addr;
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
    if (mod_length != 0)
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
                    qx = first_ext;
                    for (size_t i = 1; i < p->info.infon; i++)
                        qx = qx->next;
                    for (size_t i = 0; i < qx->identifier_extern_length; i++)
                        write_asm(fputc(tolower(*(qx->identifier_extern + i)), assembler_source), assembler_source);
                    write_asm(fputs("\n", assembler_source), assembler_source);
                }
                continue;
            } // if
            break;
        }
        //   external label generating
        qx = first_ext->next;
        while (qx != NULL)
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
            for (size_t i = 0; i < qx->identifier_extern_length; i++)
                write_asm(fputc(tolower(*(qx->identifier_extern + i)), assembler_source), assembler_source);
            write_asm(fputs(":byte\n", assembler_source), assembler_source);
            qx = qx->next;
        } // while
        write_asm(fputs(".data\n", assembler_source), assembler_source);
        // entry label generating
        q = first_ent->next;
        while (q != NULL)
        {
#if !defined POSIX
            // begin name with underlining _ in x86
            if (LBLL == 4)
                write_asm(fputc('_', assembler_source), assembler_source);
#endif
            write_asm(fputs("refalab_", assembler_source), assembler_source);
            for (size_t i = 0; i < q->identifier_extern_length; i++)
                // translate name to lower case
                write_asm(fputc(tolower(*(q->identifier_extern + i)), assembler_source), assembler_source);
            const T_U *pp = q->node;
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
            for (size_t i = 0; i < q->identifier_extern_length; i++)
                // translate name to lower case
                write_asm(fputc(tolower(*(q->identifier_extern + i)), assembler_source), assembler_source);
            write_asm(fputc('\n', assembler_source), assembler_source);
            q = q->next;
        };
#if defined POSIX
        write_asm(fputs(".section\t.note.GNU-stack,\"\",\%progbits\n", assembler_source), assembler_source);
#endif
    }
    // termination
    sfclr(&file_bytes);
    sfclr(&file_nodes);
    q = first_ent;
    while (q != NULL)
    {
        r = q->next;
#if defined mdebug
        fprintf(stderr, "free(cj) q=%p\n", (void *)q);
#endif
        free(q);
        q = r;
    }
    qx = first_ext;
    while (qx != NULL)
    {
        rx = qx->next;
#if defined mdebug
        fprintf(stderr, "free(cj) qx=%p\n", (void *)qx);
#endif
        free(qx);
        qx = rx;
    }
    return;
} // jend

//----------  end of file CJ.C  -------------------
