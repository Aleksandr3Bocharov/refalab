// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-05-29
// https://github.com/Aleksandr3Bocharov/refalab

//------------ file -- D.H-------------------- 
//       Definitions for RefalAB debugger             
//-------------------------------------------- 

#ifndef D_H
#define D_H

#include "refalab.h"

extern void (*status_table_debugger)(T_STATUS_TABLE *);
extern void refal_debugger(T_STATUS_TABLE *status_table);

#endif

//-----------  end of file D.H ------------- 
