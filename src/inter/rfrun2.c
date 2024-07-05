/*----------- file RFRUN2.c -----------------*/
/*      Refal-interpretator (part 2)         */
/*      Last edition date: 19.06.2024        */
/*-------------------------------------------*/
#include <stdio.h>
#include "refal.def"
#include "rfrun2.h"

#define NMBL sizeof(char)

static unsigned int letter(char s);
static unsigned int digit(char s);
static unsigned int not(unsigned int spcpls);

unsigned int spc(T_SPCS *pspcsp, const unsigned char *vpc, const linkcb *b)
/* specifier interpreter */
{
    /* spcs-pointer */
    T_SPCS *spcsp = pspcsp;
    unsigned char *spcvpc; /* virtual specifier counter */
    move(LBLL, vpc + 1, (unsigned char *)&spcvpc); /* spcvpc = L */
    /* positiveness feature of specifier element */
    unsigned int spcpls = TRUE;
    goto SPCNXT;
/* return from specifier element if "YES" */
SPCRET:
    if (spcsp == pspcsp)
        return spcpls;
    spcsp--;
    /* work variable */
    const unsigned int spcwrk = spcpls;
    spcpls = spcsp->spls; /*getss (spcpls,spcvpc);*/
    spcvpc = spcsp->svpc;
    if (spcwrk == TRUE)
        goto SPCRET;
    spcvpc = spcvpc + LBLL;
/* return from specifier element if "NO" */
SPCNXT:
    /*specifier code */
    const unsigned char spcopc = *spcvpc;
    spcvpc = spcvpc + NMBL;
    /* switch   */
    /* SPCOP    */
    switch (spcopc)
    {
    case 0000:
        goto SPCCLL;
    case 0001:
        goto SPCW;
    case 0002:
        goto SPCNG;
    case 0003:
        goto SPCNGW;
    case 0004:
        goto SPCSC;
    case 0005:
        goto SPCS;
    case 0006:
        goto SPCB;
    case 0007:
        goto SPCF;
    case 0010:
        goto SPCN;
    case 0011:
        goto SPCR;
    case 0012:
        goto SPCO;
    case 0013:
        goto SPCD;
    case 0014:
        goto SPCL;
    }
    /* SPCCLL(L); */
SPCCLL:
    spcsp->spls = spcpls;
    spcsp->svpc = spcvpc;
    move(LBLL, spcvpc, (unsigned char *)&spcvpc);
    spcsp++;
    spcpls = TRUE;
    goto SPCNXT;
SPCW:
    goto SPCRET;
SPCNG:
    spcpls = not(spcpls);
    goto SPCNXT;
SPCNGW:
    spcpls = not(spcpls);
    goto SPCRET;
SPCSC:
    if (cmpr(SMBL, spcvpc, (unsigned char *)&(b->tag)) == 1)
        goto SPCRET;
    spcvpc = spcvpc + SMBL;
    goto SPCNXT;
SPCS:
    if ((b->tag & 0001) == 0)
        goto SPCRET;
    goto SPCNXT;
SPCB:
    if ((b->tag & 0001) != 0)
        goto SPCRET;
    goto SPCNXT;
SPCF:
    if (b->tag == TAGF)
        goto SPCRET;
    goto SPCNXT;
SPCN:
    if (b->tag == TAGN)
        goto SPCRET;
    goto SPCNXT;
SPCR:
    if (b->tag == TAGR)
        goto SPCRET;
    goto SPCNXT;
SPCO:
    if (b->tag == TAGO)
        goto SPCRET;
    goto SPCNXT;
SPCD:
    if (b->tag != TAGO)
        goto SPCNXT;
    if (digit(b->info.infoc) == 1)
        goto SPCRET;
    goto SPCNXT;
SPCL:
    if (b->tag != TAGO)
        goto SPCNXT;
    if (letter(b->info.infoc) == 1)
        goto SPCRET;
    goto SPCNXT;
} /*             end      spc          */

static unsigned int letter(char s)
{
    if ((s >= 'A' && s <= 'Z') || /* A..Z     */
        (s >= 'a' && s <= 'z') || /* a..z     */
        (s > 127 && s < 176) ||   /* �..��..� */
        (s > 223 && s < 240))     /* �..�     */
        return TRUE;
    return FALSE;
}

static unsigned int digit(char s)
{
    if (s >= '0' && s <= '9')
        return TRUE;
    return FALSE;
}

static unsigned int not(unsigned int spcpls)
{
    if (spcpls == TRUE)
        return FALSE;
    return TRUE;
}

void link(linkcb *x, linkcb *y)
{
    x->next = y;
    y->prev = x;
}

void putjs(T_WJS *jsp, linkcb **ab1, linkcb **ab2, const unsigned int *anel, unsigned char **avpc)
{
    jsp->jsb1 = *ab1;
    jsp->jsb2 = *ab2;
    jsp->jsnel = *anel;
    jsp->jsvpc = *avpc;
}

void getjs(const T_WJS *jsp, linkcb **ab1, linkcb **ab2, unsigned int *anel, unsigned char **avpc)
{
    *ab1 = jsp->jsb1;
    *ab2 = jsp->jsb2;
    *anel = jsp->jsnel;
    *avpc = jsp->jsvpc;
}

void putts(T_TS *tsp, linkcb **ax, linkcb **ay, linkcb **az)
{
    tsp->ts0 = *ax;
    tsp->ts1 = *ay;
    tsp->ts2 = *az;
}

void getts(const T_TS *tsp, linkcb **ax, linkcb **ay, linkcb **az)
{
    *ax = tsp->ts0;
    *ay = tsp->ts1;
    *az = tsp->ts2;
}

void move(unsigned int n, const unsigned char *pf, unsigned char *pt)
{
    for (unsigned int i = 0; i < n; i++)
    {
        *pt = *pf;
        pt++;
        pf++;
    }
}

/* BLF 28.07.2004  - in case of macrodigit - shift -> 16 */
/* void move(n,pf,pt) int n; char *pf,*pt; {
int i;
   if ( *pf == TAGN ) {
      / two bytes without changes /
      for ( i=0;i<2;i++ ){
         *pt = *pf; pt++; pf++;
      }
      / body of macrodigit shift -> 16 /
      for ( i=2;i<n;i++ ){
         *(pt+2) = *pf; pt++; pf++;
      }
   }
   else {
      for ( i=0;i<n;i++ ){
         *pt = *pf; pt++; pf++;
      }
   }
}
*/

unsigned int cmpr(unsigned int n, const unsigned char *p1, const unsigned char *p2)
{
    for (unsigned int i = 1; i <= n; i++)
    {
        if (*p1 != *p2)
            return FALSE;
        p1++;
        p2++;
    }
    return TRUE;
}
/*------------ end of file RFRUN2.C ----------*/
