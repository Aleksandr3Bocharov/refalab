// Copyright 2024 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2024-09-27
// https://github.com/Aleksandr3Bocharov/RefalAB

//-----------  file  --  RFINTF.H ------------------
//           C-interface functions
//--------------------------------------------------

#ifndef RFINTF_H
#define RFINTF_H

#include <stdbool.h>
#include <stdint.h>
#include "refal.def"

extern T_REFAL refal;

extern void rfabe(const char *amsg);
extern void rfcanc(const T_ST *ast);
extern char rfcnv(char cm);
extern void rfdel(T_LINKCB *p, T_LINKCB *q);
extern void rfexec(void (*func)(void));
extern void rfinit(void);
extern void rfpex(const char *pt, const T_LINKCB *pr, const T_LINKCB *pn);
extern void rfpexm(const char *pt, const T_LINKCB *pr, const T_LINKCB *pn);
extern void rftermm(void);
extern void rftpl(T_LINKCB *r, T_LINKCB *p, T_LINKCB *q);

extern bool lcopy(T_LINKCB *r, const T_LINKCB *p, const T_LINKCB *q);
extern bool lcre(T_ST *ast);
extern bool lexist(const T_ST *ast);
extern bool lincrm(void);
extern bool lins(T_LINKCB *p, size_t l);
extern bool slins(T_LINKCB *p, size_t k);
extern bool linskd(T_ST *ast, void (*f)(void));
extern T_LINKCB *lldupl(const T_LINKCB *p, const T_LINKCB *q, const T_LINKCB *u);
extern bool lrqlk(size_t l);

#endif
