// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-06-11
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file print_errors.h  ---------- 
//               Print error file                           
//------------------------------------------- 

#ifndef PRINT_ERRORS_H
#define PRINT_ERRORS_H

#include <stddef.h>
#include <stdint.h>

extern void print_error_string(const char *error_string);
extern void print_error_two_strings(const char *error_string, const char *error2_string, uint8_t error2_string_length);
extern void print_error_three_strings(const char *error_string, const char *error2_string, uint8_t error2_string_length, const char *error3_string);
extern void print_error_string_symbol(const char *error_string, char symbol);

#endif
