// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-06-11
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file print_errors.c  ---------- 
//               Print error file                           
//------------------------------------------- 

#include <stdio.h>
#include <stdint.h>
#include "refalab.h"
#include "print_errors.h"
#include "compiler.h"

void print_error_string(uint16_t error_number, const char *error_string)
{
    char tmp[512];
    size_t line, column;
    get_location(&line, &column, scanner.last_error_cursor);
    sprintf(tmp, "***** Line %zuб Column %zu: [ERROR %u] %s\n", line, column, error_number, error_string);
    if (refalab_source_listing != NULL)
        fputs(tmp, refalab_source_listing);
    fputs(tmp, terminal);
    processing_error();
    return;
}

void print_error_two_strings(uint16_t error_number, const char *error_string, const char *error2_string, uint8_t error2_string_length)
{
    processing_error();
    char tmp1[512];
    for (uint8_t i = 0; i < error2_string_length; i++)
        tmp1[i] = *(error2_string + i);
    tmp1[error2_string_length] = '\0';
    char tmp[512];
    sprintf(tmp, "***** %s %s\n", error_string, tmp1);
    if (refalab_source_listing != NULL)
        fputs(tmp, refalab_source_listing);
    fputs(tmp, terminal);
    return;
}

void print_error_three_strings(uint16_t error_number, const char *error_string, const char *error2_string, uint8_t error2_string_length, const char *error3_string)
{
    processing_error();
    char tmp1[512];
    for (uint8_t i = 0; i < error2_string_length; i++)
        tmp1[i] = *(error2_string + i);
    tmp1[error2_string_length] = '\0';
    char tmp[512];
    sprintf(tmp, "***** %s %s %s\n", error_string, tmp1, error3_string);
    if (refalab_source_listing != NULL)
        fputs(tmp, refalab_source_listing);
    fputs(tmp, terminal);
    return;
}

void print_error_string_symbol(uint16_t error_number, const char *error_string, const char symbol)
{
    processing_error();
    char tmp[512];
    sprintf(tmp, "***** %s %c\n", error_string, symbol);
    if (refalab_source_listing != NULL)
        fputs(tmp, refalab_source_listing);
    fputs(tmp, terminal);
    return;
}

//----------  end of file print_errors.c  ---------- 
