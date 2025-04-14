// Copyright 2025 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2025-04-14
// https://github.com/Aleksandr3Bocharov/RefalAB

//----------- file -- D.C ------------------
//       RefalAB debugger functions
//------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include "refalab.h"
#include "d.h"
#include "rfintf.h"
#include "rfrun.h"

typedef enum dbg_states
{
    DBG_NOT_YET,
    DBG_ALREADY,
    DBG_DONE,
    DBG_TRAP,
    DBG_ABEND,
    DBG_ABEND1,
    DBG_EOJ
} T_DBG_STATES;

typedef enum db_states
{
    DB_NOT_YET,
    DB_ALREADY,
    DB_TRAP,
    DB_ABEND,
    DB_EOJ,
    DB_DO,
    DB_AB
} T_DB_STATES;

typedef struct DET_TAB
{
    struct DET_TAB *det_next;
    char gt;
    char ge;
    char eq;
    char ne;
    char lt;
    char le;
    char tr;
    char *det_id;
} DET_TAB;
static DET_TAB *last_det = NULL;
static DET_TAB *det_table;
static DET_TAB dummy_det_table = {NULL, 0, 0, 0, 0, 0, 0, 0, NULL};

static bool trace_cond = false;
static bool trap_cond = false;
static bool ge_all = true;
static bool eq_all = true;
static bool e1empty = false;
static bool was_ge;
static bool was_le;
static bool was_eq;
static uint32_t s_from = 0;
static uint32_t s_upto = 0;
static uint32_t s_stop = 2147483647;
static uint32_t nogcl = 0; // garbage collection counter
static size_t s_arg;
static size_t l_arg;
static char buff_id[100];
static char buff[100];
static char *arg = buff;
static uint32_t printed_step;
static uint32_t curr_step;
static uint32_t euc_step;
static uint32_t res_step;
static uint32_t curr_step1;
static uint32_t curr_step2;
static const T_LINKCB *nextk;
static const T_LINKCB *res_prevk;
static const T_LINKCB *res_nextd;
static T_LINKCB *pk;
static T_LINKCB *prevk;
static T_LINKCB *nextd;
static T_LINKCB *dot1;
static const T_LINKCB *prevk1, *nextd1;
static T_LINKCB *dot2;
static const T_LINKCB *prevk2, *nextd2;

static void init_det_flags(void);
static void get_arg(void);
static bool get_det(void);
static bool get_numb(int32_t *numb);
static bool get_yn(const char *b);
static void dbtry(T_ST *ss_st);
static void getpf(const T_ST *ss_st);
static void one_step(T_ST *ss_st);
static void pr_euc(void);
static void pr_finres(uint32_t xstep, const T_LINKCB *xprevk, const T_LINKCB *xnextd);
static void pr_imres(void);
static void pr_step(void);

void (*dbt)(T_ST *) = NULL;

