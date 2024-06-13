#ifndef CGOP_H
#define CGOP_H

struct linkti;

extern void gops(char k, struct linkti *code);
extern void gopn(char  k, char n);
extern void gopnm(char k , char n , char m);
extern void gopl(char k, char *l);
extern void ghw(short h);
extern void gsymbol(struct linkti *code);

#endif