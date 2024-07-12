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

extern bool lcopy(linkcb *r, const linkcb *p, const linkcb *q);
extern bool lcre(st *ast);
extern bool lexist(const st *ast);
extern bool lincrm();
extern bool lins(linkcb *p, unsigned int l);
extern bool slins(linkcb *p, unsigned int k);
extern bool linskd(st *ast, const unsigned char *f);
extern linkcb *lldupl(const linkcb *p, const linkcb *q, const linkcb *u);
extern bool lrqlk(unsigned int l);

#endif