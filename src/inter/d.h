// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-04-20
// https://github.com/Aleksandr3Bocharov/refalab

//------------ file -- D.H-------------------- 
//       Definitions for RefalAB debugger             
//-------------------------------------------- 

#ifndef D_H
#define D_H

#include "refalab.h"

extern void (*status_table_debugger)(T_STATUS_TABLE *);
extern void rfdbg(T_STATUS_TABLE *s_st);

#endif

//-----------  end of file D.H ------------- 
