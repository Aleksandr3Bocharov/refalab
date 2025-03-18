// Copyright 2025 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2025-03-18
// https://github.com/Aleksandr3Bocharov/RefalAB

//-------------  file  --  CERR.C  --------------- 
//               Print error file                           
//------------------------------------------------ 

#include <stdio.h>
#include "refalab.h"
#include "cerr.h"
#include "refal.h"

void pchosh(const char *s)
{
    oshibka();
    char tmp[255];
    sprintf(tmp, "***** %s\n", s);
    if (sysprint != NULL)
        fputs(tmp, sysprint);
    fputs(tmp, systerm);
    return;
}

void pchosj(const char *s, const char *sid, size_t lsid, const char *s1)
{
    oshibka();
    char tmp1[255];
    for (size_t i = 0; i < lsid; i++)
        tmp1[i] = *(sid + i);
    tmp1[lsid] = '\0';
    char tmp[512];
    sprintf(tmp, "***** %s %s %s\n", s, tmp1, s1);
    if (sysprint != NULL)
        fputs(tmp, sysprint);
    fputs(tmp, systerm);
    return;
}

void pchosa(const char *s, const char c)
{
    oshibka();
    char tmp[255];
    sprintf(tmp, "***** %s %c\n", s, c);
    if (sysprint != NULL)
        fputs(tmp, sysprint);
    fputs(tmp, systerm);
    return;
}

//--------  end  of  file  CERR.C  --------- 
