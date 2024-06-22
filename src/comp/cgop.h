#ifndef CGOP_H
#define CGOP_H

struct linkti;

extern void ghw(unsigned short int h);
extern void gopl(char k, char *l);
extern void gopn(char  k, char n);
extern void gopnm(char k , char n , char m);
extern void gops(char k, struct linkti *code);
extern void gsymbol(struct linkti *code);

#endif