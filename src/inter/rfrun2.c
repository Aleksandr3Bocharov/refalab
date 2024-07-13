//----------- file RFRUN2.c ----------------- 
//      Refal-interpretator (part 2)          
//      Last edition date: 11.07.2024         
//------------------------------------------- 
#include <stdio.h>
#include <stdbool.h>
#include "refal.def"
#include "rfrun2.h"

#define NMBL sizeof(char)

static bool letter(char s);
static bool digit(char s);

bool spc(T_SPCS *pspcsp, const unsigned char *vpc, const T_LINKCB *b)
// specifier interpreter  
{
    // spcs-pointer  
    T_SPCS *spcsp = pspcsp;
    unsigned char *spcvpc; // virtual specifier counter  
    move(LBLL, vpc + 1, (unsigned char *)&spcvpc); // spcvpc = L  
    // positiveness feature of specifier element  
    bool spcpls = true;
    goto SPCNXT;
// return from specifier element if "YES"  
SPCRET:
    if (spcsp == pspcsp)
        return spcpls;
    spcsp--;
    // work variable  
    const bool spcwrk = spcpls;
    spcpls = spcsp->spls; //getss (spcpls,spcvpc); 
    spcvpc = spcsp->svpc;
    if (spcwrk)
        goto SPCRET;
    spcvpc = spcvpc + LBLL;
// return from specifier element if "NO"  
SPCNXT:
    //specifier code  
    const unsigned char spcopc = *spcvpc;
    spcvpc = spcvpc + NMBL;
    // switch    
    // SPCOP     
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
    // SPCCLL(L);  
SPCCLL:
    spcsp->spls = spcpls;
    spcsp->svpc = spcvpc;
    move(LBLL, spcvpc, (unsigned char *)&spcvpc);
    spcsp++;
    spcpls = true;
    goto SPCNXT;
SPCW:
    goto SPCRET;
SPCNG:
    spcpls = !spcpls;
    goto SPCNXT;
SPCNGW:
    spcpls = !spcpls;
    goto SPCRET;
SPCSC:
    if (cmpr(SMBL, spcvpc, (unsigned char *)&(b->tag)))
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
} //             end      spc           

static bool letter(char s)
{
    if ((s >= 'A' && s <= 'Z') || // A..Z      
        (s >= 'a' && s <= 'z') || // a..z      
        (s > 127 && s < 176) ||   // �..��..�  
        (s > 223 && s < 240))     // �..�      
        return true;
    return false;
}

static bool digit(char s)
{
    if (s >= '0' && s <= '9')
        return true;
    return false;
}

void link(T_LINKCB *x, T_LINKCB *y)
{
    x->next = y;
    y->prev = x;
}

void putjs(T_WJS *jsp, T_LINKCB **ab1, T_LINKCB **ab2, const unsigned int *anel, unsigned char **avpc)
{
    jsp->jsb1 = *ab1;
    jsp->jsb2 = *ab2;
    jsp->jsnel = *anel;
    jsp->jsvpc = *avpc;
}

void getjs(const T_WJS *jsp, T_LINKCB **ab1, T_LINKCB **ab2, unsigned int *anel, unsigned char **avpc)
{
    *ab1 = jsp->jsb1;
    *ab2 = jsp->jsb2;
    *anel = jsp->jsnel;
    *avpc = jsp->jsvpc;
}

void putts(T_TS *tsp, T_LINKCB **ax, T_LINKCB **ay, T_LINKCB **az)
{
    tsp->ts0 = *ax;
    tsp->ts1 = *ay;
    tsp->ts2 = *az;
}

void getts(const T_TS *tsp, T_LINKCB **ax, T_LINKCB **ay, T_LINKCB **az)
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

// BLF 28.07.2004  - in case of macrodigit - shift -> 16  
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
}*/
 

bool cmpr(unsigned int n, const unsigned char *p1, const unsigned char *p2)
{
    for (unsigned int i = 1; i <= n; i++)
    {
        if (*p1 != *p2)
            return false;
        p1++;
        p2++;
    }
    return true;
}
//------------ end of file RFRUN2.C ---------- 
