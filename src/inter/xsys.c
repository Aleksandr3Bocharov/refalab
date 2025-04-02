// Copyright 2025 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2025-03-30
// https://github.com/Aleksandr3Bocharov/RefalAB

//-----------  file  --  XSYS.C --------------
//                 MO: arg
//--------------------------------------------

#include <string.h>
#include <stdint.h>
#include "refalab.h"
#include "rfintf.h"
#include "xsys.h"

static int gargc = 0;
static char **gargv = NULL;

void rfgetargs(int argc, char *argv[])
{
    gargc = argc;
    gargv = argv;
}

static void arg_(void)
{
    T_LINKCB *p = refal.preva->next;
    do
    {
        if (p->next != refal.nexta || p->tag != TAGN)
            break;
        const uint32_t argn = gcoden(p);
        if (argn >= gargc)
            return;
        p = refal.prevr;
        const char *argvn = gargv[argn];
        if (!slins(p, strlen(argvn)))
            return;
        for (size_t i = 0; argvn[i] != '\0'; i++)
        {
            p = p->next;
            p->tag = TAGO;
            p->info.codep = NULL;
            p->info.infoc = argvn[i];
        }
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char arg_0[] = {Z3 'A', 'R', 'G', (char)3};
G_L_B uint8_t refalab_arg = '\122';
void (*arg_1)(void) = arg_;

static void system_(void)
{
    const T_LINKCB *p = refal.preva->next;
    const T_LINKCB *q = p;
    size_t i;
    for (i = 0; p != refal.nexta; i++)
        if (p->tag != TAGO)
        {
            refal.upshot = 2;
            return;
        }
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
}
char system_0[] = {Z6 'S', 'Y', 'S', 'T', 'E', 'M', (char)6};
G_L_B uint8_t refalab_system = '\122';
void (*system_1)(void) = system_;

//----------  end of file XSYS.C  ---------------
