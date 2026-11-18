// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-11-17
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file print_errors.c  ----------
//               Print error file
//-------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "refalab.h"
#include "print_errors.h"
#include "compiler.h"

// Fatal error: prints the message and terminates the compiler with exit code 1.
void error_no_memory(void)
{
    fputs("\nERROR: no memory!!!\n", stdout);
    exit(1);
    return;
}

// All print_error_* functions format one diagnostic line
// "***** Line L, Column C: [ERROR nnn] text" for the position of the last scanned
// error, write it to the source listing (when one is open) and to the terminal,
// and then register the error through processing_error().
// Nothing is printed while listing is suppressed.
// This variant prints the message only.
void print_error_string(uint16_t error_number, const char *error_string)
{
    if (!suppress_listing)
    {
        char tmp[512];
        size_t line, column;
        get_location(&line, &column, scanner.last_error_cursor);
        sprintf(tmp, "***** Line %5zu, Column %5zu: [ERROR %.3u] %s\n", line, column, error_number, error_string);
        if (refalab_source_listing != NULL)
            fputs(tmp, refalab_source_listing);
        fputs(tmp, terminal);
        processing_error();
    }
    return;
}

// Message followed by a counted string (an identifier, not NUL-terminated).
void print_error_two_strings(uint16_t error_number, const char *error_string, const char *error2_string, uint8_t error2_string_length)
{
    if (!suppress_listing)
    {
        // Copy of the counted string made NUL-terminated for sprintf.
        char tmp1[512];
        memcpy(tmp1, error2_string, error2_string_length);
        tmp1[error2_string_length] = '\0';
        char tmp[512];
        size_t line, column;
        get_location(&line, &column, scanner.last_error_cursor);
        sprintf(tmp, "***** Line %5zu, Column %5zu: [ERROR %.3u] %s %s\n", line, column, error_number, error_string, tmp1);
        if (refalab_source_listing != NULL)
            fputs(tmp, refalab_source_listing);
        fputs(tmp, terminal);
        processing_error();
    }
    return;
}

// Message, a counted string (an identifier) and a trailing message.
void print_error_three_strings(uint16_t error_number, const char *error_string, const char *error2_string, uint8_t error2_string_length, const char *error3_string)
{
    if (!suppress_listing)
    {
        char tmp1[512];
        memcpy(tmp1, error2_string, error2_string_length);
        tmp1[error2_string_length] = '\0';
        char tmp[512];
        size_t line, column;
        get_location(&line, &column, scanner.last_error_cursor);
        sprintf(tmp, "***** Line %5zu, Column %5zu: [ERROR %.3u] %s %s %s\n", line, column, error_number, error_string, tmp1, error3_string);
        if (refalab_source_listing != NULL)
            fputs(tmp, refalab_source_listing);
        fputs(tmp, terminal);
        processing_error();
    }
    return;
}

// Message followed by a single character.
void print_error_string_symbol(uint16_t error_number, const char *error_string, const char symbol)
{
    if (!suppress_listing)
    {
        char tmp[512];
        size_t line, column;
        get_location(&line, &column, scanner.last_error_cursor);
        sprintf(tmp, "***** Line %5zu, Column %5zu: [ERROR %.3u] %s %c\n", line, column, error_number, error_string, symbol);
        if (refalab_source_listing != NULL)
            fputs(tmp, refalab_source_listing);
        fputs(tmp, terminal);
        processing_error();
    }
    return;
}

//----------  end of file print_errors.c  ----------
