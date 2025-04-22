// Copyright 2024 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2025-04-23
// https://github.com/Aleksandr3Bocharov/RefalAB

//-------------  file  --  CERR.H  --------------- 
//               Print error file                           
//------------------------------------------------ 

#ifndef CERR_H
#define CERR_H

#include <stddef.h>

extern void pchosa(const char *s, char c);
extern void pchose(const char *s, const char *t, size_t lt);
extern void pchosh(const char *s);
extern void pchosj(const char *s, const char *sid, size_t lsid, const char *s1);

#endif
