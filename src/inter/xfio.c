// Copyright 2025 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2025-05-10
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
#include "refalab.h"
#include "rfintf.h"

#define MAX_FILE_NAME 254
#define fmax 5

extern uint8_t refalab_true, refalab_false;

static FILE *f;
static FILE *uniput[fmax] = {NULL, NULL, NULL, NULL, NULL};
static FILE *uniget[fmax] = {NULL, NULL, NULL, NULL, NULL};

static void fopen_(void)
{
    char namf[MAX_FILE_NAME + 1];
    const T_LINKCB *p = refal.preva->next;
    bool neot1 = false;
    do
    {
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
        size_t i;
        for (i = 0; p != refal.nexta; i++)
        {
            if (p->tag != TAGO || i == MAX_FILE_NAME)
            {
                neot = true;
                break;
            }
            namf[i] = p->info.infoc;
            p = p->next;
        }
        if (neot)
            break;
        namf[i] = '\0';
        f = fopen(namf, s);
        if (f == NULL)
        {
            printf("fopen: can't open file %s\n", namf);
            neot1 = true;
            break;
        }
        if (s[0] == 'r')
            uniget[j] = f;
        else
            uniput[j] = f;
        return;
    } while (false);
    if (!neot1)
        printf("fopen: format error\n");
    refal.upshot = 2;
    return;
}
char fopen_0[] = {Z5 'F', 'O', 'P', 'E', 'N', (char)5};
G_L_B uint8_t refalab_fopen = '\122';
void (*fopen_1)(void) = fopen_;

static void fclose_(void)
{
    const T_LINKCB *p = refal.preva->next;
    do
    {
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
        fclose(f);
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
    T_LINKCB *p = refal.preva->next;
    bool neot1 = false;
    do
    {
        if (p->tag != TAGN)
            break;
        const uint32_t j = gcoden(p);
        if (j >= fmax)
            break;
        if (p->next != refal.nexta)
            break;
        f = uniget[j];
        if (f == NULL)
        {
            char namf[11];
            strcpy(namf, "REFAL0.DAT");
            namf[5] = (char)j + '0';
            f = fopen(namf, "r");
            if (f == NULL)
            {
                printf("fgets: can't open file %s\n", namf);
                neot1 = true;
                break;
            }
            uniget[j] = f;
        }
        p = refal.prevr;
        int c = getc(f);
        while (c != '\n')
        {
            if (!slins(p, 1))
                return;
            p = p->next;
            p->info.codep = NULL;
            if (c == EOF || c == '\032')
            {
                p->tag = TAGN;
                return;
            }
            p->tag = TAGO;
            p->info.infoc = (char)c;
            c = getc(f);
        }
        return;
    } while (false);
    if (!neot1)
        printf("fgets: format error\n");
    refal.upshot = 2;
    return;
}
char fgets_0[] = {Z5 'F', 'G', 'E', 'T', 'S', (char)5};
G_L_B uint8_t refalab_fgets = '\122';
void (*fgets_1)(void) = fgets_;

static void fputs_(void)
{
    const T_LINKCB *p = refal.preva->next;
    bool neot1 = false;
    do
    {
        if (p->tag != TAGN)
            break;
        const uint32_t j = gcoden(p);
        p = p->next;
        if (j >= fmax)
            break;
        f = uniput[j];
        if (f == NULL)
        {
            char namf[11];
            strcpy(namf, "REFAL0.DAT");
            namf[5] = (char)j + '0';
            f = fopen(namf, "w");
            if (f == NULL)
            {
                printf("fputs: can't open file %s\n", namf);
                neot1 = true;
                break;
            }
            uniput[j] = f;
        }
        const T_LINKCB *q = p;
        bool neot2 = false;
        while (q != refal.nexta)
        {
            if (q->tag != TAGO && q->tag != TAGLB && q->tag != TAGRB)
            {
                neot2 = true;
                break;
            }
            q = q->next;
        }
        if (neot2)
            break;
        while (p != refal.nexta)
        {
            int cc;
            if (p->tag == TAGLB)
                cc = '(';
            else if (p->tag == TAGRB)
                cc = ')';
            else
                cc = p->info.infoc;
            putc(cc, f);
            p = p->next;
        }
        putc('\n', f);
        return;
    } while (false);
    if (!neot1)
        printf("fputs: format error\n");
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
    const T_LINKCB *p = refal.preva->next;
    char namf[MAX_FILE_NAME + 1];
    size_t i;
    bool heot = false;
    do
    {
        for (i = 0; p->tag != TAGO || p->info.infoc != '*'; i++)
        {
            if (p->tag != TAGO || i == MAX_FILE_NAME)
            {
                heot = true;
                break;
            }
            namf[i] = p->info.infoc;
            p = p->next;
        }
        if (heot)
            break;
        namf[i] = '\0';
        p = p->next;
        char namt[MAX_FILE_NAME + 1]; // from => to
        for (i = 0; p != refal.nexta; i++)
        {
            if (p->tag != TAGO || i == MAX_FILE_NAME)
            {
                heot = true;
                break;
            }
            namt[i] = p->info.infoc;
            p = p->next;
        }
        if (heot)
            break;
        namt[i] = '\0';
        if (rename(namf, namt) == -1)
            rfabe("rename: error");
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char rename_0[] = {Z6 'R', 'E', 'N', 'A', 'M', 'E', (char)6};
G_L_B uint8_t refalab_rename = '\122';
void (*rename_1)(void) = rename_;

static void exist_file_(void)
{
    T_LINKCB *p = refal.preva->next;
    char namf[MAX_FILE_NAME + 1];
    size_t i;
    for (i = 0; p != refal.nexta; i++)
    {
        if (p->tag != TAGO || i == MAX_FILE_NAME)
        {
            refal.upshot = 2;
            return;
        }
        namf[i] = p->info.infoc;
        p = p->next;
    }
    namf[i] = '\0';
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
    return;
}
char exist_file_0[] = {Z2 'E', 'X', 'I', 'S', 'T', '_', 'F', 'I', 'L', 'E', (char)10};
G_L_B uint8_t refalab_exist_file = '\122';
void (*exist_file_1)(void) = exist_file_;

//----------  end of file XFIO.C  -----------
