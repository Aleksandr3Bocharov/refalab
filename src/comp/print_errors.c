// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-06-11
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file print_errors.c  ---------- 
//               Print error file                           
//------------------------------------------- 

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "refalab.h"
#include "print_errors.h"
#include "compiler.h"

void print_error_string(uint16_t error_number, const char *error_string)
{
    char tmp[512];
    size_t line, column;
    get_location(&line, &column, scanner.last_error_cursor);
    sprintf(tmp, "***** Line %zu, Column %zu: [ERROR %u] %s\n", line, column, error_number, error_string);
    if (refalab_source_listing != NULL)
        fputs(tmp, refalab_source_listing);
    fputs(tmp, terminal);
    processing_error();
    return;
}

void print_error_two_strings(uint16_t error_number, const char *error_string, const char *error2_string, uint8_t error2_string_length)
{
    char tmp1[512];
    memcpy(tmp1, error2_string, error2_string_length);
    tmp1[error2_string_length] = '\0';
    char tmp[512];
    size_t line, column;
    get_location(&line, &column, scanner.last_error_cursor);
    sprintf(tmp, "***** Line %zu, Column %zu: [ERROR %u] %s %s\n", line, column, error_number, error_string, tmp1);
    if (refalab_source_listing != NULL)
        fputs(tmp, refalab_source_listing);
    fputs(tmp, terminal);
    processing_error();
    return;
}

void print_error_three_strings(uint16_t error_number, const char *error_string, const char *error2_string, uint8_t error2_string_length, const char *error3_string)
{
    char tmp1[512];
    memcpy(tmp1, error2_string, error2_string_length);
    tmp1[error2_string_length] = '\0';
    char tmp[512];
    size_t line, column;
    get_location(&line, &column, scanner.last_error_cursor);
    sprintf(tmp, "***** Line %zu, Column %zu: [ERROR %u] %s %s %s\n", line, column, error_number, error_string, tmp1, error3_string);
    if (refalab_source_listing != NULL)
        fputs(tmp, refalab_source_listing);
    fputs(tmp, terminal);
    processing_error();
    return;
}

void print_error_string_symbol(uint16_t error_number, const char *error_string, const char symbol)
{
    char tmp[512];
    size_t line, column;
    get_location(&line, &column, scanner.last_error_cursor);
    sprintf(tmp, "***** Line %zu, Column %zu: [ERROR %u] %s %c\n", line, column, error_number, error_string, symbol);
    if (refalab_source_listing != NULL)
        fputs(tmp, refalab_source_listing);
    fputs(tmp, terminal);
    processing_error();
    return;
}

//----------  end of file print_errors.c  ---------- 
