// Copyright 2024 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2024-10-27
// https://github.com/Aleksandr3Bocharov/RefalAB

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
#include "refal.def"
#include "cj.h"
#include "cerr.h"
#include "refal.h"
#include "clu.h"

typedef struct ent
{ // entry table element
    struct ent *next;
    T_U *p;
    char e[8];
    size_t le;
} T_ENT;

typedef struct ext
{ // external pointer table element
    struct ext *next;
    T_U *p;
    char e[8];
    size_t le;
} T_EXT;

typedef struct rl
{
    T_U *point;
    uint16_t delta;
} T_RL;

typedef struct BU_
{
    size_t len;
    size_t tek;
    char *nam;
    char *buf;
    FILE *fil;
} BU;

static BU sysut1 = {0, 0, NULL, NULL, NULL};
static BU sysut2 = {0, 0, NULL, NULL, NULL};

static union
{
    char b[2];
    uint16_t w;
} d;

static T_ENT *q, *r;
static T_EXT *qx, *rx;

/*    MODE field value:
   00 - undefined;
   01 - internal,   INFO = shift against begin;
   10 - external,   INFO = external pointer number;
   11 - equivalent, INFO = another label pointer;
   XX1 - entry;
   XXX1 - still defined.
      TYPE field value:
   00 - unknown type;
   01 - function;
   10 - specifier.
*/

static T_ENT *first_ent;
static T_ENT *last_ent;
static size_t mod_length;
static T_EXT *first_ext;
static T_EXT *last_ext;
static size_t curr_addr; // module generation files
static size_t n_ext;
static T_RL rl;
static size_t k;
static uint16_t delta;

static void oshex(void)
{
    fputs("\nOSHEX: no memory!!!\n", stdout);
    exit(1);
    return;
}

static void sfop_w(const char *s, BU *b)
{
    if (b->nam != NULL)
    {
        free(b->nam);
#ifdef mdebug
        printf("free(cj) b->nam=%p\n", (void *)b->nam);
#endif
    }
    b->nam = (char *)malloc(strlen(s) + 1);
    if (b->nam == NULL)
        oshex();
#ifdef mdebug
    printf("malloc(cj): b->nam=%p\n", (void *)b->nam);
#endif
    strcpy(b->nam, s);
    size_t un = 0;
    if (b->buf == NULL)
    {
        if (options.mincomp)
        {
            if (b == &sysut2)
                un = 2040; // 2040=340*6
            else
                un = 2040; // 2040=2048-8
        }
        else
        {
            if (b == &sysut2)
                un = 49152; // 8192*6
            else
                un = 65528; // 65536-8 (for bc mojno - 4)
        }
        size_t lon;
        while (true)
        {
            b->buf = (char *)malloc(un);
            if (b->buf != NULL)
            {
#ifdef mdebug
                printf("malloc(cj): b->buf=%p un=%zu\n", (void *)b->buf, un);
#endif
                break;
            }
            else
            {
                lon = un;
                if (b == &sysut2)
                    lon /= 2;
                else
                    lon = (lon + 8) / 2 - 8;
                un = lon;
                if (un < 16)
                    oshex();
            }
        } // while
    }
    b->tek = 0;
    b->len = un;
    b->fil = NULL;
    return;
}

static void sfop_r(BU *b)
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

static void sfcl(const BU *b)
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

static void sfclr(BU *b)
{
    if (b->fil != NULL)
        unlink(b->nam);
    free(b->nam);
    free(b->buf);
#ifdef mdebug
    printf("free(sfclr) b->nam(c 0)=%p\n", (void *)b->nam);
    printf("            b->buf(c 0)=%p\n", (void *)b->buf);
#endif
    b->nam = NULL;
    b->buf = NULL;
    return;
}

static void sfwr2(void)
{
    while (true)
    {
        const size_t ost = sysut2.len - sysut2.tek;
        if (ost >= SMBL)
        {
            memcpy(sysut2.buf + sysut2.tek, &rl, SMBL);
            sysut2.tek += SMBL;
            return;
        }
        if (sysut2.fil == NULL)
        {
            sysut2.fil = fopen(sysut2.nam, Wbin);
            if (sysut2.fil == NULL)
            {
                printf("Can't open for write sysut2\n");
                exit(8);
            }
        }
        if (fwrite(sysut2.buf, sysut2.len, 1, sysut2.fil) == 0)
        {
            printf("Write i/o error in sysut2\n");
            exit(8);
        }
        sysut2.tek = 0;
    } // while
} // sfwr2

