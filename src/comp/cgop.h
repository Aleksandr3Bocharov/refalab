#ifndef CGOP_H
#define CGOP_H

struct linkti
{
    unsigned short int tag;
    union
    {
        char infoc[2];
        unsigned long int coden;
        unsigned char *codef;
    } info;
};

extern void ghw(unsigned short int h);
extern void gopl(char k, const unsigned char *l);
extern void gopn(char k, char n);
extern void gopnm(char k, char n, char m);
extern void gops(char k, const struct linkti *code);
extern void gsymbol(const struct linkti *code);

#endif