// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-05-05
// https://github.com/Aleksandr3Bocharov/refalab

//-----------  file  --  XSYS.C --------------------
//                 MO: arg, system, exit, get_env,
//                     change_dir, get_current_dir,
//                     step
//--------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include "refalab.h"
#include "rfintf.h"

extern uint8_t refalab_null;

static void arg_(void)
{
    T_LINKCB *symbol_number = refal.previous_argument->next;
    if (symbol_number->next != refal.next_argument || symbol_number->tag != TAGN)
    {
        refal.upshot = 2;
        return;
    }
    const size_t arg_number = gcoden(symbol_number);
    if (arg_number >= refal.arg.argc || *refal.arg.argv[arg_number] == '\0')
        return;
    const size_t arg_length = strlen(refal.arg.argv[arg_number]);
    if (arg_length > 2)
        if (!extended_insert_from_free_memory(refal.next_result, arg_length - 2))
            return;
    T_LINKCB *last_arg_argument = set_string_expression(refal.arg.argv[arg_number], refal.next_result);
    transplantation(refal.previous_result, refal.next_result, last_arg_argument->next);
    return;
}
char arg_0[] = {Z3 'A', 'R', 'G', (char)3};
G_L_B uint8_t refalab_arg = '\122';
void (*arg_1)(void) = arg_;

static void system_(void)
{
    T_LINKCB *current_argument = refal.previous_argument->next;
    char command[8192];
    current_argument = get_string_expression(command, 8191, current_argument, refal.next_argument);
    if (current_argument != refal.next_argument)
    {
        refal.upshot = 2;
        return;
    }
    int system_result = system(command);
#if defined POSIX
    if (WIFEXITED(system_result) != 0)
        system_result = WEXITSTATUS(system_result);
    else
        system_result = -1;
#endif
    int64_t system_result_x64 = system_result;
    T_LINKCB *number_argument = refal.previous_argument;
    if (system_result_x64 < 0)
    {
        if (number_argument->next == refal.next_argument)
            if (!extended_insert_from_free_memory(number_argument, 1))
                return;
        number_argument->tag = TAGO;
        number_argument->info.code = NULL;
        number_argument->info.infoc = '-';
        number_argument = number_argument->next;
        system_result_x64 = -system_result_x64;
    }
    number_argument->tag = TAGN;
    number_argument->info.code = NULL;
    pcoden(number_argument, (uint32_t)system_result_x64);
    transplantation(refal.previous_result, refal.next_result, number_argument->next);
    return;
}
char system_0[] = {Z6 'S', 'Y', 'S', 'T', 'E', 'M', (char)6};
G_L_B uint8_t refalab_system = '\122';
void (*system_1)(void) = system_;

static void exit_(void)
{
    do
    {
        const T_LINKCB *current_argument = refal.previous_argument->next;
        const char sign = current_argument->info.infoc;
        int sign_digit = 1;
        if (current_argument->tag == TAGO && (sign == '-' || sign == '+'))
        {
            current_argument = current_argument->next;
            if (sign == '-')
                sign_digit = -1;
        }
        if (current_argument->next != refal.next_argument || current_argument->tag != TAGN)
            break;
        const int64_t exit_code_absolute = gcoden(current_argument);
        if (sign_digit == 1 ? exit_code_absolute > 2147483647 : exit_code_absolute > 2147483648)
            break;
        const int exit_code = (int)(sign_digit * exit_code_absolute);
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
    T_LINKCB *current_argument = refal.previous_argument->next;
    char environment_name[32768];
    current_argument = get_string_expression(environment_name, 32767, current_argument, refal.next_argument);
    if (current_argument != refal.next_argument)
    {
        refal.upshot = 2;
        return;
    }
    const char *environment_value = getenv(environment_name);
    if (environment_value == NULL)
    {
        refal.previous_argument->info.codef = &refalab_null;
        transplantation(refal.previous_result, refal.previous_argument->previous, refal.previous_argument->next);
        return;
    }
    const int32_t need_yet_result = (int32_t)strlen(environment_value) - ((int32_t)strlen(environment_name) + 1);
    if (need_yet_result > 0)
        if (!extended_insert_from_free_memory(refal.next_result, (size_t)need_yet_result))
            return;
    T_LINKCB *last_environment_value_argument = set_string_expression(environment_value, refal.next_result);
    if (last_environment_value_argument != NULL)
        transplantation(refal.previous_result, refal.next_result, last_environment_value_argument->next);
    return;
}
char get_env_0[] = {Z7 'G', 'E', 'T', '_', 'E', 'N', 'V', (char)7};
G_L_B uint8_t refalab_get_env = '\122';
void (*get_env_1)(void) = get_env_;

static void change_dir_(void)
{
    T_LINKCB *current_argument = refal.previous_argument->next;
    char directory_name[MAX_PATHFILENAME + 1];
    current_argument = get_string_expression(directory_name, MAX_PATHFILENAME, current_argument, refal.next_argument);
    if (current_argument != refal.next_argument)
    {
        refal.upshot = 2;
        return;
    }
    const int change_directory_result = chdir(directory_name);
    if (change_directory_result == -1)
    {
        const int error_number = errno;
        const char *string_error = strerror(error_number);
        const int32_t result_yet_need = (int32_t)strlen(string_error) - ((int32_t)strlen(directory_name) + 1);
        if (result_yet_need > 0)
            if (!extended_insert_from_free_memory(refal.next_result, (size_t)result_yet_need))
                return;
        T_LINKCB *last_error_argument = set_string_expression(string_error, refal.next_result);
        transplantation(refal.previous_result, refal.next_result, last_error_argument->next);
    }
    return;
}
char change_dir_0[] = {Z2 'C', 'H', 'A', 'N', 'G', 'E', '_', 'D', 'I', 'R', (char)10};
G_L_B uint8_t refalab_change_dir = '\122';
void (*change_dir_1)(void) = change_dir_;

static void get_current_dir_(void)
{
    if (refal.previous_argument->next != refal.next_argument)
    {
        refal.upshot = 2;
        return;
    }
    char *get_current_directory_result = getcwd(NULL, 0);
    if (get_current_directory_result == NULL)
        return;
    if (extended_insert_from_free_memory(refal.next_result, strlen(get_current_directory_result) - 1))
    {
        set_string_expression(get_current_directory_result, refal.next_result);
        transplantation(refal.previous_result, refal.next_result, refal.next_argument);
    }
    free(get_current_directory_result);
    return;
}
char get_current_dir_0[] = {Z7 'G', 'E', 'T', '_', 'C', 'U', 'R', 'R', 'E', 'N', 'T', '_', 'D', 'I', 'R', (char)15};
G_L_B uint8_t refalab_get_current_dir = '\122';
void (*get_current_dir_1)(void) = get_current_dir_;

static void step_(void)
{
    if (refal.previous_argument->next != refal.next_argument)
    {
        refal.upshot = 2;
        return;
    }
    refal.previous_argument->tag = TAGN;
    refal.previous_argument->info.code = NULL;
    pcoden(refal.previous_argument, refal.current_status_table->step);
    transplantation(refal.previous_result, refal.previous_argument->previous, refal.previous_argument->next);
    return;
}
char step_0[] = {Z4 'S', 'T', 'E', 'P', (char)4};
G_L_B uint8_t refalab_step = '\122';
void (*step_1)(void) = step_;

//----------  end of file XSYS.C  ---------------
