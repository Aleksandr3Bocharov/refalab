// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-11-17
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file debugger_interface.h  ----------
//       RefalAB debugger functions
//-------------------------------------------------

#ifndef DEBUGGER_INTERFACE_H
#define DEBUGGER_INTERFACE_H

#include "refalab.h"

// Hook of the debugger: it is called by the function Try instead of running the interpreter
// freely when a debugger is linked into the program (NULL otherwise).
extern void (*status_table_debugger)(T_STATUS_TABLE *);
// The debugger itself: runs the program under the control of the user.
extern void refal_debugger(T_STATUS_TABLE *status_table);

#endif
