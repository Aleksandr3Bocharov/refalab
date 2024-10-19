// Copyright 2024 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2024-10-19
// https://github.com/Aleksandr3Bocharov/RefalAB

//----------- file RFRUN2.C -----------------
//      Refal-interpretator (part 2)
//-------------------------------------------

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "refal.def"
#include "rfrun2.h"

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
    memcpy(&spcvpc, vpc + NMBL, LBLL);
    uint8_t spcopc;
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
            spcpls = spcsp->spls;
            spcvpc = spcsp->svpc;
            if (spcwrk)
                break;
            spcvpc = spcvpc + LBLL;
            sp_state = SPCNXT;
            break;
        // return from specifier element if "NO"
        case SPCNXT:
            // specifier code
            spcopc = *spcvpc;
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
            if (memcmp(spcvpc, &b->tag, SMBL) == 0)
            {
                sp_state = SPCRET;
                break;
            }
            spcvpc = spcvpc + SMBL;
            sp_state = SPCNXT;
            break;
        case SPCS:
            if ((b->tag & 0001) == TAGO)
            {
                sp_state = SPCRET;
                break;
            }
            sp_state = SPCNXT;
            break;
        case SPCB:
            if ((b->tag & 0001) != TAGO)
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
    if ((s >= 'A' && s <= 'Z') ||                 // A..Z
        (s >= 'a' && s <= 'z') ||                 // a..z
        ((uint8_t)s > 127 && (uint8_t)s < 176) || // �..��..�
        ((uint8_t)s > 223 && (uint8_t)s < 240))   // �..�
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

//------------ end of file RFRUN2.C ----------