void rfdbg(T_ST *s_st)
{
    // read task for debugging
    init_det_flags();
    //----------------------------------
    printf("\n ***** RefalAB debugger ***** \n");

    printf("\n > (function list) : ");
    fgets(buff, 100, stdin);
    size_t i;
    for (i = 0; *(buff + i) == ' '; i++)
        ;
    if (*(buff + i) != '\n')
    {
        arg = buff + i;
        trace_cond = true;
        ge_all = false;
        while (*arg != '\n')
        {
            get_arg();
            get_det();
            det_table->gt = 1;
            arg = arg + l_arg + s_arg;
        }
    }
    printf("\n >= (function list) : ");
    fgets(buff, 100, stdin);
    for (i = 0; *(buff + i) == ' '; i++)
        ;
    if (*(buff + i) != '\n')
    {
        arg = buff + i;
        trace_cond = true;
        ge_all = false;
        while (*arg != '\n')
        {
            get_arg();
            get_det();
            det_table->ge = 1;
            arg = arg + l_arg + s_arg;
        }
    }
    printf("\n = (function list) : ");
    fgets(buff, 100, stdin);
    for (i = 0; *(buff + i) == ' '; i++)
        ;
    if (*(buff + i) != '\n')
    {
        arg = buff + i;
        trace_cond = true;
        eq_all = false;
        while (*arg != '\n')
        {
            get_arg();
            get_det();
            det_table->eq = 1;
            arg = arg + l_arg + s_arg;
        }
    }
    printf("\n != (function list) : ");
    fgets(buff, 100, stdin);
    for (i = 0; *(buff + i) == ' '; i++)
        ;
    if (*(buff + i) != '\n')
    {
        arg = buff + i;
        trace_cond = true;
        while (*arg != '\n')
        {
            get_arg();
            get_det();
            det_table->ne = 1;
            arg = arg + l_arg + s_arg;
        }
    }
    printf("\n < (function list) : ");
    fgets(buff, 100, stdin);
    for (i = 0; *(buff + i) == ' '; i++)
        ;
    if (*(buff + i) != '\n')
    {
        arg = buff + i;
        trace_cond = true;
        while (*arg != '\n')
        {
            get_arg();
            get_det();
            det_table->lt = 1;
            arg = arg + l_arg + s_arg;
        }
    }
    printf("\n <= (function list) : ");
    fgets(buff, 100, stdin);
    for (i = 0; *(buff + i) == ' '; i++)
        ;
    if (*(buff + i) != '\n')
    {
        arg = buff + i;
        trace_cond = true;
        while (*arg != '\n')
        {
            get_arg();
            get_det();
            det_table->le = 1;
            arg = arg + l_arg + s_arg;
        }
    }
    printf("\n TRAP (function list) : ");
    fgets(buff, 100, stdin);
    for (i = 0; *(buff + i) == ' '; i++)
        ;
    if (*(buff + i) != '\n')
    {
        arg = buff + i;
        trap_cond = true;
        while (*arg != '\n')
        {
            get_arg();
            get_det();
            det_table->tr = 1;
            arg = arg + l_arg + s_arg;
        }
    }
    while (true)
    {
        printf("\n STOP (step number) : ");
        fgets(buff, 100, stdin);
        for (i = 0; *(buff + i) == ' '; i++)
            ;
        if (*(buff + i) != '\n')
            if (!get_numb((int32_t *)&s_stop))
                continue;
        break;
    }
    while (true)
    {
        printf("\n FROM (step number) : ");
        fgets(buff, 100, stdin);
        for (i = 0; *(buff + i) == ' '; i++)
            ;
        if (*(buff + i) != '\n')
            if (!get_numb((int32_t *)&s_from))
                continue;
        break;
    }
    while (true)
    {
        printf("\n TO (step number) : ");
        fgets(buff, 100, stdin);
        for (i = 0; *(buff + i) == ' '; i++)
            ;
        if (*(buff + i) != '\n')
            if (!get_numb((int32_t *)&s_upto))
                continue;
        break;
    }
    while (true)
    {
        printf("\n E.1= (y/n) : ");
        fgets(buff, 100, stdin);
        for (i = 0; *(buff + i) == ' '; i++)
            ;
        if (*(buff + i) != '\n')
            if (!get_yn(buff + i))
                continue;
        break;
    }
    //  set FROM and TO
    if (!s_from && (s_upto || trace_cond))
        s_from = 1;
    if (!s_upto && s_from)
        s_upto = 0x7FFFFFFF;
    //==================================
    //  initialization
    dbt = dbtry;
    printed_step = 0;
    euc_step = 0;
    res_step = 0;
    res_prevk = res_nextd = NULL;
    T_DBG_STATES dbg_state = DBG_NOT_YET;
    while (true)
        switch (dbg_state)
        {
            // station "not yet"
        case DBG_NOT_YET:
            if (s_st->dot == NULL)
            {
                dbg_state = DBG_DONE;
                break;
            }
            if (s_stop < s_st->step)
            {
                dbg_state = DBG_ABEND;
                break;
            }
            getpf(s_st);
            if (!ge_all && !det_table->ge && !det_table->gt)
            {
                if (det_table->tr)
                {
                    dbg_state = DBG_TRAP;
                    break;
                }
                one_step(s_st);
                if (s_st->state != 1)
                {
                    dbg_state = DBG_ABEND;
                    break;
                }
                dbg_state = DBG_NOT_YET;
                break;
            }
            // enter into station "is already"
            if ((!ge_all && !det_table->ge) || det_table->gt)
                was_ge = false;
            else
            {
                was_ge = true;
                if (!ge_all)
                    pr_euc();
            }
            //  cut
            curr_step1 = curr_step;
            prevk1 = prevk;
            nextd1 = nextd;
            if (pk->info.codep == NULL)
                dot1 = NULL;
            else
            {
                dot1 = pk->info.codep;
                pk->info.codep = NULL;
            }
            dbg_state = DBG_ALREADY;
            break;
        // "is already" station
        case DBG_ALREADY:
            if (det_table->le || det_table->lt)
            {
                //  "isn't already"
                if (det_table->lt)
                    was_le = false;
                else
                {
                    was_le = true;
                    pr_euc();
                }
                //   cut
                curr_step2 = curr_step;
                prevk2 = prevk;
                nextd2 = nextd;
                if (pk->info.codep == NULL)
                    dot2 = NULL;
                else
                {
                    dot2 = pk->info.codep;
                    pk->info.codep = NULL;
                }
                // compute call entirely
                bool quit = false;
                while (s_st->dot != NULL)
                {
                    getpf(s_st);
                    if (det_table->tr)
                    {
                        dbg_state = DBG_TRAP;
                        quit = true;
                        break;
                    }
                    one_step(s_st);
                    if (s_stop < s_st->step)
                    {
                        dbg_state = DBG_ABEND;
                        quit = true;
                        break;
                    }
                    curr_step = s_st->step + 1;
                    if (s_st->state != 1)
                    {
                        dbg_state = DBG_ABEND;
                        quit = true;
                        break;
                    }
                }
                if (quit)
                    break;
                //  joint
                curr_step = s_st->step;
                s_st->dot = dot2;
                if (was_le)
                    pr_finres(curr_step2, prevk2, nextd2);
            } // for label ALREADY
            else
            { // step in station "is already"
                if (s_stop < s_st->step)
                {
                    dbg_state = DBG_ABEND;
                    break;
                }
                if ((!eq_all && !det_table->eq) || det_table->ne)
                    was_eq = false;
                else
                {
                    was_eq = true;
                    pr_euc();
                }
                if (det_table->tr)
                {
                    dbg_state = DBG_TRAP;
                    break;
                }
                one_step(s_st);
                if (s_st->state != 1)
                {
                    dbg_state = DBG_ABEND;
                    break;
                }
                if (was_eq)
                    pr_imres();
            }
            if (s_st->dot != NULL)
            {
                getpf(s_st);
                dbg_state = DBG_ALREADY;
                break;
            }
            //  joint
            s_st->dot = dot1;
            if (!ge_all && was_ge)
                pr_finres(curr_step1, prevk1, nextd1);
            dbg_state = DBG_NOT_YET;
            break;
        case DBG_DONE:
            printf("\nConcretization is executed ");
            dbg_state = DBG_EOJ;
            break;
        case DBG_TRAP:
            printf("\nFunction name trap");
            dbg_state = DBG_ABEND1;
            break;
        case DBG_ABEND:
            switch (s_st->state)
            {
            case 1:
                printf("\nStop on step number ");
                break;
            case 2:
                printf("\nRecognition impossible");
                break;
            case 3:
                printf("\nFree memory exhausted ");
            }
            getpf(s_st);
            dbg_state = DBG_ABEND1;
            break;
        case DBG_ABEND1:
            printf("\nLeading functional term: ");
            rfpexm("     ", prevk, nextd);
            dbg_state = DBG_EOJ;
            break;
        case DBG_EOJ:
            printf("\nCompleted steps number = %u", s_st->step);
            printf("\nView field: ");
            rfpexm("     ", s_st->view, s_st->view);
            if (s_st->store->next != s_st->store)
            {
                printf("\nBurried: ");
                rfpexm("     ", s_st->store, s_st->store);
            }
            if (nogcl != 0)
                printf("\nGarbage collection number = %d", nogcl);
            if (refal.tm.mode)
            {
                timespec_get(&refal.tm.stop, TIME_UTC);
                int32_t in = (int32_t)(refal.tm.stop.tv_nsec - refal.tm.start.tv_nsec);
                uint32_t is = (uint32_t)difftime(refal.tm.stop.tv_sec, refal.tm.start.tv_sec);
                if (in < 0)
                {
                    in += 1000000000;
                    is--;
                }
                uint32_t im = is / 60;
                is %= 60;
                const uint32_t ih = im / 60;
                im %= 60;
                char s[30];
                sprintf(s, "%02u:%02u:%02u.%09d", ih, im, is, in);
                printf("\nElapsed time = %s", s);
            }
            rfcanc(s_st);
            rftermm();
            printf("\n");
            exit(0);
            return;
        }
}

