// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-03-25
// https://github.com/Aleksandr3Bocharov/refalab

//-------------  file  --  CERR.H  --------------- 
//               Print error file                           
//------------------------------------------------ 

#ifndef CERR_H
#define CERR_H

#include <stddef.h>

extern void print_error_string(const char *error_string);
extern void print_error_two_strings(const char *error_string, const char *error2_string, uint8_t error2_string_length);
extern void print_error_three_strings(const char *error_string, const char *error2_string, uint8_t error2_string_length, const char *error3_string);
extern void print_error_string_symbol(const char *error_string, char symbol);

#endif
