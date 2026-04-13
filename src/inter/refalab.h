// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-04-14
// https://github.com/Aleksandr3Bocharov/refalab

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
#define Z0 '0', '0', '0', '0', '0', '0',

#define G_L_B
#define gcoden(p) p->info.coden
#define pcoden(p, W) p->info.coden = W
#define BRA(b) (b->tag & 0001) != 0
#define NBRA(b) (b->tag & 0001) == 0

#define NMBL 1 // sizeof(uint8_t)
#define ZBLL 2 // sizeof(uint16_t)
#define LBLL sizeof(void *)
#define SMBL (ZBLL + LBLL)

#define MAX_NUMBER 0xFFFFFFFFLL

#define MAX_STOP 0x7FFFFFFF

#if defined POSIX
#define MAX_PATHFILENAME 4096
#else
#define MAX_PATHFILENAME 260
#endif

// pragma for 1 byte aligniment structure ------------
#pragma pack(1)

typedef struct linkcb
{
    struct linkcb *previous;
    struct linkcb *next;
    uint16_t tag;
    union
    {
        char infoc;
        uint32_t coden;
        struct linkcb *codep;
        uint8_t *codef;
        void *code;
    } info;
} T_LINKCB;

// #pragma pack(pop)
#pragma pack()

// ------------ end of pragma ------------------------------------

typedef struct status_table_
{
    struct status_table_ *stprev;
    struct status_table_ *stnext;
    uint32_t state;
    T_LINKCB *dot;
    uint32_t step;
    uint32_t stop;
    T_LINKCB *view;
    T_LINKCB *store;
} T_STATUS_TABLE;

typedef struct timespec T_TIMESPEC;

typedef struct refal_
{
    T_STATUS_TABLE *crprev;
    T_STATUS_TABLE *crnext;
    uint32_t upshot;
    T_LINKCB *preva;
    T_LINKCB *nexta;
    T_LINKCB *prevr;
    T_LINKCB *nextr;
    T_STATUS_TABLE *currst;
    T_LINKCB *flhead;
    T_LINKCB *svar;
    T_LINKCB *dvar;
    struct
    {
        size_t argc;
        char **argv;
    } arg;
    struct
    {
        bool mode;
        T_TIMESPEC start;
        T_TIMESPEC stop;
    } tm;
} T_REFAL;

#endif

//-------  end of file REFALAB.H ---------
