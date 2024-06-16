#ifndef CJ_H
#define CJ_H

typedef struct u T_U;
typedef struct BU_ BU;

extern void jbyte(char bb);
extern void j3addr(T_U *pp);
extern void jlabel(struct u *pp);
extern void jequ(struct u *pp, struct u *qq);
extern void jentry(struct u *pp, char *ee, int ll);
extern void jextrn(struct u *pp, char *ee, int ll);
extern void jstart(char *ee, int ll);
extern void sfop_w(char *s, BU *b);
extern void sfclose(BU *b);
extern unsigned int jwhere();
extern void jend();
extern void jendo();

#endif