static void dbtry(T_ST *ss_st)
{
    T_LINKCB *v1 = prevk;
    T_LINKCB *v2 = nextd;
    T_LINKCB *v3 = pk;
    const T_LINKCB *v4 = nextk;
    uint32_t v5 = res_step;
    const T_LINKCB *v6 = res_prevk;
    const T_LINKCB *v7 = res_nextd;
    T_DB_STATES db_state = DB_NOT_YET;
    while (true)
        switch (db_state)
        {
        case DB_NOT_YET:
            if (ss_st->dot == NULL)
            {
                db_state = DB_DO;
                break;
            }
            if (s_stop < ss_st->step)
            {
                db_state = DB_ABEND;
                break;
            }
            getpf(ss_st);
            if (!ge_all && !det_table->ge && !det_table->gt)
            {
                if (det_table->tr)
                {
                    db_state = DB_TRAP;
                    break;
                }
                one_step(ss_st);
                if (ss_st->state != 1)
                {
                    db_state = DB_AB;
                    break;
                }
                db_state = DB_NOT_YET;
                break;
            }
            // enter into station "is already"
            if ((!ge_all && !det_table->ge) || det_table->gt)
                was_ge = false;
            else
            {
                was_ge = true;
                if (!ge_all)
                    pr_euc();
            }
            //  cut
            curr_step1 = curr_step;
            prevk1 = prevk;
            nextd1 = nextd;
            if (pk->info.codep == NULL)
                dot1 = NULL;
            else
            {
                dot1 = pk->info.codep;
                pk->info.codep = NULL;
            }
            db_state = DB_ALREADY;
            break;
        // "is already" station
        case DB_ALREADY:
            if (det_table->le || det_table->lt)
            {
                //  "isn't already"
                if (det_table->lt)
                    was_le = false;
                else
                {
                    was_le = true;
                    pr_euc();
                }
                //   cut
                curr_step2 = curr_step;
                prevk2 = prevk;
                nextd2 = nextd;
                if (pk->info.codep == NULL)
                    dot2 = NULL;
                else
                {
                    dot2 = pk->info.codep;
                    pk->info.codep = NULL;
                }
                // compute call entirely
                bool quit = false;
                while (ss_st->dot != NULL)
                {
                    getpf(ss_st);
                    if (det_table->tr)
                    {
                        db_state = DB_TRAP;
                        quit = true;
                        break;
                    }
                    one_step(ss_st);
                    if (s_stop < ss_st->step)
                    {
                        db_state = DB_ABEND;
                        quit = true;
                        break;
                    }
                    curr_step = ss_st->step + 1;
                    if (ss_st->state != 1)
                    {
                        db_state = DB_AB;
                        quit = true;
                        break;
                    }
                }
                if (quit)
                    break;
                //  joint
                curr_step = ss_st->step;
                ss_st->dot = dot2;
                if (was_le)
                    pr_finres(curr_step2, prevk2, nextd2);
            } // for label ALREADY
            else
            { // step in station "is already"
                if (s_stop < ss_st->step)
                {
                    db_state = DB_ABEND;
                    break;
                }
                if ((!eq_all && !det_table->eq) || det_table->ne)
                    was_eq = false;
                else
                {
                    was_eq = true;
                    pr_euc();
                }
                if (det_table->tr)
                {
                    db_state = DB_TRAP;
                    break;
                }
                one_step(ss_st);
                if (ss_st->state != 1)
                {
                    db_state = DB_AB;
                    break;
                }
                if (was_eq)
                    pr_imres();
            }
            if (ss_st->dot != NULL)
            {
                getpf(ss_st);
                db_state = DB_ALREADY;
                break;
            }
            //  joint
            ss_st->dot = dot1;
            if (!ge_all && was_ge)
                pr_finres(curr_step1, prevk1, nextd1);
            db_state = DB_NOT_YET;
            break;
        case DB_TRAP:
            printf("\nFunction name trap");
            db_state = DB_ABEND;
            break;
        case DB_ABEND:
            printf("\nLeading functional term: ");
            rfpexm("     ", prevk, nextd);
            db_state = DB_EOJ;
            break;
        case DB_EOJ:
            printf("\nCompleted steps number = %u", ss_st->step);
            printf("\nView field: ");
            rfpexm("     ", ss_st->view, ss_st->view);
            if (ss_st->store->next != ss_st->store)
            {
                printf("\nBurried: ");
                rfpexm("     ", ss_st->store, ss_st->store);
            }
            if (nogcl != 0)
                printf("\nGarbage collection number = %d", nogcl);
            if (refal.tm.mode)
            {
                timespec_get(&refal.tm.stop, TIME_UTC);
                int32_t in = (int32_t)(refal.tm.stop.tv_nsec - refal.tm.start.tv_nsec);
                uint32_t is = (uint32_t)difftime(refal.tm.stop.tv_sec, refal.tm.start.tv_sec);
                if (in < 0)
                {
                    in += 1000000000;
                    is--;
                }
                uint32_t im = is / 60;
                is %= 60;
                const uint32_t ih = im / 60;
                im %= 60;
                char s[30];
                sprintf(s, "%02u:%02u:%02u.%09d", ih, im, is, in);
                printf("\nElapsed time = %s", s);
            }
            printf("\n");
            exit(0);
            return;
        case DB_DO:
        case DB_AB:
            prevk = v1;
            nextd = v2;
            pk = v3;
            nextk = v4;
            res_step = v5;
            res_prevk = v6;
            res_nextd = v7;
            return;
        }
}

