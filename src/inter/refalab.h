// Copyright 2025 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2024-03-20
// https://github.com/Aleksandr3Bocharov/RefalAB

//--------------- file -- REFALAB.H -----------------
//          Interpretator version
//---------------------------------------------------

#ifndef REFALAB_H
#define REFALAB_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#define TAGO 0
#define TAGF 2
#define TAGN 4
#define TAGR 6
#define TAGLB 1
#define TAGRB 3
#define TAGK 5
#define TAGD 7

#define Z_0 0
#define Z1 '0', '0', '0', '0', '0',
#define Z2 '0', '0', '0', '0',
#define Z3 '0', '0', '0',
#define Z4 '0', '0',
#define Z5 '0',
#define Z6
#define Z7 '0', '0', '0', '0', '0', '0', '0',
#define Z8 '0', '0', '0', '0', '0', '0',

#define G_L_B
#define gcoden(p) p->info.coden
#define pcoden(p, W) p->info.coden = W

#define NMBL sizeof(char)
#define ZBLL sizeof(uint16_t)
#define LBLL sizeof(void *)
#define SMBL (ZBLL + LBLL)

// pragma for 1 byte aligniment structure ------------
#pragma pack(1)

    typedef struct linkcb_
{
    struct linkcb_ *prev;
    struct linkcb_ *next;
    uint16_t tag;
    union
    {
        char infoc;
        uint32_t coden;
        struct linkcb_ *codep;
        uint8_t *codef;
    } info;
} T_LINKCB;

// #pragma pack(pop)
#pragma pack()

// ------------ end of pragma ------------------------------------

typedef struct st_
{
    struct st_ *stprev;
    struct st_ *stnext;
    uint32_t state;
    T_LINKCB *dot;
    uint32_t step;
    uint32_t stop;
    T_LINKCB *view;
    T_LINKCB *store;
} T_ST;

typedef struct timespec T_TIMESPEC;

typedef struct refal_
{
    T_ST *crprev;
    T_ST *crnext;
    uint32_t upshot;
    T_LINKCB *preva;
    T_LINKCB *nexta;
    T_LINKCB *prevr;
    T_LINKCB *nextr;
    T_ST *currst;
    T_LINKCB *flhead;
    T_LINKCB *svar;
    T_LINKCB *dvar;
    struct
    {
        bool mode;
        T_TIMESPEC start;
        T_TIMESPEC stop;
    } tm;
} T_REFAL;

#endif

//-------  end of file REFALAB.H ---------
