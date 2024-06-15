#ifndef CH_H
#define CH_H

extern void sextrn(char *idp, int lidp, char *ide, int lide);
extern void fndef(char *idp, int lid);
extern void sempty(char *idp, int lid);
extern void sentry(char *idp, int lidp, char *ide, int lide);
extern void spdef(char *idp, int lid);
extern void sswap(char *idp, int lid);
extern struct u *spref(char *idp, int lid, char d);
extern struct u *fnref(char *idp, int lid);
extern void sequ(char *id1, int lid1, char *id0, int lid0);

#endif