/*------------------file----CGOP.C--------------------*/
/*    generation of the assembly language operators   */
/*          Last edition date : 14.06.2024            */
/*----------------------------------------------------*/
#include "refal.def"
#include "cgop.h"
#include "cj.h"

struct linkti
{
    unsigned short int tagg;
    union
    {
        char infoc;
        /*      unsigned long int coden;*/
        char *codef;
    } infoo;
};

struct _TAG
{
    char b1;
    char b2;
};

extern void gopn(char k, char n)
{
    jbyte(k);
    jbyte(n);
}

extern void gopnm(char k, char n, char m)
{
    jbyte(k);
    jbyte(n);
    jbyte(m);
}

extern void gopl(char k, char *l)
{
    jbyte(k);
    j3addr((T_U *)l);
}

extern void gsymbol(struct linkti *code)
{
    struct _TAG *q;
    char *r;
    register int i;
    q = (struct _TAG *)&(code->tagg);
    jbyte(q->b1);
    jbyte(q->b2);
    if (code->tagg == 2)
    {
        j3addr((T_U *)code->infoo.codef);
        return;
        /*      jbyte( '\0' ); jbyte( '\0' );  return; - for no LARGE IBM_PC */
    };
    r = code->infoo.codef;
    if (code->tagg == 0)
    {
        jbyte(*r);
        jbyte(*(r + 1));
        jbyte('\0');
        jbyte('\0');
    }
    else
        for (i = 0; i < LBLL; i++)
            jbyte(*(r + i));
}

extern void gops(char k, struct linkti *code)
{
    jbyte(k);
    gsymbol(code);
}

extern void ghw(unsigned short int h)
{
    struct _TAG *po;
    po = (struct _TAG *)&h; /*  eg */
    jbyte(po->b1);
    jbyte(po->b2);
}
