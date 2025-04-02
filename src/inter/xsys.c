// Copyright 2025 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2025-04-01
// https://github.com/Aleksandr3Bocharov/RefalAB

//-----------  file  --  XSYS.C --------------
//                 MO: arg
//--------------------------------------------

#include <stdio.h>
#include <stdlib.h>
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
    char *cmd = (char *)malloc(i + 1);
    if (cmd == NULL)
        rfabe("system: error");
    p = refal.preva->next;
    for (size_t j = 0; j < i; j++)
    {
        *(cmd + j) = p->info.infoc;
        p = p->next;
    }
    *(cmd + i) = '\0';
    // fflush(NULL);
    int sys = system(cmd);
    free(cmd);
    p = refal.prevr;
    if (sys < 0)
    {
        if (!slins(p, 1))
            return;
        p = p->next;
        p->info.codep = NULL;
        p->tag = TAGO;
        p->info.infoc = '-';
        sys = -sys;
    }
    if (!slins(p, 1))
        return;
    p = p->next;
    p->tag = TAGN;
    p->info.codep = NULL;
    if (sys > 16777215)
    {
        pcoden(p, sys >> 24);
        if (!slins(p, 1))
            return;
        p = p->next;
        p->tag = TAGN;
        p->info.codep = NULL;
        sys &= 0xffffff;
    }
    pcoden(p, sys);
    return;
}
char system_0[] = {Z6 'S', 'Y', 'S', 'T', 'E', 'M', (char)6};
G_L_B uint8_t refalab_system = '\122';
void (*system_1)(void) = system_;

//----------  end of file XSYS.C  ---------------
