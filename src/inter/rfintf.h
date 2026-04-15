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
extern void rfcanc(const T_STATUS_TABLE *ast);
extern void rfdel(T_LINKCB *p, T_LINKCB *q);
extern void rfexec(uint8_t *func);
extern void refal_get_args(int argc, char *argv[]);
extern T_LINKCB *rfgstr(char *str, size_t l, T_LINKCB *p);
extern void rfinit(void);
extern void rfpex(const char *pt, const T_LINKCB *pr, const T_LINKCB *pn, const bool nl);
extern void rfpexm(const char *pt, const T_LINKCB *pr, const T_LINKCB *pn, const bool nl);
extern void rfpexs(const char *pt, const T_LINKCB *pr, const T_LINKCB *pn, const bool nl);
extern bool rfreof(int c, FILE *f, T_LINKCB *p);
extern T_LINKCB *rfrstr(const char *str, T_LINKCB *p);
extern void rftermm(void);
extern void rftpl(T_LINKCB *r, T_LINKCB *p, T_LINKCB *q);

extern bool lcopy(T_LINKCB *r, const T_LINKCB *p, const T_LINKCB *q);
extern bool lcre(T_STATUS_TABLE *ast);
extern bool lexist(const T_STATUS_TABLE *ast);
extern bool more_free_memory(void);
extern bool insert_from_free_memory_list(T_LINKCB *where, size_t count);
extern bool slins(T_LINKCB *p, size_t k);
extern bool linskd(T_STATUS_TABLE *ast, uint8_t *f);
extern T_LINKCB *lldupl(const T_LINKCB *p, const T_LINKCB *q, const T_LINKCB *u);
extern bool check_count_free_memory_list(size_t count);

#endif
