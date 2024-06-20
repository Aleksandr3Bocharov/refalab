#ifndef CH_H
#define CH_H

extern struct i_lbl *genlbl();

extern void fndef(char *idp, int lid);
extern void spdef(char *idp, int lid);
extern struct u *fnref(char *idp, int lid);
extern struct u *spref(char *idp, int lid, char d);

extern void sempty(char *idp, int lid);
extern void sentry(char *idp, int lidp, char *ide, int lide);
extern void sequ(char *id1, int lid1, char *id0, int lid0);
extern void sextrn(char *idp, int lidp, char *ide, int lide);
extern void sswap(char *idp, int lid);

extern void s_init();
extern void s_term();
extern void s_end();

#endif