//    procedures
static void init_det_flags(void)
{
    for (DET_TAB *det = last_det; det != NULL;)
    {
        free(det->det_id);
        DET_TAB *det1 = det;
        det = det->det_next;
        free(det1);
    }
    last_det = NULL;
    return;
}

static void one_step(T_ST *ss_st)
{
    ss_st->stop = ss_st->step + 1;
    while (true)
    {
        rfrun(ss_st);
        if (ss_st->state != 3)
            break;
        if (refal.dvar != NULL)
            nogcl++;
        if (lincrm())
            continue;
        break;
    }
    if (e1empty && ss_st->state == 2)
    {
        pr_step();
        if (euc_step != curr_step)
        {
            euc_step = curr_step;
            rfpexm("       Leading term : ", prevk, nextd);
        }
        printf("\n*** Recognition impossible ");
        printf("\n*** Change leading term by empty term and continue ***");
        ss_st->dot = pk->info.codep;
        rfdel(prevk, nextd);
        ss_st->state = 1;
        ss_st->step++;
    };
    return;
}

static void pr_step(void)
{
    if (curr_step != printed_step)
    {
        printf("\n***** Step %u", curr_step);
        printed_step = curr_step;
    }
    return;
}

static void pr_euc(void)
{
    if (curr_step > s_upto || curr_step < s_from)
        return;
    if (euc_step != curr_step)
    {
        euc_step = curr_step;
        if (res_step != curr_step - 1 || res_prevk != prevk ||
            res_nextd != nextd)
        {
            pr_step();
            rfpexm("      Leading term : ", prevk, nextd);
        }
    }
    return;
}

