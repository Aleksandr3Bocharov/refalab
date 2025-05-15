// Copyright 2025 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2025-05-14
// https://github.com/Aleksandr3Bocharov/RefalAB

//-----------  file  --  XFIO.C ------------
//           MO: file input/output
//           MO: file remove/rename/exist
//------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include "refalab.h"
#include "rfintf.h"

#define fmax 5

extern uint8_t refalab_true, refalab_false;
extern uint8_t refalab_null;

static FILE *f;
static FILE *uniput[fmax] = {NULL, NULL, NULL, NULL, NULL};
static FILE *uniget[fmax] = {NULL, NULL, NULL, NULL, NULL};

char feof_0[] = {Z4 'F', 'E', 'O', 'F', (char)4};
G_L_B uint8_t refalab_feof = '\002';

char ferror_0[] = {Z6 'F', 'E', 'R', 'R', 'O', 'R', (char)6};
G_L_B uint8_t refalab_ferror = '\002';

char stdin_0[] = {Z5 'S', 'T', 'D', 'I', 'N', (char)5};
G_L_B uint8_t refalab_stdin = '\002';

char stdout_0[] = {Z6 'S', 'T', 'D', 'O', 'U', 'T', (char)6};
G_L_B uint8_t refalab_stdout = '\002';

char stderr_0[] = {Z6 'S', 'T', 'D', 'E', 'R', 'R', (char)6};
G_L_B uint8_t refalab_stderr = '\002';

