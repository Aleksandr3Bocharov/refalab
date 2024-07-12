#ifndef RFRUN2_H
#define RFRUN2_H

#include "refal.def"

struct wjs
{ // jump stack structure  
    linkcb *jsb1;
    linkcb *jsb2;
    unsigned int jsnel;
    unsigned char *jsvpc;
};
typedef struct wjs T_WJS;

struct ts
{ // translation stack structure 
    linkcb *ts0;
    linkcb *ts1;
    linkcb *ts2;
};
typedef struct ts T_TS;

struct spcs
{
    unsigned int spls;
    unsigned char *svpc;
};
typedef struct spcs T_SPCS;

extern unsigned int cmpr(unsigned int n, const unsigned char *p1, const unsigned char *p2);
extern void link(linkcb *x, linkcb *y);
extern void move(unsigned int n, const unsigned char *pf, unsigned char *pt);
extern unsigned int spc(T_SPCS *pspcsp, const unsigned char *vpc, const linkcb *b);

extern void getjs(const T_WJS *jsp, linkcb **ab1, linkcb **ab2, unsigned int *anel, unsigned char **avpc);
extern void putjs(T_WJS *jsp, linkcb **ab1, linkcb **ab2, const unsigned int *anel, unsigned char **avpc);
extern void getts(const T_TS *tsp, linkcb **ax, linkcb **ay, linkcb **az);
extern void putts(T_TS *tsp, linkcb **ax, linkcb **ay, linkcb **az);

#endif