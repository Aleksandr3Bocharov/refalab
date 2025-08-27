// Copyright 2025 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2025-08-27
// https://github.com/Aleksandr3Bocharov/RefalAB

//-----------  file  --  XFIO.C ---------------
//           MO: file input/output
//           MO: file/dir remove/rename/exist
//---------------------------------------------

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

#define fmax 10

extern uint8_t refalab_true, refalab_false;
extern uint8_t refalab_null;
extern uint8_t refalab_begin, refalab_end, refalab_cur;

static FILE *f;
static FILE *files[fmax] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

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
        char c = p->info.infoc;
        char s[3];
        s[1] = '\0';
        s[2] = '\0';
        if (c == 'R' || c == 'r')
            s[0] = 'r';
        else if (c == 'W' || c == 'w')
            s[0] = 'w';
        else if (c == 'A' || c == 'a')
            s[0] = 'a';
        else
            break;
        p = p->next;
        if (p->tag == TAGO)
        {
            c = p->info.infoc;
            if (c == 'B' || c == 'b')
                s[1] = 'b';
            else
                break;
            p = p->next;
        }
        if (p->tag != TAGN)
            break;
        const uint32_t j = gcoden(p);
        if (j >= fmax)
            break;
        p = p->next;
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
            rfabe("fopen: malloc error");
        p = q;
        for (size_t k = 0; k < i; k++)
        {
            *(namf + k) = p->info.infoc;
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
        }
        files[j] = f;
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
        if (p->tag != TAGN)
            break;
        const uint32_t j = gcoden(p);
        if (j >= fmax)
            break;
        if (p->next != refal.nexta)
            break;
        f = files[j];
        files[j] = NULL;
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
        if (p->tag == TAGN)
        {
            const uint32_t j = gcoden(p);
            if (j >= fmax)
                break;
            f = files[j];
        }
        else if (p->tag == TAGF)
        {
            if (p->info.codef != &refalab_stdin)
                break;
            f = stdin;
        }
        else
            break;
        if (p->next != refal.nexta)
            break;
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
            enum
            {
                OK,
                FEOF,
                FERROR
            } err = OK;
            if (c == EOF)
            {
                if (feof(f) != 0)
                    err = FEOF;
                else if (ferror(f) != 0)
                    err = FERROR;
            }
            if (err != OK)
            {
                p->tag = TAGF;
                if (err == FEOF)
                    p->info.codef = &refalab_feof;
                else
                    p->info.codef = &refalab_ferror;
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
        if (p->tag == TAGN)
        {
            const uint32_t j = gcoden(p);
            if (j >= fmax)
                break;
            f = files[j];
        }
        else if (p->tag == TAGF)
        {
            if (p->info.codef == &refalab_stdout)
                f = stdout;
            else if (p->info.codef == &refalab_stderr)
                f = stderr;
            else
                break;
        }
        else
            break;
        p = p->next;
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
            const int pcc = putc(cc, f);
            enum
            {
                OK,
                FEOF,
                FERROR
            } err = OK;
            if (pcc == EOF)
            {
                if (feof(f) != 0)
                    err = FEOF;
                else if (ferror(f) != 0)
                    err = FERROR;
            }
            if (err != OK)
            {
                p = refal.prevr;
                if (!slins(p, 1))
                    return;
                p = p->next;
                p->tag = TAGF;
                if (err == FEOF)
                    p->info.codef = &refalab_feof;
                else
                    p->info.codef = &refalab_ferror;
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

static void fread_(void)
{
    do
    {
        T_LINKCB *p = refal.preva->next;
        if (p->tag != TAGN)
            break;
        const uint32_t j = gcoden(p);
        if (j >= fmax)
            break;
        f = files[j];
        p = p->next;
        if (p->tag != TAGN)
            break;
        uint32_t count = gcoden(p);
        if (p->next != refal.nexta)
            break;
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
        if (!slins(p, count))
            return;
        for (; count > 0; count--)
        {
            p = p->next;
            p->info.codep = NULL;
            const int c = getc(f);
            enum
            {
                OK,
                FEOF,
                FERROR
            } err = OK;
            if (c == EOF)
            {
                if (feof(f) != 0)
                    err = FEOF;
                else if (ferror(f) != 0)
                    err = FERROR;
            }
            if (err != OK)
            {
                p->tag = TAGF;
                if (err == FEOF)
                    p->info.codef = &refalab_feof;
                else
                    p->info.codef = &refalab_ferror;
                rfdel(p, refal.nextr);
                return;
            }
            p->tag = TAGN;
            pcoden(p, (uint8_t)c);
        }
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char fread_0[] = {Z5 'F', 'R', 'E', 'A', 'D', (char)5};
G_L_B uint8_t refalab_fread = '\122';
void (*fread_1)(void) = fread_;

static void fwrite_(void)
{
    do
    {
        T_LINKCB *p = refal.preva->next;
        if (p->tag != TAGN)
            break;
        const uint32_t j = gcoden(p);
        if (j >= fmax)
            break;
        f = files[j];
        p = p->next;
        const T_LINKCB *q = p;
        bool neot = false;
        while (q != refal.nexta)
        {
            if (q->tag != TAGO && q->tag != TAGN)
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
            if (p->tag == TAGO)
                cc = p->info.infoc;
            else
                cc = (uint8_t)gcoden(p);
            const int pcc = putc(cc, f);
            enum
            {
                OK,
                FEOF,
                FERROR
            } err = OK;
            if (pcc == EOF)
            {
                if (feof(f) != 0)
                    err = FEOF;
                else if (ferror(f) != 0)
                    err = FERROR;
            }
            if (err != OK)
            {
                p = refal.prevr;
                if (!slins(p, 1))
                    return;
                p = p->next;
                p->tag = TAGF;
                if (err == FEOF)
                    p->info.codef = &refalab_feof;
                else
                    p->info.codef = &refalab_ferror;
                return;
            }
            p = p->next;
        }
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char fwrite_0[] = {Z6 'F', 'W', 'R', 'I', 'T', 'E', (char)6};
G_L_B uint8_t refalab_fwrite = '\122';
void (*fwrite_1)(void) = fwrite_;

static void fseek_(void)
{
    do
    {
        T_LINKCB *p = refal.preva->next;
        if (p->tag != TAGN)
            break;
        const uint32_t j = gcoden(p);
        if (j >= fmax)
            break;
        f = files[j];
        p = p->next;
        const char zn = p->info.infoc;
        long int z = 1;
        if (p->tag == TAGO && (zn == '-' || zn == '+'))
        {
            p = p->next;
            if (zn == '-')
                z = -1;
        }
        if (p->tag != TAGN)
            break;
        const long int offset = z * (long int)gcoden(p);
        p = p->next;
        if (p->tag != TAGF)
            break;
        int origin;
        if (p->info.codef == &refalab_begin)
            origin = SEEK_SET;
        else if (p->info.codef == &refalab_end)
            origin = SEEK_END;
        else if (p->info.codef == &refalab_cur)
            origin = SEEK_CUR;
        else
            break;
        if (p->next != refal.nexta)
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
        const int res = fseek(f, offset, origin);
        const int err = errno;
        if (res == -1)
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
char fseek_0[] = {Z5 'F', 'S', 'E', 'E', 'K', (char)5};
G_L_B uint8_t refalab_fseek = '\122';
void (*fseek_1)(void) = fseek_;

static void ftell_(void)
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
        f = files[j];
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
        long int res = ftell(f);
        const int err = errno;
        if (res == -1)
        {
            char *serr = strerror(err);
            if (!slins(p, strlen(serr)))
                return;
            for (size_t i = 0; *(serr + i) != '\0'; i++)
            {
                p = p->next;
                p->tag = TAGO;
                p->info.codep = NULL;
                p->info.infoc = *(serr + i);
            }
            return;
        }
        if (!slins(p, 1))
            return;
        p = p->next;
        p->tag = TAGN;
        p->info.codep = NULL;
        if (res > MAX_NUMBER)
        {
            pcoden(p, (uint32_t)res >> 24);
            if (!slins(p, 1))
                return;
            p = p->next;
            p->tag = TAGN;
            p->info.codep = NULL;
            res &= MAX_NUMBER;
        }
        pcoden(p, (uint32_t)res);
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char ftell_0[] = {Z5 'F', 'T', 'E', 'L', 'L', (char)5};
G_L_B uint8_t refalab_ftell = '\122';
void (*ftell_1)(void) = ftell_;

static void is_feof_(void)
{
    do
    {
        T_LINKCB *p = refal.preva->next;
        if (p->tag == TAGN)
        {
            const uint32_t j = gcoden(p);
            if (j >= fmax)
                break;
            f = files[j];
        }
        else if (p->tag == TAGF)
        {
            if (p->info.codef == &refalab_stdin)
                f = stdin;
            else if (p->info.codef == &refalab_stdout)
                f = stdout;
            else if (p->info.codef == &refalab_stderr)
                f = stderr;
            else
                break;
        }
        else
            break;
        if (p->next != refal.nexta)
            break;
        p->tag = TAGF;
        if (f == NULL)
            p->info.codef = &refalab_null;
        else if (feof(f) != 0)
            p->info.codef = &refalab_true;
        else
            p->info.codef = &refalab_false;
        rftpl(refal.prevr, p->prev, p->next);
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char is_feof_0[] = {Z7 'I', 'S', '_', 'F', 'E', 'O', 'F', (char)7};
G_L_B uint8_t refalab_is_feof = '\122';
void (*is_feof_1)(void) = is_feof_;

static void is_ferror_(void)
{
    do
    {
        T_LINKCB *p = refal.preva->next;
        if (p->tag == TAGN)
        {
            const uint32_t j = gcoden(p);
            if (j >= fmax)
                break;
            f = files[j];
        }
        else if (p->tag == TAGF)
        {
            if (p->info.codef == &refalab_stdin)
                f = stdin;
            else if (p->info.codef == &refalab_stdout)
                f = stdout;
            else if (p->info.codef == &refalab_stderr)
                f = stderr;
            else
                break;
        }
        else
            break;
        if (p->next != refal.nexta)
            break;
        p->tag = TAGF;
        if (f == NULL)
            p->info.codef = &refalab_null;
        else if (ferror(f) != 0)
            p->info.codef = &refalab_true;
        else
            p->info.codef = &refalab_false;
        rftpl(refal.prevr, p->prev, p->next);
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char is_ferror_0[] = {Z1 'I', 'S', '_', 'F', 'E', 'R', 'R', 'O', 'R', (char)9};
G_L_B uint8_t refalab_is_ferror = '\122';
void (*is_ferror_1)(void) = is_ferror_;

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
        rfabe("remove_file: malloc error");
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
        rfabe("rename: malloc error");
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
        rfabe("rename: malloc error");
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
        rfabe("exist_file: malloc error");
    p = refal.preva->next;
    for (size_t j = 0; j < i; j++)
    {
        *(namf + j) = p->info.infoc;
        p = p->next;
    }
    *(namf + i) = '\0';
    p = refal.prevr;
    if (!slins(p, 1))
    {
        free(namf);
        return;
    }
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

static void exist_dir_(void)
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
    char *namd = (char *)malloc(i + 1);
    if (namd == NULL)
        rfabe("exist_dir: malloc error");
    p = refal.preva->next;
    for (size_t j = 0; j < i; j++)
    {
        *(namd + j) = p->info.infoc;
        p = p->next;
    }
    *(namd + i) = '\0';
    p = refal.prevr;
    if (!slins(p, 1))
    {
        free(namd);
        return;
    }
    p = p->next;
    p->tag = TAGF;
    p->info.codef = &refalab_false;
    struct stat st_buf;
    if (stat(namd, &st_buf) == 0)
        if (S_ISDIR(st_buf.st_mode))
            p->info.codef = &refalab_true;
    free(namd);
    return;
}
char exist_dir_0[] = {Z1 'E', 'X', 'I', 'S', 'T', '_', 'D', 'I', 'R', (char)9};
G_L_B uint8_t refalab_exist_dir = '\122';
void (*exist_dir_1)(void) = exist_dir_;

//----------  end of file XFIO.C  -----------
