// Copyright 2025 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2025-09-25
// https://github.com/Aleksandr3Bocharov/RefalAB

//-----------  file  --  XSYS.C --------------------
//                 MO: arg, system, exit, get_env,
//                     change_dir, get_working_dir
//--------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include "refalab.h"
#include "rfintf.h"
#include "xsys.h"

extern uint8_t refalab_null;

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
    if (argn >= gargc || *gargv[argn] == '\0')
        return;
    const int32_t d = (int32_t)strlen(gargv[argn]) - 2;
    if (d > 0)
        if (!slins(refal.nextr, (size_t)d))
            return;
    p = rfrstr(gargv[argn], refal.nextr);
    rftpl(refal.prevr, refal.nextr, p->next);
    return;
}
char arg_0[] = {Z3 'A', 'R', 'G', (char)3};
G_L_B uint8_t refalab_arg = '\122';
void (*arg_1)(void) = arg_;

static void system_(void)
{
    T_LINKCB *p = refal.preva->next;
    char cmd[8192];
    p = rfgstr(cmd, 8191, p);
    if (p != refal.nexta)
    {
        refal.upshot = 2;
        return;
    }
    int sys = system(cmd);
#ifdef POSIX
    if (WIFEXITED(sys) != 0)
        sys = WEXITSTATUS(sys);
    else
        sys = -1;
#endif
    int64_t sys_64 = sys;
    p = refal.preva;
    if (sys_64 < 0)
    {
        if (p->next == refal.nexta)
            if (!slins(p, 1))
                return;
        p->tag = TAGO;
        p->info.codep = NULL;
        p->info.infoc = '-';
        p = p->next;
        sys_64 = -sys_64;
    }
    p->tag = TAGN;
    p->info.codep = NULL;
    pcoden(p, (uint32_t)sys_64);
    rftpl(refal.prevr, refal.nextr, p->next);
    return;
}
char system_0[] = {Z6 'S', 'Y', 'S', 'T', 'E', 'M', (char)6};
G_L_B uint8_t refalab_system = '\122';
void (*system_1)(void) = system_;

static void exit_(void)
{
    do
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
            break;
        const int64_t exit_code_abs = gcoden(p);
        if (z == 1 ? exit_code_abs > 2147483647 : exit_code_abs > 2147483648)
            break;
        const int exit_code = (int)(z * exit_code_abs);
        exit(exit_code);
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char exit_0[] = {Z4 'E', 'X', 'I', 'T', (char)4};
G_L_B uint8_t refalab_exit = '\122';
void (*exit_1)(void) = exit_;

static void get_env_(void)
{
    T_LINKCB *p = refal.preva->next;
    char env_name[32768];
    p = rfgstr(env_name, 32767, p);
    if (p != refal.nexta)
    {
        refal.upshot = 2;
        return;
    }
    const char *env_value = getenv(env_name);
    if (env_value == NULL)
    {
        refal.preva->info.codef = &refalab_null;
        rftpl(refal.prevr, refal.nextr, refal.preva->next);
        return;
    }
    const int32_t d = (int32_t)strlen(env_value) - ((int32_t)strlen(env_name) + 1);
    if (d > 0)
        if (!slins(refal.nextr, (size_t)d))
            return;
    p = rfrstr(env_value, refal.nextr);
    if (p != NULL)
        rftpl(refal.prevr, refal.nextr, p->next);
    return;
}
char get_env_0[] = {Z7 'G', 'E', 'T', '_', 'E', 'N', 'V', (char)7};
G_L_B uint8_t refalab_get_env = '\122';
void (*get_env_1)(void) = get_env_;

static void change_dir_(void)
{
    T_LINKCB *p = refal.preva->next;
    char namd[256];
    p = rfgstr(namd, 255, p);
    if (p != refal.nexta)
    {
        refal.upshot = 2;
        return;
    }
    const int ch = chdir(namd);
    const int err = errno;
    if (ch == -1)
    {
        const char *serr = strerror(err);
        const int32_t d = (int32_t)strlen(serr) - ((int32_t)strlen(namd) + 1);
        if (d > 0)
            if (!slins(refal.nextr, (size_t)d))
                return;
        p = rfrstr(serr, refal.nextr);
        rftpl(refal.prevr, refal.nextr, p->next);
    }
    return;
}
char change_dir_0[] = {Z2 'C', 'H', 'A', 'N', 'G', 'E', '_', 'D', 'I', 'R', (char)10};
G_L_B uint8_t refalab_change_dir = '\122';
void (*change_dir_1)(void) = change_dir_;

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
    if (slins(refal.nextr, strlen(cwd) - 1))
    {
        rfrstr(cwd, refal.nextr);
        rftpl(refal.prevr, refal.nextr, refal.nexta);
    }
    free(cwd);
    return;
}
char get_current_dir_0[] = {Z7 'G', 'E', 'T', '_', 'C', 'U', 'R', 'R', 'E', 'N', 'T', '_', 'D', 'I', 'R', (char)15};
G_L_B uint8_t refalab_get_current_dir = '\122';
void (*get_current_dir_1)(void) = get_current_dir_;

//----------  end of file XSYS.C  ---------------
