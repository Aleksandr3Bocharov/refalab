//------------------file----CGOP.C--------------------
//    generation of the assembly language operators
//          Last edition date : 11.07.2024
//----------------------------------------------------
#include <stdint.h>
#include "refal.def"
#include "cgop.h"
#include "cj.h"

typedef struct _TAG
{
    char b1;
    char b2;
} T_TAG;

void gopn(char k, char n)
{
    jbyte(k);
    jbyte(n);
    return;
}

void gopnm(char k, char n, char m)
{
    jbyte(k);
    jbyte(n);
    jbyte(m);
    return;
}

void gopl(char k, const uint8_t *l)
{
    jbyte(k);
    j3addr((T_U *)l);
    return;
}

void gsymbol(const T_LINKTI *code)
{
    const T_TAG *q = (T_TAG *)&(code->tag);
    jbyte(q->b1);
    jbyte(q->b2);
    if (code->tag == TAGF)
    {
        j3addr((T_U *)code->info.codef);
        return;
        //      jbyte( '\0' ); jbyte( '\0' );  return; - for no LARGE IBM_PC
    };
    const char *r = (char *)&(code->info.codef);
    if (code->tag == TAGO)
    {
        jbyte(*r);
        jbyte(*(r + 1));
        for (size_t i = 2; i < LBLL; i++)
            jbyte('\0');
        //jbyte('\0');
        //jbyte('\0');
    }
    else
        for (size_t i = 0; i < LBLL; i++)
            jbyte(*(r + i));
    return;
}

void gops(char k, const T_LINKTI *code)
{
    jbyte(k);
    gsymbol(code);
    return;
}

/*
void ghw(uint16_t h)
{
    const T_TAG *po = (T_TAG *)&h; //  eg
    jbyte(po->b1);
    jbyte(po->b2);
    return;
}
*/