/*----------- file RFRUN2.c -----------------*/
/*      Refal-interpretator (part 2)         */
/*      Last edition date: 19.06.2024        */
/*-------------------------------------------*/
#include <stdio.h>
#include "refal.def"
#include "rfrun2.h"

#define NMBL sizeof(char)

struct wjs
{
    linkcb *jsb1;
    linkcb *jsb2;
    int jsnel;
    char *jsvpc;
};

struct ts
{
    linkcb *ts0;
    linkcb *ts1;
    linkcb *ts2;
};

struct spcs
{
    int spls;
    char *svpc;
};

static int letter(unsigned char s);
static int digit(char s);
static int not(int spcpls);

extern int spc(struct spcs *pspcsp, char *vpc, linkcb *b)
/* specifier interpreter */
{
    int spcwrk;         /* work variable */
    int spcpls;         /* positiveness feature of specifier element */
    struct spcs *spcsp; /* spcs-pointer */
    char *spcvpc;       /* virtual specifier counter */
    char spcopc;        /*specifier code */
    spcsp = pspcsp;
    move(LBLL, vpc + 1, spcvpc); /* spcvpc = L */
    spcpls = TRUE;
    goto SPCNXT;
/* return from specifier element if "YES" */
SPCRET:
    if (spcsp == pspcsp)
        return spcpls;
    spcsp--;
    spcwrk = spcpls;
    spcpls = spcsp->spls; /*getss (spcpls,spcvpc);*/
    spcvpc = spcsp->svpc;
    if (spcwrk == TRUE)
        goto SPCRET;
    spcvpc = spcvpc + LBLL;
/* return from specifier element if "NO" */
SPCNXT:
    spcopc = *spcvpc;
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
    move(LBLL, spcvpc, spcvpc);
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
    if (cmpr(SMBL, spcvpc, (char *)&(b->tag)) == 1)
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

static int letter(unsigned char s)
{
    if ((s >= 'A' && s <= 'Z') || /* A..Z     */
        (s >= 'a' && s <= 'z') || /* a..z     */
        (s > 127 && s < 176) ||   /* �..��..� */
        (s > 223 && s < 240))     /* �..�     */
        return 1;
    return 0;
}

static int digit(char s)
{
    if (s >= '0' && s <= '9')
        return 1;
    return 0;
}

static int not(int spcpls)
{
    if (spcpls == TRUE)
        return FALSE;
    return TRUE;
}

extern void link(linkcb *x, linkcb *y)
{
    x->next = y;
    y->prev = x;
}

extern void putjs(struct wjs *jsp, linkcb **ab1, linkcb **ab2, int *anel, char **avpc)
{
    jsp->jsb1 = *ab1;
    jsp->jsb2 = *ab2;
    jsp->jsnel = *anel;
    jsp->jsvpc = *avpc;
}

extern void getjs(struct wjs *jsp, linkcb **ab1, linkcb **ab2, int *anel, char **avpc)
{
    *ab1 = jsp->jsb1;
    *ab2 = jsp->jsb2;
    *anel = jsp->jsnel;
    *avpc = jsp->jsvpc;
}

extern void putts(struct ts *tsp, linkcb **ax, linkcb **ay, linkcb **az)
{
    tsp->ts0 = *ax;
    tsp->ts1 = *ay;
    tsp->ts2 = *az;
}

extern void getts(struct ts *tsp, linkcb **ax, linkcb **ay, linkcb **az)
{
    *ax = tsp->ts0;
    *ay = tsp->ts1;
    *az = tsp->ts2;
}

extern void move(int n, char *pf, char *pt)
{
    int i;
    for (i = 0; i < n; i++)
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

extern int cmpr(int n, char *p1, char *p2)
{
    int i;
    for (i = 1; i <= n; i++)
    {
        if (*p1 != *p2)
            return 0;
        p1++;
        p2++;
    }
    return 1;
}
/*------------ end of file RFRUN2.C ----------*/