static void sfrd1(char *c, size_t n)
{
    while (true)
    {
        const size_t ost = sysut1.len - sysut1.tek;
        if (ost >= n)
        {
            memcpy(c, sysut1.buf + sysut1.tek, n);
            sysut1.tek += n;
            return;
        }
        memcpy(c, sysut1.buf + sysut1.tek, ost);
        sysut1.tek = 0;
        n -= ost;
        c += ost;
    } // while
} // sfrd1

static void sfrd2(void)
{
    while (true)
    {
        const size_t ost = sysut2.len - sysut2.tek;
        if (ost >= SMBL)
        {
            memcpy(&rl, sysut2.buf + sysut2.tek, SMBL);
            sysut2.tek += SMBL;
            return;
        }
        sysut2.tek = 0;
    } // while
} // sfrd2

void jstart(void)
{
    delta = 0;
    sfop_w("sysut1.rf", &sysut1);
    sfop_w("sysut2.rf", &sysut2);
    first_ent = (T_ENT *)malloc(sizeof(T_ENT));
    if (first_ent == NULL)
        oshex();
#ifdef mdebug
    printf("malloc(cj): first_ent=%p\n", (void *)first_ent);
#endif
    last_ent = first_ent;
    first_ent->next = NULL;
    first_ext = (T_EXT *)malloc(sizeof(T_EXT));
    if (first_ext == NULL)
        oshex();
#ifdef mdebug
    printf("malloc(cj): first_ext=%p\n", (void *)first_ext);
#endif
    last_ext = first_ext;
    first_ext->next = NULL;
    curr_addr = 0;
    n_ext = 1;
    return;
} // jstart

size_t jwhere(void)
{
    if (curr_addr > 65535)
    {
        printf("Module too long\n");
        exit(1);
    }
    return curr_addr;
}

void jbyte(char bb)
{
    if (sysut1.tek != sysut1.len)
    {
        *(sysut1.buf + sysut1.tek) = bb;
        sysut1.tek++;
    }
    else
    {
        if (sysut1.fil == NULL)
        {
            sysut1.fil = fopen(sysut1.nam, Wbin);
            if (sysut1.fil == NULL)
            {
                printf("Can't open for write sysut1\n");
                exit(8);
            }
        }
        if (fwrite(sysut1.buf, sysut1.len, 1, sysut1.fil) == 0)
        {
            printf("Write i/o error in sysut1\n");
            exit(8);
        }
        *sysut1.buf = bb;
        sysut1.tek = 1;
    }
    delta++;
    curr_addr++;
    return;
} // jbyte

void j3addr(T_U *pp)
{
    rl.point = pp;
    rl.delta = delta;
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
        if (r->le == ll && strncmp(r->e, ee, ll < r->le ? ll : r->le) == 0)
            return;
    }
    r = (T_ENT *)malloc(sizeof(T_ENT));
    if (r == NULL)
        oshex();
#ifdef mdebug
    printf("malloc(cj): r(ent)=%p\n", (void *)r);
#endif
    last_ent->next = r;
    last_ent = r;
    r->p = pp;
    r->next = NULL;
    r->le = 8 < ll ? 8 : ll;
    strncpy(r->e, ee, r->le);
    pp->mode |= '\040';
    return;
} // jentry

void jextrn(T_U *pp, const char *ee, size_t ll)
// ee label
{
    //  label length
    rx = (T_EXT *)malloc(sizeof(T_EXT));
    if (rx == NULL)
        oshex();
#ifdef mdebug
    printf("malloc(cj): rx(ext)=%p\n", (void *)rx);
#endif
    last_ext->next = rx;
    last_ext = rx;
    rx->p = pp;
    rx->next = NULL;
    rx->le = 8 < ll ? 8 : ll;
    strncpy(rx->e, ee, rx->le);
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
    rl.delta = delta;
    rl.point = NULL;
    sfwr2();
    sfcl(&sysut1);
    sfcl(&sysut2);
    mod_length = curr_addr;
    return;
} // zakon

