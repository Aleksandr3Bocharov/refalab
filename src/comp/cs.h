// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-04-07
// https://github.com/Aleksandr3Bocharov/refalab

//-------------  file  --  CS.H  ---------------
//         Work with identifier table
//----------------------------------------------

#ifndef CH_H
#define CH_H

#include <stddef.h>
#include "clu.h"

typedef struct info_label
{
    void *info;
    uint8_t mode;
} T_INFO_LABEL;

extern T_INFO_LABEL *generate_info_label(void);

extern void fndef(const char *idp, size_t lid);
extern void spdef(const char *idp, size_t lid);
extern T_U *fnref(const char *idp, size_t lid);
extern T_U *spref(const char *idp, size_t lid, char d);

extern void sempty(const char *idp, uint8_t lid);
extern void sentry(const char *idp, uint8_t lidp, const char *ide, uint8_t lide);
extern void sequ(const char *id1, size_t lid1, const char *id0, size_t lid0);
extern void sextrn(const char *idp, uint8_t lidp, const char *ide, uint8_t lide);
extern void sswap(const char *idp, uint8_t lid);

extern void s_init(void);
extern void s_term(void);
extern void s_end(void);

#endif
