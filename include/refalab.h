// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-04-14
// https://github.com/Aleksandr3Bocharov/refalab

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
#define BRACKET(b) (b->tag & 0001) != 0
#define NOT_BRACKET(b) (b->tag & 0001) == 0

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

typedef struct status_table
{
    struct status_table *previous;
    struct status_table *next;
    uint8_t state;
    T_LINKCB *dot;
    uint32_t step;
    uint32_t stop;
    T_LINKCB *view;
    T_LINKCB *store;
} T_STATUS_TABLE;

typedef struct timespec T_TIMESPEC;

typedef struct refal
{
    T_STATUS_TABLE *last_status_table;
    T_STATUS_TABLE *first_status_table;
    uint8_t upshot;
    T_LINKCB *previous_argument;
    T_LINKCB *next_argument;
    T_LINKCB *previous_result;
    T_LINKCB *next_result;
    T_STATUS_TABLE *current_status_table;
    T_LINKCB *free_memory_list_head;
    T_LINKCB *static_boxes;
    T_LINKCB *dynamic_boxes;
    struct
    {
        size_t argc;
        char **argv;
    } arg;
    struct
    {
        bool mode;
        T_TIMESPEC start_time;
        T_TIMESPEC stop_time;
    } timer;
} T_REFAL;

extern T_REFAL refal;

extern void refal_abort_end(const char *abort_message);
extern void rfcanc(const T_STATUS_TABLE *ast);
extern void rfdel(T_LINKCB *p, T_LINKCB *q);
extern void rfexec(uint8_t *func);
extern void refal_get_args(int argc, char *argv[]);
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
extern bool lcre(T_STATUS_TABLE *ast);
extern bool lexist(const T_STATUS_TABLE *ast);
extern bool more_free_memory(void);
extern bool lins(T_LINKCB *p, size_t l);
extern bool slins(T_LINKCB *p, size_t k);
extern bool linskd(T_STATUS_TABLE *ast, uint8_t *f);
extern T_LINKCB *lldupl(const T_LINKCB *p, const T_LINKCB *q, const T_LINKCB *u);
extern bool check_count_free_memory_list(size_t count);

extern void rfrun(T_STATUS_TABLE *ast);

extern void (*dbt)(T_STATUS_TABLE *);
extern void rfdbg(T_STATUS_TABLE *s_st);

#endif

//-------  end of file REFALAB.H ---------
