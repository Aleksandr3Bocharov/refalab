// Copyright 2025 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2025-09-25
// https://github.com/Aleksandr3Bocharov/RefalAB

//-----------  file  --  XFIO.C ---------------
//           MO: file input/output
//           MO: file/dir remove/rename/exist
//---------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
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
        char namf[256];
        bool neot = false;
        size_t i;
        for (i = 0; p != refal.nexta; i++)
        {
            if (p->tag != TAGO || i == 255)
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
        const int err = errno;
        files[j] = f;
        if (f == NULL)
        {
            const char *serr = strerror(err);
            const int32_t d = (int32_t)strlen(serr) - ((int32_t)strlen(s) + (int32_t)strlen(namf) + 2);
            if (d > 0)
                if (!slins(refal.nextr, (size_t)d))
                    return;
            p = rfrstr(serr, refal.nextr);
            rftpl(refal.prevr, refal.nextr, p->next);
        }
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
            refal.preva->info.codef = &refalab_null;
            rftpl(refal.prevr, refal.nextr, refal.preva->next);
            return;
        }
        const int cl = fclose(f);
        const int err = errno;
        if (cl == EOF)
        {
            const char *serr = strerror(err);
            if (!slins(refal.nextr, strlen(serr) - 2))
                return;
            rfrstr(serr, refal.nextr);
            rftpl(refal.prevr, refal.nextr, refal.nexta);
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
        if (f == NULL)
        {
            refal.preva->info.codef = &refalab_null;
            rftpl(refal.prevr, refal.nextr, refal.preva->next);
            return;
        }
        p = refal.prevr;
        int c = getc(f);
        while (c != '\n')
        {
            if (!slins(p, 1))
                return;
            p = p->next;
            p->info.codep = NULL;
            if (rfreof(c, f, p))
                return;
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
        const T_LINKCB *q = p->next;
        bool neot = false;
        while (q != refal.nexta)
        {
            if (q->tag != TAGO)
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
            refal.preva->info.codef = &refalab_null;
            rftpl(refal.prevr, refal.nextr, refal.preva->next);
            return;
        }
        p = p->next;
        while (p != refal.nexta)
        {
            const int cc = p->info.infoc;
            const int pcc = putc(cc, f);
            if (rfreof(pcc, f, refal.preva))
            {
                rftpl(refal.prevr, refal.nextr, refal.preva->next);
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

static void fprint_(void)
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
        if (f == NULL)
        {
            refal.preva->info.codef = &refalab_null;
            rftpl(refal.prevr, refal.nextr, refal.preva->next);
            return;
        }
        p = p->next;
        while (p != refal.nexta)
        {
            int pcc = 0;
            char s[512];
            if (p->tag == TAGO)
                pcc = putc(p->info.infoc, f);
            else if (p->tag == TAGLB)
                pcc = putc('(', f);
            else if (p->tag == TAGRB)
                pcc = putc(')', f);
            else if (p->tag == TAGN)
            {
                sprintf(s, "'%u'", gcoden(p));
                pcc = fputs(s, f);
            }
            else if (p->tag == TAGF)
            {
                sprintf(s, "'");
                const char *n = (char *)(p->info.codef) - 1;
                const uint8_t l = (uint8_t)*n;
                n -= l;
                char ch[2];
                ch[1] = '\0';
                for (size_t k = 1; k <= l; k++, n++)
                {
                    ch[0] = (char)toupper(*n);
                    strcat(s, ch);
                }
                strcat(s, "'");
                pcc = fputs(s, f);
            }
            else if (p->tag == TAGR)
            {
                sprintf(s, "'%%%p'", (void *)p->info.codep);
                pcc = fputs(s, f);
            }
            else if ((p->tag & 0001) != TAGO)
                rfabe("fprint: unknown bracket type");
            else
            {
                sprintf(s, "'%x,%p'", p->tag, (void *)p->info.codep);
                pcc = fputs(s, f);
            }
            if (rfreof(pcc, f, refal.preva))
            {
                rftpl(refal.prevr, refal.nextr, refal.preva->next);
                return;
            }
            p = p->next;
        }
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char fprint_0[] = {Z6 'F', 'P', 'R', 'I', 'N', 'T', (char)6};
G_L_B uint8_t refalab_fprint = '\122';
void (*fprint_1)(void) = fprint_;

static void fprints_(void)
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
        if (f == NULL)
        {
            refal.preva->info.codef = &refalab_null;
            rftpl(refal.prevr, refal.nextr, refal.preva->next);
            return;
        }
        p = p->next;
        while (p != refal.nexta)
        {
            int pcc = 0;
            char s[512];
            if (p->tag == TAGO)
                pcc = putc(p->info.infoc, f);
            else if (p->tag == TAGLB)
                pcc = putc('(', f);
            else if (p->tag == TAGRB)
                pcc = putc(')', f);
            else if (p->tag == TAGN)
            {
                sprintf(s, "%u", gcoden(p));
                pcc = fputs(s, f);
            }
            else if (p->tag == TAGF)
            {
                s[0] = '\0';
                const char *n = (char *)(p->info.codef) - 1;
                const uint8_t l = (uint8_t)*n;
                n -= l;
                char ch[2];
                ch[1] = '\0';
                for (size_t k = 1; k <= l; k++, n++)
                {
                    ch[0] = (char)toupper(*n);
                    strcat(s, ch);
                }
                pcc = fputs(s, f);
            }
            else if (p->tag == TAGR)
            {
                sprintf(s, "%%%p", (void *)p->info.codep);
                pcc = fputs(s, f);
            }
            else if ((p->tag & 0001) != TAGO)
                rfabe("fprints: unknown bracket type");
            else
            {
                sprintf(s, "%x,%p", p->tag, (void *)p->info.codep);
                pcc = fputs(s, f);
            }
            if (rfreof(pcc, f, refal.preva))
            {
                rftpl(refal.prevr, refal.nextr, refal.preva->next);
                return;
            }
            p = p->next;
        }
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char fprints_0[] = {Z7 'F', 'P', 'R', 'I', 'N', 'T', 'S', (char)7};
G_L_B uint8_t refalab_fprints = '\122';
void (*fprints_1)(void) = fprints_;

static void fprintm_(void)
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
        if (f == NULL)
        {
            refal.preva->info.codef = &refalab_null;
            rftpl(refal.prevr, refal.nextr, refal.preva->next);
            return;
        }
        p = p->next;
        bool fr = false;
        while (p != refal.nexta)
        {
            int pcc = 0;
            char s[512];
            if (p->tag == TAGO)
            {
                if (!fr)
                {
                    fr = true;
                    sprintf(s, "'%c", p->info.infoc);
                    pcc = fputs(s, f);
                }
                else
                    pcc = putc(p->info.infoc, f);
            }
            else
            {
                if (fr)
                {
                    fr = false;
                    sprintf(s, "'");
                }
                else
                    s[0] = '\0';
                if (p->tag == TAGLB)
                    strcat(s, "(");
                else if (p->tag == TAGRB)
                    strcat(s, ")");
                else if (p->tag == TAGN)
                {
                    char sn[512];
                    sprintf(sn, "%u", gcoden(p));
                    strcat(s, sn);
                    if (p->next->tag == TAGN)
                        strcat(s, " ");
                }
                else if (p->tag == TAGF)
                {
                    strcat(s, "&");
                    const char *n = (char *)(p->info.codef) - 1;
                    const uint8_t l = (uint8_t)*n;
                    n -= l;
                    char ch[2];
                    ch[1] = '\0';
                    for (size_t k = 1; k <= l; k++, n++)
                    {
                        ch[0] = (char)toupper(*n);
                        strcat(s, ch);
                    }
                    if (p->next->tag == TAGN)
                        strcat(s, " ");
                }
                else if (p->tag == TAGR)
                {
                    char sr[512];
                    sprintf(sr, "/%%%p/", (void *)p->info.codep);
                    strcat(s, sr);
                }
                else if ((p->tag & 0001) != TAGO)
                    rfabe("fprintm: unknown bracket type");
                else
                {
                    char sp[512];
                    sprintf(sp, "/%x,%p/", p->tag, (void *)p->info.codep);
                    strcat(s, sp);
                }
                pcc = fputs(s, f);
            }
            if (rfreof(pcc, f, refal.preva))
            {
                rftpl(refal.prevr, refal.nextr, refal.preva->next);
                return;
            }
            p = p->next;
        }
        if (fr)
        {
            const int pcc = putc('\'', f);
            if (rfreof(pcc, f, refal.preva))
                rftpl(refal.prevr, refal.nextr, refal.preva->next);
        }
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char fprintm_0[] = {Z7 'F', 'P', 'R', 'I', 'N', 'T', 'M', (char)7};
G_L_B uint8_t refalab_fprintm = '\122';
void (*fprintm_1)(void) = fprintm_;

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
        if (f == NULL)
        {
            refal.preva->info.codef = &refalab_null;
            rftpl(refal.prevr, refal.nextr, refal.preva->next);
            return;
        }
        p = refal.prevr;
        if (!slins(p, count))
            return;
        for (; count > 0; count--)
        {
            p = p->next;
            p->info.codep = NULL;
            const int c = getc(f);
            if (rfreof(c, f, p))
            {
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
        const T_LINKCB *q = p->next;
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
            refal.preva->info.codef = &refalab_null;
            rftpl(refal.prevr, refal.nextr, refal.preva->next);
            return;
        }
        p = p->next;
        while (p != refal.nexta)
        {
            int cc;
            if (p->tag == TAGO)
                cc = p->info.infoc;
            else
                cc = (uint8_t)gcoden(p);
            const int pcc = putc(cc, f);
            if (rfreof(pcc, f, refal.preva))
            {
                rftpl(refal.prevr, refal.nextr, refal.preva->next);
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
        int64_t z = 1;
        if (p->tag == TAGO && (zn == '-' || zn == '+'))
        {
            p = p->next;
            if (zn == '-')
                z = -1;
        }
        if (p->tag != TAGN)
            break;
        const int64_t offset_abs = gcoden(p);
        if (z == 1 ? offset_abs > 2147483647 : offset_abs > 2147483648)
            break;
        const long int offset = (long int)(z * offset_abs);
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
            refal.preva->info.codef = &refalab_null;
            rftpl(refal.prevr, refal.nextr, refal.preva->next);
            return;
        }
        const int res = fseek(f, offset, origin);
        const int err = errno;
        if (res == -1)
        {
            const char *serr = strerror(err);
            if (!slins(refal.nextr, strlen(serr) - 3 - (z == 1 ? 1 : 2)))
                return;
            rfrstr(serr, refal.nextr);
            rftpl(refal.prevr, refal.nextr, refal.nexta);
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
        if (f == NULL)
        {
            refal.preva->info.codef = &refalab_null;
            rftpl(refal.prevr, refal.nextr, refal.preva->next);
            return;
        }
        long int res = ftell(f);
        const int err = errno;
        if (res == -1)
        {
            const char *serr = strerror(err);
            if (!slins(refal.nextr, strlen(serr) - 2))
                return;
            rfrstr(serr, refal.nextr);
            rftpl(refal.prevr, refal.nextr, refal.nexta);
            return;
        }
        pcoden(p, (uint32_t)res);
        rftpl(refal.prevr, refal.preva, refal.nexta);
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char ftell_0[] = {Z5 'F', 'T', 'E', 'L', 'L', (char)5};
G_L_B uint8_t refalab_ftell = '\122';
void (*ftell_1)(void) = ftell_;

static void is_eof_(void)
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
        if (f == NULL)
        {
            refal.preva->info.codef = &refalab_null;
            rftpl(refal.prevr, refal.nextr, refal.preva->next);
            return;
        }
        if (feof(f) != 0 || ferror(f) != 0)
            refal.preva->info.codef = &refalab_true;
        else
            refal.preva->info.codef = &refalab_false;
        rftpl(refal.prevr, refal.nextr, refal.preva->next);
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char is_eof_0[] = {Z6 'I', 'S', '_', 'E', 'O', 'F', (char)6};
G_L_B uint8_t refalab_is_eof = '\122';
void (*is_eof_1)(void) = is_eof_;

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
        if (f == NULL)
        {
            refal.preva->info.codef = &refalab_null;
            rftpl(refal.prevr, refal.nextr, refal.preva->next);
            return;
        }
        if (feof(f) != 0)
            refal.preva->info.codef = &refalab_true;
        else
            refal.preva->info.codef = &refalab_false;
        rftpl(refal.prevr, refal.nextr, refal.preva->next);
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
        if (f == NULL)
        {
            refal.preva->info.codef = &refalab_null;
            rftpl(refal.prevr, refal.nextr, refal.preva->next);
            return;
        }
        if (ferror(f) != 0)
            refal.preva->info.codef = &refalab_true;
        else
            refal.preva->info.codef = &refalab_false;
        rftpl(refal.prevr, refal.nextr, refal.preva->next);
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
    char namf[256];
    size_t i;
    for (i = 0; p != refal.nexta; i++)
    {
        if (p->tag != TAGO || i == 255)
        {
            refal.upshot = 2;
            return;
        }
        namf[i] = p->info.infoc;
        p = p->next;
    }
    namf[i] = '\0';
    const int u = unlink(namf);
    const int err = errno;
    if (u == -1)
    {
        const char *serr = strerror(err);
        const int32_t d = (int32_t)strlen(serr) - ((int32_t)strlen(namf) + 1);
        if (d > 0)
            if (!slins(refal.nextr, (size_t)d))
                return;
        p = rfrstr(serr, refal.nextr);
        rftpl(refal.prevr, refal.nextr, p->next);
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
    const int32_t al = (int32_t)strlen(namf) + (int32_t)strlen(namt) + 2;
    free(namf);
    free(namt);
    if (r == -1)
    {
        const char *serr = strerror(err);
        // const int32_t al = (int32_t)strlen(namf) + (int32_t)strlen(namt) + 2;
        const int32_t d = (int32_t)strlen(serr) - al;
        if (d > 0)
            if (!slins(refal.nextr, (size_t)d))
                return;
        p = rfrstr(serr, refal.nextr);
        rftpl(refal.prevr, refal.nextr, p->next);
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
    struct stat st_buf;
    if (stat(namf, &st_buf) == 0 && S_ISREG(st_buf.st_mode))
        refal.preva->info.codef = &refalab_true;
    else
        refal.preva->info.codef = &refalab_false;
    rftpl(refal.prevr, refal.nextr, refal.preva->next);
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
    struct stat st_buf;
    if (stat(namd, &st_buf) == 0 && S_ISDIR(st_buf.st_mode))
        refal.preva->info.codef = &refalab_true;
    else
        refal.preva->info.codef = &refalab_false;
    rftpl(refal.prevr, refal.nextr, refal.preva->next);
    free(namd);
    return;
}
char exist_dir_0[] = {Z1 'E', 'X', 'I', 'S', 'T', '_', 'D', 'I', 'R', (char)9};
G_L_B uint8_t refalab_exist_dir = '\122';
void (*exist_dir_1)(void) = exist_dir_;

//----------  end of file XFIO.C  -----------
