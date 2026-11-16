// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-11-16
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file interpreter.h  ----------
//      Refal-interpretator
//-----------------------------------------

#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "refalab.h"

extern void refal_run(T_STATUS_TABLE *status_table);
#if defined mdebug
extern void debug_print_expression(const char *begin_string, const T_LINKCB *before, const T_LINKCB *after);
#endif

#endif