static void pr_imres(void)
{
    if (curr_step > s_upto || curr_step < s_from)
        return;
    pr_step();
    rfpexm("      Result : ", prevk, nextd);
    res_step = curr_step;
    res_prevk = prevk;
    res_nextd = nextd;
    return;
}

static void pr_finres(uint32_t xstep, const T_LINKCB *xprevk, const T_LINKCB *xnextd)
{
    if (curr_step > s_upto || curr_step < s_from)
        return;
    pr_step();
    if (curr_step == res_step && res_prevk == xprevk &&
        res_nextd == xnextd)
    {
        if (xstep == curr_step)
            return;
        printf("\n----- This is result of call on step %u", xstep);
    }
    else
    {
        if (xstep == curr_step)
        {
            pr_imres();
            return;
        }
        printf("\n----- Result of call on step %u : ", xstep);
        rfpexm("     ", xprevk, xnextd);
        res_step = curr_step;
        res_prevk = xprevk;
        res_nextd = xnextd;
    }
    return;
}

static void getpf(const T_ST *ss_st)
{
    curr_step = ss_st->step + 1;
    pk = ss_st->dot->info.codep;
    prevk = pk->prev;
    nextd = ss_st->dot->next;
    nextk = pk->next;
    size_t i;
    uint8_t id_l;
    const uint8_t *p_id;
    if (nextk->tag != TAGF)
    {
        buff_id[0] = '%';
        buff_id[1] = '\0';
    }
    else
    {
        p_id = nextk->info.codef - 1;
        id_l = *p_id;
        p_id -= id_l;
        for (i = 0; i < id_l; i++)
            buff_id[i] = (char)toupper(*(p_id + i));
        buff_id[id_l] = '\0';
    }
    det_table = last_det;
    if (strlen(buff_id) == 0)
        det_table = NULL;
    else
        while (det_table != NULL)
        {
            for (i = 0; buff_id[i] != '\0'; i++)
                if (*(det_table->det_id + i) != buff_id[i])
                    break;
            if (buff_id[i] == '\0' && *(det_table->det_id + i) == '\0')
                break;
            det_table = det_table->det_next;
        }
    if (det_table == NULL)
        det_table = &dummy_det_table;
    return;
}

