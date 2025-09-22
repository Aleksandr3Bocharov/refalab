// Copyright 2025 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2025-09-21
// https://github.com/Aleksandr3Bocharov/RefalAB

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

#define MAX_STOP 0x7FFFFFFF

extern T_REFAL refal;

extern void rfabe(const char *amsg);
extern void rfcanc(const T_ST *ast);
extern void rfdel(T_LINKCB *p, T_LINKCB *q);
extern void rfexec(uint8_t *func);
extern void rfinit(void);
extern void rfpex(const char *pt, const T_LINKCB *pr, const T_LINKCB *pn, const bool nl);
extern void rfpexm(const char *pt, const T_LINKCB *pr, const T_LINKCB *pn, const bool nl);
extern void rfpexs(const char *pt, const T_LINKCB *pr, const T_LINKCB *pn, const bool nl);
extern void rfrbool(bool b, T_LINKCB *p);
extern bool rfreof(int c, FILE *f, T_LINKCB *p);
extern void rfrstr(const char *str);
extern void rftermm(void);
extern void rftpl(T_LINKCB *r, T_LINKCB *p, T_LINKCB *q);

extern bool lcopy(T_LINKCB *r, const T_LINKCB *p, const T_LINKCB *q);
extern bool lcre(T_ST *ast);
extern bool lexist(const T_ST *ast);
extern bool lincrm(void);
extern bool lins(T_LINKCB *p, size_t l);
extern bool slins(T_LINKCB *p, size_t k);
extern bool linskd(T_ST *ast, uint8_t *f);
extern T_LINKCB *lldupl(const T_LINKCB *p, const T_LINKCB *q, const T_LINKCB *u);
extern bool lrqlk(size_t l);

#endif
