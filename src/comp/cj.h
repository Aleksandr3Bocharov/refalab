// Copyright 2024 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2024-09-27
// https://github.com/Aleksandr3Bocharov/RefalAB

//-----------------  file  --  cj.h  -------------------
//             generator of assembler text
//------------------------------------------------------

#ifndef CJ_H
#define CJ_H

#include <stdio.h>
#include "clu.h"

extern void j3addr(T_U *pp);
extern void jbyte(char bb);
extern void jend(void);
extern void jentry(T_U *pp, const char *ee, size_t ll);
extern void jequ(T_U *pp, T_U *qq);
extern void jextrn(T_U *pp, const char *ee, size_t ll);
extern void jlabel(T_U *pp);
extern void jstart();
extern size_t jwhere(void);

#endif
