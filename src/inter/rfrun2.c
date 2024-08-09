//----------- file RFRUN2.c -----------------
//      Refal-interpretator (part 2)
//      Last edition date: 11.07.2024
//-------------------------------------------
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "refal.def"
#include "rfrun2.h"

#define NMBL sizeof(char)

typedef enum sp_states
{
    SPCRET,
    SPCNXT,
    SPCCLL,
    SPCW,
    SPCNG,
    SPCNGW,
    SPCSC,
    SPCS,
    SPCB,
    SPCF,
    SPCN,
    SPCR,
    SPCD,
    SPCO,
    SPCL
} T_SP_STATES;

static bool letter(char s);
static bool digit(char s);

bool spc(T_SPCS *pspcsp, const uint8_t *vpc, const T_LINKCB *b)
// specifier interpreter
{
    // spcs-pointer
    T_SPCS *spcsp = pspcsp;
    uint8_t *spcvpc;                   // virtual specifier counter
    memcpy(&spcvpc, vpc + NMBL, LBLL); // spcvpc = L
    // positiveness feature of specifier element
    bool spcpls = true;
    T_SP_STATES sp_state = SPCNXT;
    while (true)
        switch (sp_state)
        {
        // return from specifier element if "YES"
        case SPCRET:
            if (spcsp == pspcsp)
                return spcpls;
            spcsp--;
            // work variable
            const bool spcwrk = spcpls;
            spcpls = spcsp->spls; // getss (spcpls,spcvpc);
            spcvpc = spcsp->svpc;
            if (spcwrk)
                break;
            spcvpc = spcvpc + LBLL;
            sp_state = SPCNXT;
            break;
        // return from specifier element if "NO"
        case SPCNXT:
            // specifier code
            const uint8_t spcopc = *spcvpc;
            spcvpc = spcvpc + NMBL;
            // switch
            // SPCOP
            switch (spcopc)
            {
            case 0000:
                sp_state = SPCCLL;
                break;
            case 0001:
                sp_state = SPCW;
                break;
            case 0002:
                sp_state = SPCNG;
                break;
            case 0003:
                sp_state = SPCNGW;
                break;
            case 0004:
                sp_state = SPCSC;
                break;
            case 0005:
                sp_state = SPCS;
                break;
            case 0006:
                sp_state = SPCB;
                break;
            case 0007:
                sp_state = SPCF;
                break;
            case 0010:
                sp_state = SPCN;
                break;
            case 0011:
                sp_state = SPCR;
                break;
            case 0012:
                sp_state = SPCO;
                break;
            case 0013:
                sp_state = SPCD;
                break;
            case 0014:
                sp_state = SPCL;
            }
            break;
            // SPCCLL(L);
        case SPCCLL:
            spcsp->spls = spcpls;
            spcsp->svpc = spcvpc;
            memcpy(&spcvpc, spcvpc, LBLL);
            spcsp++;
            spcpls = true;
            sp_state = SPCNXT;
            break;
        case SPCW:
            sp_state = SPCRET;
            break;
        case SPCNG:
            spcpls = !spcpls;
            sp_state = SPCNXT;
            break;
        case SPCNGW:
            spcpls = !spcpls;
            sp_state = SPCRET;
            break;
        case SPCSC:
            if (memcmp(spcvpc, &(b->tag), SMBL) == 0)
            {
                sp_state = SPCRET;
                break;
            }
            spcvpc = spcvpc + SMBL;
            sp_state = SPCNXT;
            break;
        case SPCS:
            if ((b->tag & 0001) == 0)
            {
                sp_state = SPCRET;
                break;
            }
            sp_state = SPCNXT;
            break;
        case SPCB:
            if ((b->tag & 0001) != 0)
            {
                sp_state = SPCRET;
                break;
            }
            sp_state = SPCNXT;
            break;
        case SPCF:
            if (b->tag == TAGF)
            {
                sp_state = SPCRET;
                break;
            }
            sp_state = SPCNXT;
            break;
        case SPCN:
            if (b->tag == TAGN)
            {
                sp_state = SPCRET;
                break;
            }
            sp_state = SPCNXT;
            break;
        case SPCR:
            if (b->tag == TAGR)
            {
                sp_state = SPCRET;
                break;
            }
            sp_state = SPCNXT;
            break;
        case SPCO:
            if (b->tag == TAGO)
            {
                sp_state = SPCRET;
                break;
            }
            sp_state = SPCNXT;
            break;
        case SPCD:
            if (b->tag != TAGO)
            {
                sp_state = SPCNXT;
                break;
            }
            if (digit(b->info.infoc))
            {
                sp_state = SPCRET;
                break;
            }
            sp_state = SPCNXT;
            break;
        case SPCL:
            if (b->tag != TAGO)
            {
                sp_state = SPCNXT;
                break;
            }
            if (letter(b->info.infoc))
            {
                sp_state = SPCRET;
                break;
            }
            sp_state = SPCNXT;
        }
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
    return;
}

void putjs(T_WJS *jsp, T_LINKCB **ab1, T_LINKCB **ab2, const size_t *anel, uint8_t **avpc)
{
    jsp->jsb1 = *ab1;
    jsp->jsb2 = *ab2;
    jsp->jsnel = *anel;
    jsp->jsvpc = *avpc;
    return;
}

void getjs(const T_WJS *jsp, T_LINKCB **ab1, T_LINKCB **ab2, size_t *anel, uint8_t **avpc)
{
    *ab1 = jsp->jsb1;
    *ab2 = jsp->jsb2;
    *anel = jsp->jsnel;
    *avpc = jsp->jsvpc;
    return;
}

void putts(T_TS *tsp, T_LINKCB **ax, T_LINKCB **ay, T_LINKCB **az)
{
    tsp->ts0 = *ax;
    tsp->ts1 = *ay;
    tsp->ts2 = *az;
    return;
}

void getts(const T_TS *tsp, T_LINKCB **ax, T_LINKCB **ay, T_LINKCB **az)
{
    *ax = tsp->ts0;
    *ay = tsp->ts1;
    *az = tsp->ts2;
    return;
}

/*void move(size_t n, const uint8_t *pf, uint8_t *pt)
{
    for (size_t i = 0; i < n; i++)
    {
        *pt = *pf;
        pt++;
        pf++;
    }
    return;
}*/

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

/*bool cmpr(size_t n, const uint8_t *p1, const uint8_t *p2)
{
    for (size_t i = 1; i <= n; i++)
    {
        if (*p1 != *p2)
            return false;
        p1++;
        p2++;
    }
    return true;
}*/
//------------ end of file RFRUN2.C ----------
