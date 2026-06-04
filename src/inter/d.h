// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-06-05
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file debugger_interface.h  ----------
//       RefalAB debugger functions
//-------------------------------------------------

#ifndef DEBUGGER_INTERFACE_H
#define DEBUGGER_INTERFACE_H

#include "refalab.h"

extern void (*status_table_debugger)(T_STATUS_TABLE *);
extern void refal_debugger(T_STATUS_TABLE *status_table);

#endif
