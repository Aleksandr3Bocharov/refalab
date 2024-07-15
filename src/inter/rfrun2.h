#ifndef RFRUN2_H
#define RFRUN2_H

#include <stdbool.h>
#include <stdint.h>
#include "refal.def"

typedef struct wjs
{ // jump stack structure  
    T_LINKCB *jsb1;
    T_LINKCB *jsb2;
    unsigned int jsnel;
    uint8_t *jsvpc;
} T_WJS;

typedef struct ts
{ // translation stack structure 
    T_LINKCB *ts0;
    T_LINKCB *ts1;
    T_LINKCB *ts2;
} T_TS;

typedef struct spcs
{
    bool spls;
    uint8_t *svpc;
} T_SPCS;

extern bool cmpr(unsigned int n, const uint8_t *p1, const uint8_t *p2);
extern void link(T_LINKCB *x, T_LINKCB *y);
extern void move(unsigned int n, const uint8_t *pf, uint8_t *pt);
extern bool spc(T_SPCS *pspcsp, const uint8_t *vpc, const T_LINKCB *b);

extern void getjs(const T_WJS *jsp, T_LINKCB **ab1, T_LINKCB **ab2, unsigned int *anel, uint8_t **avpc);
extern void putjs(T_WJS *jsp, T_LINKCB **ab1, T_LINKCB **ab2, const unsigned int *anel, uint8_t **avpc);
extern void getts(const T_TS *tsp, T_LINKCB **ax, T_LINKCB **ay, T_LINKCB **az);
extern void putts(T_TS *tsp, T_LINKCB **ax, T_LINKCB **ay, T_LINKCB **az);

#endif