#ifndef CJ_H
#define CJ_H

typedef struct u T_U;

extern void jbyte(char bb);
extern void j3addr(T_U *pp);
extern void jlabel(struct u *pp);
extern void jequ(struct u *pp, struct u *qq);
extern void jentry(struct u *pp, char *ee, int ll);
extern void jextrn(struct u *pp, char *ee, int ll);

#endif