#ifndef CJ_H
#define CJ_H

#include "clu.h"

typedef struct BU_ BU;

extern BU sysl;

extern void j3addr(T_U *pp);
extern void jbyte(char bb);
extern void jend();
extern void jendo();
extern void jentry(struct u *pp, char *ee, int ll);
extern void jequ(struct u *pp, struct u *qq);
extern void jextrn(struct u *pp, char *ee, int ll);
extern void jlabel(struct u *pp);
extern void jstart(char *ee, int ll);
extern unsigned int jwhere();

extern void sfclose(BU *b);
extern void sfop_w(char *s, BU *b);

#endif