static void fopen_(void)
{
    do
    {
        T_LINKCB *p = refal.preva->next;
        if (p->tag != TAGO)
            break;
        const char c = p->info.infoc;
        p = p->next;
        uint32_t j;
        if (p->tag != TAGN)
            break;
        j = gcoden(p);
        p = p->next;
        if (j >= fmax)
            break;
        char s[2];
        s[1] = '\0';
        if (c == 'R' || c == 'r')
            s[0] = 'r';
        else if (c == 'W' || c == 'w')
            s[0] = 'w';
        else if (c == 'A' || c == 'a')
            s[0] = 'a';
        else
            break;
        bool neot = false;
        T_LINKCB *q = p;
        size_t i;
        for (i = 0; p != refal.nexta; i++)
        {
            if (p->tag != TAGO)
            {
                neot = true;
                break;
            }
            p = p->next;
        }
        if (neot)
            break;
        char *namf = (char *)malloc(i + 1);
        if (namf == NULL)
            rfabe("fopen: error");
        p = q;
        for (size_t j = 0; j < i; j++)
        {
            *(namf + j) = p->info.infoc;
            p = p->next;
        }
        *(namf + i) = '\0';
        f = fopen(namf, s);
        const int err = errno;
        free(namf);
        if (f == NULL)
        {
            char *serr = strerror(err);
            p = refal.prevr;
            if (!slins(p, strlen(serr)))
                return;
            for (i = 0; *(serr + i) != '\0'; i++)
            {
                p = p->next;
                p->tag = TAGO;
                p->info.codep = NULL;
                p->info.infoc = *(serr + i);
            }
            return;
        }
        if (s[0] == 'r')
            uniget[j] = f;
        else
            uniput[j] = f;
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char fopen_0[] = {Z5 'F', 'O', 'P', 'E', 'N', (char)5};
G_L_B uint8_t refalab_fopen = '\122';
void (*fopen_1)(void) = fopen_;

static void fclose_(void)
{
    do
    {
        T_LINKCB *p = refal.preva->next;
        if (p->tag != TAGO)
            break;
        const char c = p->info.infoc;
        p = p->next;
        if (p->tag != TAGN)
            break;
        const uint32_t j = gcoden(p);
        if (j >= fmax)
            break;
        if (p->next != refal.nexta)
            break;
        if (c == 'R' || c == 'r')
        {
            f = uniget[j];
            uniget[j] = NULL;
        }
        else if (c == 'W' || c == 'w')
        {
            f = uniput[j];
            uniput[j] = NULL;
        }
        else
            break;
        if (f == NULL)
        {
            p = refal.prevr;
            if (!slins(p, 1))
                return;
            p = p->next;
            p->tag = TAGF;
            p->info.codef = &refalab_null;
            return;
        }
        const int cl = fclose(f);
        const int err = errno;
        if (cl == EOF)
        {
            char *serr = strerror(err);
            p = refal.prevr;
            if (!slins(p, strlen(serr)))
                return;
            for (size_t i = 0; *(serr + i) != '\0'; i++)
            {
                p = p->next;
                p->tag = TAGO;
                p->info.codep = NULL;
                p->info.infoc = *(serr + i);
            }
        }
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char fclose_0[] = {Z6 'F', 'C', 'L', 'O', 'S', 'E', (char)6};
G_L_B uint8_t refalab_fclose = '\122';
void (*fclose_1)(void) = fclose_;

static void fgets_(void)
{
    do
    {
        T_LINKCB *p = refal.preva->next;
        if (p->tag != TAGN)
            break;
        const uint32_t j = gcoden(p);
        if (j >= fmax)
            break;
        if (p->next != refal.nexta)
            break;
        f = uniget[j];
        p = refal.prevr;
        if (f == NULL)
        {
            if (!slins(p, 1))
                return;
            p = p->next;
            p->tag = TAGF;
            p->info.codef = &refalab_null;
            return;
        }
        int c = getc(f);
        while (c != '\n')
        {
            if (!slins(p, 1))
                return;
            p = p->next;
            p->info.codep = NULL;
            if (c == EOF)
            {
                p->tag = TAGN;
                if (feof(f) != 0)
                {
                    if (!slins(p, 1))
                        return;
                    p = p->next;
                    p->tag = TAGF;
                    p->info.codef = &refalab_feof;
                }
                if (ferror(f) != 0)
                {
                    if (!slins(p, 1))
                        return;
                    p = p->next;
                    p->tag = TAGF;
                    p->info.codef = &refalab_ferror;
                }
                return;
            }
            p->tag = TAGO;
            p->info.infoc = (char)c;
            c = getc(f);
        }
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char fgets_0[] = {Z5 'F', 'G', 'E', 'T', 'S', (char)5};
G_L_B uint8_t refalab_fgets = '\122';
void (*fgets_1)(void) = fgets_;

static void fputs_(void)
{
    do
    {
        T_LINKCB *p = refal.preva->next;
        if (p->tag != TAGN)
            break;
        const uint32_t j = gcoden(p);
        p = p->next;
        if (j >= fmax)
            break;
        f = uniput[j];
        const T_LINKCB *q = p;
        bool neot = false;
        while (q != refal.nexta)
        {
            if (q->tag != TAGO && q->tag != TAGLB && q->tag != TAGRB)
            {
                neot = true;
                break;
            }
            q = q->next;
        }
        if (neot)
            break;
        if (f == NULL)
        {
            p = refal.prevr;
            if (!slins(p, 1))
                return;
            p = p->next;
            p->tag = TAGF;
            p->info.codef = &refalab_null;
            return;
        }
        while (p != refal.nexta)
        {
            int cc;
            if (p->tag == TAGLB)
                cc = '(';
            else if (p->tag == TAGRB)
                cc = ')';
            else
                cc = p->info.infoc;
            if (putc(cc, f) == EOF)
            {
                p = refal.prevr;
                if (feof(f) != 0)
                {
                    if (!slins(p, 1))
                        return;
                    p = p->next;
                    p->tag = TAGF;
                    p->info.codef = &refalab_feof;
                }
                if (ferror(f) != 0)
                {
                    if (!slins(p, 1))
                        return;
                    p = p->next;
                    p->tag = TAGF;
                    p->info.codef = &refalab_ferror;
                }
                return;
            }
            p = p->next;
        }
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char fputs_0[] = {Z5 'F', 'P', 'U', 'T', 'S', (char)5};
G_L_B uint8_t refalab_fputs = '\122';
void (*fputs_1)(void) = fputs_;

static void remove_file_(void)
{
    T_LINKCB *p = refal.preva->next;
    size_t i;
    for (i = 0; p != refal.nexta; i++)
    {
        if (p->tag != TAGO)
        {
            refal.upshot = 2;
            return;
        }
        p = p->next;
    }
    char *namf = (char *)malloc(i + 1);
    if (namf == NULL)
        rfabe("remove_file: error");
    p = refal.preva->next;
    for (size_t j = 0; j < i; j++)
    {
        *(namf + j) = p->info.infoc;
        p = p->next;
    }
    *(namf + i) = '\0';
    const int u = unlink(namf);
    const int err = errno;
    free(namf);
    if (u == -1)
    {
        char *serr = strerror(err);
        p = refal.prevr;
        if (!slins(p, strlen(serr)))
            return;
        for (i = 0; *(serr + i) != '\0'; i++)
        {
            p = p->next;
            p->tag = TAGO;
            p->info.codep = NULL;
            p->info.infoc = *(serr + i);
        }
    }
    return;
}
char remove_file_0[] = {Z3 'R', 'E', 'M', 'O', 'V', 'E', '_', 'F', 'I', 'L', 'E', (char)11};
G_L_B uint8_t refalab_remove_file = '\122';
void (*remove_file_1)(void) = remove_file_;

static void rename_(void)
{
    T_LINKCB *p = refal.preva->next;
    size_t i;
    for (i = 0; p->tag != TAGO || p->info.infoc != '*'; i++)
    {
        if (p->tag != TAGO)
        {
            refal.upshot = 2;
            return;
        }
        p = p->next;
    }
    p = p->next;
    size_t j;
    for (j = 0; p != refal.nexta; j++)
    {
        if (p->tag != TAGO)
        {
            refal.upshot = 2;
            return;
        }
        p = p->next;
    }
    char *namf = (char *)malloc(i + 1);
    if (namf == NULL)
        rfabe("rename: error");
    p = refal.preva->next;
    size_t k;
    for (k = 0; k < i; k++)
    {
        *(namf + k) = p->info.infoc;
        p = p->next;
    }
    *(namf + i) = '\0';
    char *namt = (char *)malloc(j + 1); // from -> to
    if (namt == NULL)
        rfabe("rename: error");
    p = p->next;
    for (k = 0; k < j; k++)
    {
        *(namt + k) = p->info.infoc;
        p = p->next;
    }
    *(namt + j) = '\0';
    const int r = rename(namf, namt);
    const int err = errno;
    free(namf);
    free(namt);
    if (r == -1)
    {
        char *serr = strerror(err);
        p = refal.prevr;
        if (!slins(p, strlen(serr)))
            return;
        for (i = 0; *(serr + i) != '\0'; i++)
        {
            p = p->next;
            p->tag = TAGO;
            p->info.codep = NULL;
            p->info.infoc = *(serr + i);
        }
    }
    return;
}
char rename_0[] = {Z6 'R', 'E', 'N', 'A', 'M', 'E', (char)6};
G_L_B uint8_t refalab_rename = '\122';
void (*rename_1)(void) = rename_;

static void exist_file_(void)
{
    T_LINKCB *p = refal.preva->next;
    size_t i;
    for (i = 0; p != refal.nexta; i++)
    {
        if (p->tag != TAGO)
        {
            refal.upshot = 2;
            return;
        }
        p = p->next;
    }
    char *namf = (char *)malloc(i + 1);
    if (namf == NULL)
        rfabe("exist_file: error");
    p = refal.preva->next;
    for (size_t j = 0; j < i; j++)
    {
        *(namf + j) = p->info.infoc;
        p = p->next;
    }
    *(namf + i) = '\0';
    p = refal.prevr;
    if (!slins(p, 1))
        return;
    p = p->next;
    p->tag = TAGF;
    p->info.codef = &refalab_false;
    struct stat st_buf;
    if (stat(namf, &st_buf) == 0)
        if (S_ISREG(st_buf.st_mode))
            p->info.codef = &refalab_true;
    free(namf);
    return;
}
char exist_file_0[] = {Z2 'E', 'X', 'I', 'S', 'T', '_', 'F', 'I', 'L', 'E', (char)10};
G_L_B uint8_t refalab_exist_file = '\122';
void (*exist_file_1)(void) = exist_file_;

//----------  end of file XFIO.C  -----------
