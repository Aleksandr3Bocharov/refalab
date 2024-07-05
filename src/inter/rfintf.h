#ifndef RFINTF_H
#define RFINTF_H

#include "refal.def"

extern REFAL refal;

extern void rfabe(const char *amsg);
extern void rfcanc(const st *ast);
extern char rfcnv(char cm);
extern void rfdel(linkcb *p, linkcb *q);
extern void rfexec(const unsigned char *func);
extern void rfinit();
extern void rfpex(const char *pt, const linkcb *pr, const linkcb *pn);
extern void rfpexm(const char *pt, const linkcb *pr, const linkcb *pn);
extern void rftermm();
extern void rftpl(linkcb *r, linkcb *p, linkcb *q);

extern unsigned int lcopy(linkcb *r, const linkcb *p, const linkcb *q);
extern unsigned int lcre(st *ast);
extern unsigned int lexist(const st *ast);
extern unsigned int lincrm();
extern unsigned int lins(linkcb *p, unsigned int l);
extern unsigned int slins(linkcb *p, unsigned int k);
extern unsigned int linskd(st *ast, const unsigned char *f);
extern linkcb *lldupl(const linkcb *p, const linkcb *q, const linkcb *u);
extern unsigned int lrqlk(unsigned int l);

#endif