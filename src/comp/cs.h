#ifndef CH_H
#define CH_H

extern struct i_lbl *genlbl();

extern void fndef(char *idp, unsigned int lid);
extern void spdef(char *idp, unsigned int lid);
extern struct u *fnref(char *idp, unsigned int lid);
extern struct u *spref(char *idp, unsigned int lid, char d);

extern void sempty(char *idp, unsigned int lid);
extern void sentry(char *idp, unsigned int lidp, char *ide, unsigned int lide);
extern void sequ(char *id1, unsigned int lid1, char *id0, unsigned int lid0);
extern void sextrn(char *idp, unsigned int lidp, char *ide, unsigned int lide);
extern void sswap(char *idp, unsigned int lid);

extern void s_init();
extern void s_term();
extern void s_end();

#endif