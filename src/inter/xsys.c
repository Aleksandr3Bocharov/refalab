// Copyright 2025 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2025-05-07
// https://github.com/Aleksandr3Bocharov/RefalAB

//-----------  file  --  XSYS.C ----------------
//                 MO: arg, system, exit,
//                     get_env, get_working_dir
//----------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include "refalab.h"
#include "rfintf.h"
#include "xsys.h"

static size_t gargc = 0;
static char **gargv = NULL;

void rfgetargs(int argc, char *argv[])
{
    gargc = (size_t)argc;
    gargv = argv;
}

static void arg_(void)
{
    T_LINKCB *p = refal.preva->next;
    if (p->next != refal.nexta || p->tag != TAGN)
    {
        refal.upshot = 2;
        return;
    }
    const size_t argn = gcoden(p);
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
    int sys = system(cmd);
    free(cmd);
#ifdef POSIX
    if (WIFEXITED(sys) != 0)
        sys = WEXITSTATUS(sys);
    else
        sys = -1;
#endif
    p = refal.prevr;
    if (sys == -2147483648)
    {
        if (!slins(p, 3))
            return;
        p = p->next;
        p->info.codep = NULL;
        p->tag = TAGO;
        p->info.infoc = '-';
        p = p->next;
        p->tag = TAGN;
        p->info.codep = NULL;
        pcoden(p, 128);
        p = p->next;
        p->tag = TAGN;
        p->info.codep = NULL;
        return;
    }
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
    if (sys > MAX_NUMBER)
    {
        pcoden(p, (uint32_t)sys >> 24);
        if (!slins(p, 1))
            return;
        p = p->next;
        p->tag = TAGN;
        p->info.codep = NULL;
        sys &= MAX_NUMBER;
    }
    pcoden(p, (uint32_t)sys);
    return;
}
char system_0[] = {Z6 'S', 'Y', 'S', 'T', 'E', 'M', (char)6};
G_L_B uint8_t refalab_system = '\122';
void (*system_1)(void) = system_;

static void exit_(void)
{
    const T_LINKCB *p = refal.preva->next;
    const char zn = p->info.infoc;
    int z = 1;
    if (p->tag == TAGO && (zn == '-' || zn == '+'))
    {
        p = p->next;
        if (zn == '-')
            z = -1;
    }
    if (p->next != refal.nexta || p->tag != TAGN)
    {
        refal.upshot = 2;
        return;
    }
    const int exit_code = z * (int)gcoden(p);
    exit(exit_code);
    return;
}
char exit_0[] = {Z4 'E', 'X', 'I', 'T', (char)4};
G_L_B uint8_t refalab_exit = '\122';
void (*exit_1)(void) = exit_;

static void get_env_(void)
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
    char *env_name = (char *)malloc(i + 1);
    if (env_name == NULL)
        rfabe("get_env: error");
    p = refal.preva->next;
    for (size_t j = 0; j < i; j++)
    {
        *(env_name + j) = p->info.infoc;
        p = p->next;
    }
    *(env_name + i) = '\0';
    const char *env_value = getenv(env_name);
    free(env_name);
    if (env_value == NULL)
        return;
    p = refal.prevr;
    if (!slins(p, strlen(env_value)))
        return;
    for (i = 0; *(env_value + i) != '\0'; i++)
    {
        p = p->next;
        p->tag = TAGO;
        p->info.codep = NULL;
        p->info.infoc = *(env_value + i);
    }
    return;
}
char get_env_0[] = {Z7 'G', 'E', 'T', '_', 'E', 'N', 'V', (char)7};
G_L_B uint8_t refalab_get_env = '\122';
void (*get_env_1)(void) = get_env_;

static void get_current_dir_(void)
{
    if (refal.preva->next != refal.nexta)
    {
        refal.upshot = 2;
        return;
    }
    char *cwd = getcwd(NULL, 0);
    if (cwd == NULL)
        return;
    T_LINKCB *p = refal.prevr;
    if (!slins(p, strlen(cwd)))
        return;
    for (size_t i = 0; *(cwd + i) != '\0'; i++)
    {
        p = p->next;
        p->tag = TAGO;
        p->info.codep = NULL;
        p->info.infoc = *(cwd + i);
    }
    free(cwd);
    return;
}
char get_current_dir_0[] = {Z7 'G', 'E', 'T', '_', 'C', 'U', 'R', 'R', 'E', 'N', 'T', '_', 'D', 'I', 'R', (char)15};
G_L_B uint8_t refalab_get_current_dir = '\122';
void (*get_current_dir_1)(void) = get_current_dir_;

//----------  end of file XSYS.C  ---------------
