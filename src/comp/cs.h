// Copyright 2024 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2024-10-19
// https://github.com/Aleksandr3Bocharov/RefalAB

//-------------  file  --  CS.H  ---------------
//         Work with identifier table
//----------------------------------------------

#ifndef CH_H
#define CH_H

#include <stddef.h>
#include "clu.h"

typedef struct i_lbl
{
    union
    {
        size_t infoln;
        struct i_lbl *infolp;
    } infol;
    char model;
} T_I_LBL;

extern T_I_LBL *genlbl(void);

extern void fndef(const char *idp, size_t lid);
extern void spdef(const char *idp, size_t lid);
extern T_U *fnref(const char *idp, size_t lid);
extern T_U *spref(const char *idp, size_t lid, char d);

extern void sempty(const char *idp, size_t lid);
extern void sentry(const char *idp, size_t lidp, const char *ide, size_t lide);
extern void sequ(const char *id1, size_t lid1, const char *id0, size_t lid0);
extern void sextrn(const char *idp, size_t lidp, const char *ide, size_t lide);
extern void sswap(const char *idp, size_t lid);

extern void s_init(void);
extern void s_term(void);
extern void s_end(void);

#endif
