// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-03-25
// https://github.com/Aleksandr3Bocharov/refalab

//-------------  file  --  CERR.C  --------------- 
//               Print error file                           
//------------------------------------------------ 

#include <stdio.h>
#include "refalab.h"
#include "cerr.h"
#include "refal.h"

void print_error_string(const char *error_string)
{
    processing_error();
    char tmp[256];
    sprintf(tmp, "***** %s\n", error_string);
    if (file_source_listing != NULL)
        fputs(tmp, file_source_listing);
    fputs(tmp, terminal);
    return;
}

void print_error_two_strings(const char *error_string, const char *error2_string, uint8_t error2_string_length)
{
    processing_error();
    char tmp1[256];
    for (uint8_t i = 0; i < error2_string_length; i++)
        tmp1[i] = *(error2_string + i);
    tmp1[error2_string_length] = '\0';
    char tmp[512];
    sprintf(tmp, "***** %s %s\n", error_string, tmp1);
    if (file_source_listing != NULL)
        fputs(tmp, file_source_listing);
    fputs(tmp, terminal);
    return;
}

void print_error_three_strings(const char *error_string, const char *error2_string, uint8_t error2_string_length, const char *error3_string)
{
    processing_error();
    char tmp1[256];
    for (uint8_t i = 0; i < error2_string_length; i++)
        tmp1[i] = *(error2_string + i);
    tmp1[error2_string_length] = '\0';
    char tmp[512];
    sprintf(tmp, "***** %s %s %s\n", error_string, tmp1, error3_string);
    if (file_source_listing != NULL)
        fputs(tmp, file_source_listing);
    fputs(tmp, terminal);
    return;
}

void print_error_string_symbol(const char *error_string, const char symbol)
{
    processing_error();
    char tmp[256];
    sprintf(tmp, "***** %s %c\n", error_string, symbol);
    if (file_source_listing != NULL)
        fputs(tmp, file_source_listing);
    fputs(tmp, terminal);
    return;
}

//--------  end  of  file  CERR.C  --------- 