void jend(void)
{
    zakon();
    if (options.multmod)
    {
        strcat(mod_i, ".asm");
        syslin = fopen(mod_i, "w");
        if (syslin == NULL)
        {
            printf("Can't open %s\n", mod_i);
            exit(8);
        }
    }
    d.w = 0;
    // heading generating
    fputs(".data\n", syslin);
    char bufs[81];
    sprintf(bufs, "_d%d$:\n", nommod);
    fputs(bufs, syslin);
    //  empty module test
    if (mod_length != 0)
    {
        // text generating
        sfop_r(&sysut1);
        sfop_r(&sysut2);
        while (true)
        {
            sfrd2();
            delta = rl.delta;
            for (k = 0; k < delta; k++)
            {
                sfrd1(&d.b[0], 1);
                if (k % 60 == 0)
                {
                    if (k != 0)
                        fputc('\n', syslin);
                    fputs("\t.byte\t", syslin);
                }
                sprintf(bufs, "%d", d.w);
                fputs(bufs, syslin);
                if (k % 60 != 59 && k != (size_t)(delta - 1))
                    fputc(',', syslin);
            }
            fputc('\n', syslin);
            const T_U *p = rl.point;
            if (p != NULL)
            {
                while ((p->mode & '\300') == '\300')
                    p = p->info.infop;
                if ((p->mode & '\300') != '\200')
                {
                    //    nonexternal label
                    if (LBLL == 4)
                        sprintf(bufs, "\t.long\t_d%d$+%zu\n", nommod, p->info.infon);
                    else
                        sprintf(bufs, "\t.quad\t_d%d$+%zu\n", nommod, p->info.infon);
                    fputs(bufs, syslin);
                }
                else
                {
//     external   label
#ifdef UNIX
                    // begin name without underlining _
                    if (LBLL == 4)
                        fputs("\t.long\t", syslin);
                    else
                        fputs("\t.quad\t", syslin);
#else // Windows - with underlining _ in x86
                    if (LBLL == 4)
                        fputs("\t.long\t_", syslin);
                    else
                        fputs("\t.quad\t", syslin);
#endif
                    qx = first_ext;
                    for (size_t i = 1; i < p->info.infon; i++)
                        qx = qx->next;
                    for (size_t i = 0; i < qx->le; i++)
                        fputc(tolower(*(qx->e + i)), syslin);
                    fputs("\n", syslin);
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
#ifdef UNIX
            // begin name without underlining _
            fputs("\t.extern\t", syslin);
#else // Windows
            if (LBLL == 4)
                fputs("\t.extern\t_", syslin);
            else
                fputs("\t.extern\t", syslin);
#endif
            for (size_t i = 0; i < qx->le; i++)
                fputc(tolower(*(qx->e + i)), syslin);
            fputs(":byte\n", syslin);
            qx = qx->next;
        } // while
        fputs(".data\n", syslin);
        // entry label generating
        q = first_ent->next;
        while (q != NULL)
        {
#ifndef UNIX
            // begin name with underlining _ in x86
            if (LBLL == 4)
                fputc('_', syslin);
#endif
            for (size_t i = 0; i < q->le; i++)
                // translate name to lower case
                fputc(tolower(*(q->e + i)), syslin);
            const T_U *pp = q->p;
            while ((pp->mode & '\300') == '\300')
                pp = pp->info.infop;
#ifdef UNIX
            // begin name without underlining _
            sprintf(bufs, "\t=_d%d$+%zu\n\t.globl\t", nommod, pp->info.infon);
#else // Windows
            if (LBLL == 4)
                sprintf(bufs, "\t=_d%d$+%zu\n\t.globl\t_", nommod, pp->info.infon);
            else
                sprintf(bufs, "\t=_d%d$+%zu\n\t.globl\t", nommod, pp->info.infon);
#endif
            fputs(bufs, syslin);
            for (size_t i = 0; i < q->le; i++)
                // translate name to lower case
                fputc(tolower(*(q->e + i)), syslin);
            fputc('\n', syslin);
            q = q->next;
        };
    }
    // termination
    sfclr(&sysut1);
    sfclr(&sysut2);
    if (options.multmod)
    {
        fclose(syslin);
        if (mod_length != 0)
        {
            fputc(' ', systxt);
            fputs(mod_i, systxt);
        }
        else
        {
            nommod--;
            unlink(mod_i);
        }
    }
    q = first_ent;
    while (q != NULL)
    {
        r = q->next;
#ifdef mdebug
        printf("free(cj) q=%p\n", (void *)q);
#endif
        free(q);
        q = r;
    }
    qx = first_ext;
    while (qx != NULL)
    {
        rx = qx->next;
#ifdef mdebug
        printf("free(cj) qx=%p\n", (void *)qx);
#endif
        free(qx);
        qx = rx;
    }
    return;
} // jend

//----------  end of file CJ.C  -------------------
