#ifndef RFINTF_H
#define RFINTF_H

#include "refal.def"

extern REFAL refal;

extern void rfabe(char *amsg);
extern void rfcanc(st *ast);
extern char rfcnv(char cm);
extern void rfdel(linkcb *p, linkcb *q);
extern void rfexec(char *func);
extern void rfinit();
extern void rfpex(char *pt, linkcb *pr, linkcb *pn);
extern void rfpexm(char *pt, linkcb *pr, linkcb *pn);
extern void rftermm();
extern void rftpl(linkcb *r, linkcb *p, linkcb *q);

extern int lcopy(linkcb *r, linkcb *p, linkcb *q);
extern int lcre(st *ast);
extern int lexist(st *ast);
extern int lincrm();
extern int lins(linkcb *p, int l);
extern int slins(linkcb *p, int k);
extern int linskd(st *ast, char *f);
extern linkcb *lldupl(linkcb *p, linkcb *q, linkcb *u);
extern int lrqlk(int l);

#endif