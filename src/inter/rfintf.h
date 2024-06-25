#ifndef RFINTF_H
#define RFINTF_H

#include "refal.def"

extern REFAL refal;

extern void rfabe(char *amsg);
extern void rfcanc(st *ast);
extern char rfcnv(char cm);
extern void rfdel(linkcb *p, linkcb *q);
extern void rfexec(unsigned char *func);
extern void rfinit();
extern void rfpex(char *pt, linkcb *pr, linkcb *pn);
extern void rfpexm(char *pt, linkcb *pr, linkcb *pn);
extern void rftermm();
extern void rftpl(linkcb *r, linkcb *p, linkcb *q);

extern unsigned int lcopy(linkcb *r, linkcb *p, linkcb *q);
extern unsigned int lcre(st *ast);
extern unsigned int lexist(st *ast);
extern unsigned int lincrm();
extern unsigned int lins(linkcb *p, unsigned int l);
extern unsigned int slins(linkcb *p, unsigned int k);
extern unsigned int linskd(st *ast, unsigned char *f);
extern linkcb *lldupl(linkcb *p, linkcb *q, linkcb *u);
extern unsigned int lrqlk(unsigned int l);

#endif