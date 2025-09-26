// Copyright 2025 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2025-09-25
// https://github.com/Aleksandr3Bocharov/RefalAB

//--------------- file -- REFALAB.H -----------------
//          Developer version
//---------------------------------------------------

#ifndef REFALAB_H
#define REFALAB_H

#include <stdio.h>
#include <stddef.h>
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

#define NMBL sizeof(char)
#define ZBLL sizeof(uint16_t)
#define LBLL sizeof(void *)
#define SMBL (ZBLL + LBLL)

#define MAX_NUMBER 0xFFFFFFFFLL

#define MAX_STOP 0x7FFFFFFF

#define MAX_PATHFILENAME 4096

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

extern T_REFAL refal;

extern void rfabe(const char *amsg);
extern void rfcanc(const T_ST *ast);
extern void rfdel(T_LINKCB *p, T_LINKCB *q);
extern void rfexec(uint8_t *func);
extern T_LINKCB *rfgstr(char *str, size_t l, T_LINKCB *p);
extern void rfinit(void);
extern void rfpex(const char *pt, const T_LINKCB *pr, const T_LINKCB *pn, const bool nl);
extern void rfpexm(const char *pt, const T_LINKCB *pr, const T_LINKCB *pn, const bool nl);
extern void rfpexs(const char *pt, const T_LINKCB *pr, const T_LINKCB *pn, const bool nl);
extern bool rfreof(int c, FILE *f, T_LINKCB *p);
extern T_LINKCB *rfrstr(const char *str, T_LINKCB *p);
extern void rftermm(void);
extern void rftpl(T_LINKCB *r, T_LINKCB *p, T_LINKCB *q);

extern bool lcopy(T_LINKCB *r, const T_LINKCB *p, const T_LINKCB *q);
extern bool lcre(T_ST *ast);
extern bool lexist(const T_ST *ast);
extern bool lincrm(void);
extern bool lins(T_LINKCB *p, size_t l);
extern bool slins(T_LINKCB *p, size_t k);
extern bool linskd(T_ST *ast, uint8_t *f);
extern T_LINKCB *lldupl(const T_LINKCB *p, const T_LINKCB *q, const T_LINKCB *u);
extern bool lrqlk(size_t l);

extern void rfrun(T_ST *ast);

extern void rfgetargs(int argc, char *argv[]);

extern void (*dbt)(T_ST *);
extern void rfdbg(T_ST *s_st);

#endif

//-------  end of file REFALAB.H ---------
