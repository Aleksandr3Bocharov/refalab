#ifndef CGOP_H
#define CGOP_H

#include <stdint.h>

typedef struct linkti
{
    uint16_t tag;
    union
    {
        char infoc[2];
        unsigned long int coden;
        uint8_t *codef;
    } info;
} T_LINKTI;

extern void ghw(uint16_t h);
extern void gopl(char k, const uint8_t *l);
extern void gopn(char k, char n);
extern void gopnm(char k, char n, char m);
extern void gops(char k, const T_LINKTI *code);
extern void gsymbol(const T_LINKTI *code);

#endif