static void get_arg(void)
{
    for (l_arg = 0;; l_arg++)
    {
        *(arg + l_arg) = (char)toupper(*(arg + l_arg));
        if (*(arg + l_arg) == '\n' || *(arg + l_arg) == ' ' || *(arg + l_arg) == '\0' || *(arg + l_arg) == ',')
            break;
    }
    for (s_arg = 0; *(arg + s_arg) == ' ' || *(arg + s_arg) == ','; s_arg++)
        ;
    return;
}

static bool get_det(void)
{
    det_table = last_det;
    while (det_table != NULL)
    {
        if (strncmp(det_table->det_id, arg, l_arg) == 0)
            if (*(det_table->det_id + l_arg) == '\0')
                return true;
        det_table = det_table->det_next;
    }
    bool ab = false;
    det_table = (DET_TAB *)malloc(sizeof(DET_TAB));
    if (det_table == NULL)
        ab = true;
    else
    {
        det_table->det_id = malloc(l_arg + 1);
        if (det_table->det_id == NULL)
            ab = true;
    }
    if (ab)
    {
        printf("\nRefalAB debugger: no storage\n");
        exit(1);
        return false;
    }
    for (size_t i = 0; i < l_arg; i++)
        *(det_table->det_id + i) = *(arg + i);
    *(det_table->det_id + l_arg) = '\0';
    det_table->det_next = last_det;
    last_det = det_table;
    det_table->ge = 0;
    det_table->gt = 0;
    det_table->eq = 0;
    det_table->ne = 0;
    det_table->le = 0;
    det_table->lt = 0;
    det_table->tr = 0;
    return true;
}

static bool get_numb(int32_t *numb)
{
    if (sscanf(buff, "%d", numb) == 0 || *numb < 1)
    {
        printf("\n                        Invalid number; repeat please.");
        return false;
    }
    return true;
}

static bool get_yn(const char *b)
{
    if (*b != 'y' && *b != 'n')
    {
        printf("\n                        Answer is \"y/n\"; repeat please.");
        return false;
    }
    if (*b == 'y')
        e1empty = true;
    return true;
}

//-----------  end of file D.C -------------
