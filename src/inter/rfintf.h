// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-04-14
// https://github.com/Aleksandr3Bocharov/refalab

//-----------  file  --  RFINTF.H ------------------
//           C-interface functions
//--------------------------------------------------

#ifndef RFINTF_H
#define RFINTF_H

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "refalab.h"

extern T_REFAL refal;

extern void refal_abort_end(const char *abort_message);
extern void delete_status_table(const T_STATUS_TABLE *status_table);
extern void insert_to_free_memory_list(T_LINKCB *before, T_LINKCB *after);
extern void refal_execute(uint8_t *refalab_function);
extern void refal_get_args(int argc, char *argv[]);
extern T_LINKCB *get_string_expression(char *string, size_t max_string_size, T_LINKCB *begin, const T_LINKCB *max_after);
extern void refal_initiate(void);
extern void print_expression(const char *begin_string, const T_LINKCB *before, const T_LINKCB *after, const bool new_line);
extern void print_expression_m(const char *begin_string, const T_LINKCB *before, const T_LINKCB *after, const bool new_line);
extern void print_expression_s(const char *begin_string, const T_LINKCB *before, const T_LINKCB *after, const bool new_line);
extern bool rfreof(int c, FILE *f, T_LINKCB *p);
extern T_LINKCB *rfrstr(const char *str, T_LINKCB *p);
extern void refal_terminate_memory(void);
extern void transplantation(T_LINKCB *where, T_LINKCB *before, T_LINKCB *after);

extern bool copy_expression(T_LINKCB *where, T_LINKCB *before, T_LINKCB *after);
extern bool create_status_table(T_STATUS_TABLE *status_table);
extern bool exist_status_table(const T_STATUS_TABLE *status_table);
extern bool more_free_memory(void);
extern bool insert_from_free_memory_list(T_LINKCB *where, size_t count);
extern bool extended_insert_from_free_memory_list(T_LINKCB *where, size_t count);
extern bool insert_view_k_function_dot(T_STATUS_TABLE *status_table, uint8_t *refalab_function);
extern T_LINKCB *find_duplicate(const T_LINKCB *before, const T_LINKCB *after, const T_LINKCB *before_duplicate);
extern bool check_count_free_memory_list(size_t count);

#endif
