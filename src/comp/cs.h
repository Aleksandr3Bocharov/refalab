#ifndef CH_H
#define CH_H

#include "clu.h"

typedef struct i_lbl
{
    union
    {
        unsigned int infoln;
        struct i_lbl *infolp;
    } infol;
    char model;
} T_I_LBL;

extern T_I_LBL *genlbl();

extern void fndef(const char *idp, size_t lid);
extern void spdef(const char *idp, size_t lid);
extern T_U *fnref(const char *idp, unsigned int lid);
extern T_U *spref(const char *idp, size_t lid, char d);

extern void sempty(const char *idp, size_t lid);
extern void sentry(const char *idp, unsigned int lidp, const char *ide, size_t lide);
extern void sequ(const char *id1, unsigned int lid1, const char *id0, unsigned int lid0);
extern void sextrn(const char *idp, size_t lidp, const char *ide, size_t lide);
extern void sswap(const char *idp, size_t lid);

extern void s_init();
extern void s_term();
extern void s_end();

#endif