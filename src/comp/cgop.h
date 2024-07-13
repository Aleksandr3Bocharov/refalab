#ifndef CGOP_H
#define CGOP_H

typedef struct linkti
{
    unsigned short int tag;
    union
    {
        char infoc[2];
        unsigned long int coden;
        unsigned char *codef;
    } info;
} T_LINKTI;

extern void ghw(unsigned short int h);
extern void gopl(char k, const unsigned char *l);
extern void gopn(char k, char n);
extern void gopnm(char k, char n, char m);
extern void gops(char k, const T_LINKTI *code);
extern void gsymbol(const T_LINKTI *code);

#endif