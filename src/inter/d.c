// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-04-20
// https://github.com/Aleksandr3Bocharov/refalab

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

typedef enum debugger_states
{
    DBG_NOT_YET,
    DBG_ALREADY,
    DBG_DONE,
    DBG_TRAP,
    DBG_ABEND,
    DBG_ABEND1,
    DBG_EOJ
} T_DEBUGGER_STATES;

typedef enum status_table_debugger_states
{
    DB_NOT_YET,
    DB_ALREADY,
    DB_TRAP,
    DB_ABEND,
    DB_EOJ,
    DB_DO,
    DB_AB
} T_STATUS_TABLE_DEBUGGER_STATES;

typedef struct determination
{
    struct determination *next;
    bool gt;
    bool ge;
    bool eq;
    bool ne;
    bool lt;
    bool le;
    bool tr;
    char *identifier;
} T_DETERMINATION;
static T_DETERMINATION *last_determination = NULL;
static T_DETERMINATION *current_determination;
static T_DETERMINATION dummy_determination = {NULL, 0, 0, 0, 0, 0, 0, 0, NULL};

static bool trace_condition = false;
static bool ge_all = true;
static bool eq_all = true;
static bool e_empty = false;
static bool was_ge;
static bool was_le;
static bool was_eq;
static uint32_t step_from = 0;
static uint32_t step_upto = 0;
static uint32_t step_stop = MAX_STOP;
static uint32_t garbage_collection_number = 0; // garbage collection counter
static size_t parameters_length;
static char *parameters = NULL;
static size_t s_arg;
static size_t l_arg;
static char *parameter = NULL;
static char buff_id[100];
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
static char *fgetline(FILE *file, size_t *line_length);
static void get_arg(void);
static bool get_det(void);
static bool get_numb(int32_t *numb);
static bool get_yn(const char *b);
static void dbtry(T_STATUS_TABLE *status_table);
static void getpf(const T_STATUS_TABLE *status_table);
static void one_step(T_STATUS_TABLE *status_table);
static void pr_euc(void);
static void pr_finres(uint32_t xstep, const T_LINKCB *xprevk, const T_LINKCB *xnextd);
static void pr_imres(void);
static void pr_step(void);

void (*status_table_debugger)(T_STATUS_TABLE *) = NULL;

void refal_debugger(T_STATUS_TABLE *status_table)
{
    // read task for debugging
    init_det_flags();
    //----------------------------------
    printf("\n ***** RefalAB debugger ***** \n");
    //----------------------------------
    printf("\n > (function list) : ");
    parameters = fgetline(stdin, &parameters_length);
    if (parameters == NULL)
    {
        printf("\nRefalAB debugger: no storage or EOF in stdin\n");
        exit(1);
    }
    size_t i;
    for (i = 0; *(parameters + i) == ' '; i++)
        ;
    if (*(parameters + i) != '\n')
    {
        parameter = parameters + i;
        trace_condition = true;
        ge_all = false;
        while (*parameter != '\n')
        {
            get_arg();
            get_det();
            current_determination->gt = true;
            parameter = parameter + l_arg + s_arg;
        }
    }
    printf("\n >= (function list) : ");
    fgets(parameters, 100, stdin);
    for (i = 0; *(parameters + i) == ' '; i++)
        ;
    if (*(parameters + i) != '\n')
    {
        parameter = parameters + i;
        trace_condition = true;
        ge_all = false;
        while (*parameter != '\n')
        {
            get_arg();
            get_det();
            current_determination->ge = true;
            parameter = parameter + l_arg + s_arg;
        }
    }
    printf("\n = (function list) : ");
    fgets(parameters, 100, stdin);
    for (i = 0; *(parameters + i) == ' '; i++)
        ;
    if (*(parameters + i) != '\n')
    {
        parameter = parameters + i;
        trace_condition = true;
        eq_all = false;
        while (*parameter != '\n')
        {
            get_arg();
            get_det();
            current_determination->eq = true;
            parameter = parameter + l_arg + s_arg;
        }
    }
    printf("\n != (function list) : ");
    fgets(parameters, 100, stdin);
    for (i = 0; *(parameters + i) == ' '; i++)
        ;
    if (*(parameters + i) != '\n')
    {
        parameter = parameters + i;
        trace_condition = true;
        while (*parameter != '\n')
        {
            get_arg();
            get_det();
            current_determination->ne = true;
            parameter = parameter + l_arg + s_arg;
        }
    }
    printf("\n < (function list) : ");
    fgets(parameters, 100, stdin);
    for (i = 0; *(parameters + i) == ' '; i++)
        ;
    if (*(parameters + i) != '\n')
    {
        parameter = parameters + i;
        trace_condition = true;
        while (*parameter != '\n')
        {
            get_arg();
            get_det();
            current_determination->lt = true;
            parameter = parameter + l_arg + s_arg;
        }
    }
    printf("\n <= (function list) : ");
    fgets(parameters, 100, stdin);
    for (i = 0; *(parameters + i) == ' '; i++)
        ;
    if (*(parameters + i) != '\n')
    {
        parameter = parameters + i;
        trace_condition = true;
        while (*parameter != '\n')
        {
            get_arg();
            get_det();
            current_determination->le = true;
            parameter = parameter + l_arg + s_arg;
        }
    }
    printf("\n TRAP (function list) : ");
    fgets(parameters, 100, stdin);
    for (i = 0; *(parameters + i) == ' '; i++)
        ;
    if (*(parameters + i) != '\n')
    {
        parameter = parameters + i;
        while (*parameter != '\n')
        {
            get_arg();
            get_det();
            current_determination->tr = true;
            parameter = parameter + l_arg + s_arg;
        }
    }
    while (true)
    {
        printf("\n STOP (step number) : ");
        fgets(parameters, 100, stdin);
        for (i = 0; *(parameters + i) == ' '; i++)
            ;
        if (*(parameters + i) != '\n')
            if (!get_numb((int32_t *)&step_stop))
                continue;
        break;
    }
    while (true)
    {
        printf("\n FROM (step number) : ");
        fgets(parameters, 100, stdin);
        for (i = 0; *(parameters + i) == ' '; i++)
            ;
        if (*(parameters + i) != '\n')
            if (!get_numb((int32_t *)&step_from))
                continue;
        break;
    }
    while (true)
    {
        printf("\n TO (step number) : ");
        fgets(parameters, 100, stdin);
        for (i = 0; *(parameters + i) == ' '; i++)
            ;
        if (*(parameters + i) != '\n')
            if (!get_numb((int32_t *)&step_upto))
                continue;
        break;
    }
    while (true)
    {
        printf("\n E._= (y/n) : ");
        fgets(parameters, 100, stdin);
        for (i = 0; *(parameters + i) == ' '; i++)
            ;
        if (*(parameters + i) != '\n')
            if (!get_yn(parameters + i))
                continue;
        break;
    }
    printf("\n");
    //  set FROM and TO
    if (!step_from && (step_upto || trace_condition))
        step_from = 1;
    if (!step_upto && step_from)
        step_upto = 0x7FFFFFFF;
    //==================================
    //  initialization
    status_table_debugger = dbtry;
    printed_step = 0;
    euc_step = 0;
    res_step = 0;
    res_prevk = res_nextd = NULL;
    T_DEBUGGER_STATES debugger_state = DBG_NOT_YET;
    while (true)
        switch (debugger_state)
        {
            // station "not yet"
        case DBG_NOT_YET:
            if (status_table->dot == NULL)
            {
                debugger_state = DBG_DONE;
                break;
            }
            if (step_stop < status_table->step)
            {
                debugger_state = DBG_ABEND;
                break;
            }
            getpf(status_table);
            if (!ge_all && !current_determination->ge && !current_determination->gt)
            {
                if (current_determination->tr)
                {
                    debugger_state = DBG_TRAP;
                    break;
                }
                one_step(status_table);
                if (status_table->state != 1)
                {
                    debugger_state = DBG_ABEND;
                    break;
                }
                debugger_state = DBG_NOT_YET;
                break;
            }
            // enter into station "is already"
            if ((!ge_all && !current_determination->ge) || current_determination->gt)
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
            if (pk->info.code == NULL)
                dot1 = NULL;
            else
            {
                dot1 = pk->info.codep;
                pk->info.code = NULL;
            }
            debugger_state = DBG_ALREADY;
            break;
        // "is already" station
        case DBG_ALREADY:
            if (current_determination->le || current_determination->lt)
            {
                //  "isn't already"
                if (current_determination->lt)
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
                if (pk->info.code == NULL)
                    dot2 = NULL;
                else
                {
                    dot2 = pk->info.codep;
                    pk->info.code = NULL;
                }
                // compute call entirely
                bool quit = false;
                while (status_table->dot != NULL)
                {
                    getpf(status_table);
                    if (current_determination->tr)
                    {
                        debugger_state = DBG_TRAP;
                        quit = true;
                        break;
                    }
                    one_step(status_table);
                    if (step_stop < status_table->step)
                    {
                        debugger_state = DBG_ABEND;
                        quit = true;
                        break;
                    }
                    curr_step = status_table->step + 1;
                    if (status_table->state != 1)
                    {
                        debugger_state = DBG_ABEND;
                        quit = true;
                        break;
                    }
                }
                if (quit)
                    break;
                //  joint
                curr_step = status_table->step;
                status_table->dot = dot2;
                if (was_le)
                    pr_finres(curr_step2, prevk2, nextd2);
            } // for label ALREADY
            else
            { // step in station "is already"
                if (step_stop < status_table->step)
                {
                    debugger_state = DBG_ABEND;
                    break;
                }
                if ((!eq_all && !current_determination->eq) || current_determination->ne)
                    was_eq = false;
                else
                {
                    was_eq = true;
                    pr_euc();
                }
                if (current_determination->tr)
                {
                    debugger_state = DBG_TRAP;
                    break;
                }
                one_step(status_table);
                if (status_table->state != 1)
                {
                    debugger_state = DBG_ABEND;
                    break;
                }
                if (was_eq)
                    pr_imres();
            }
            if (status_table->dot != NULL)
            {
                getpf(status_table);
                debugger_state = DBG_ALREADY;
                break;
            }
            //  joint
            status_table->dot = dot1;
            if (!ge_all && was_ge)
                pr_finres(curr_step1, prevk1, nextd1);
            debugger_state = DBG_NOT_YET;
            break;
        case DBG_DONE:
            printf("Concretization is executed\n");
            debugger_state = DBG_EOJ;
            break;
        case DBG_TRAP:
            printf("Function name trap\n");
            debugger_state = DBG_ABEND1;
            break;
        case DBG_ABEND:
            switch (status_table->state)
            {
            case 1:
                printf("Stop on step number\n");
                break;
            case 2:
                printf("Recognition impossible\n");
                break;
            case 3:
                printf("Free memory exhausted\n");
            }
            getpf(status_table);
            debugger_state = DBG_ABEND1;
            break;
        case DBG_ABEND1:
            printf("Leading functional term:\n");
            print_expression_m("     ", prevk, nextd, true);
            debugger_state = DBG_EOJ;
            break;
        case DBG_EOJ:
            printf("Completed steps number = %u\n", status_table->step);
            printf("View field:\n");
            print_expression_m("     ", status_table->view, status_table->view, true);
            if (status_table->store->next != status_table->store)
            {
                printf("Burried:\n");
                print_expression_m("     ", status_table->store, status_table->store, true);
            }
            if (garbage_collection_number != 0)
                printf("Garbage collection number = %d\n", garbage_collection_number);
            if (refal.timer.mode)
            {
                timespec_get(&refal.timer.stop_time, TIME_UTC);
                long int in = refal.timer.stop_time.tv_nsec - refal.timer.start_time.tv_nsec;
                long long int is = (long long int)difftime(refal.timer.stop_time.tv_sec, refal.timer.start_time.tv_sec);
                if (in < 0)
                {
                    in += 1000000000;
                    is--;
                }
                long long int im = is / 60;
                is %= 60;
                const long long int ih = im / 60;
                im %= 60;
                char s[64];
                sprintf(s, "%02lld:%02lld:%02lld.%09ld", ih, im, is, in);
                printf("Elapsed time = %s\n", s);
            }
            delete_status_table(status_table);
            refal_terminate_memory();
            exit(0);
            return;
        }
}

static void dbtry(T_STATUS_TABLE *status_table)
{
    T_LINKCB *v1 = prevk;
    T_LINKCB *v2 = nextd;
    T_LINKCB *v3 = pk;
    const T_LINKCB *v4 = nextk;
    uint32_t v5 = res_step;
    const T_LINKCB *v6 = res_prevk;
    const T_LINKCB *v7 = res_nextd;
    T_STATUS_TABLE_DEBUGGER_STATES status_table_debugger_state = DB_NOT_YET;
    while (true)
        switch (status_table_debugger_state)
        {
        case DB_NOT_YET:
            if (status_table->dot == NULL)
            {
                status_table_debugger_state = DB_DO;
                break;
            }
            if (step_stop < status_table->step)
            {
                status_table_debugger_state = DB_ABEND;
                break;
            }
            getpf(status_table);
            if (!ge_all && !current_determination->ge && !current_determination->gt)
            {
                if (current_determination->tr)
                {
                    status_table_debugger_state = DB_TRAP;
                    break;
                }
                one_step(status_table);
                if (status_table->state != 1)
                {
                    status_table_debugger_state = DB_AB;
                    break;
                }
                status_table_debugger_state = DB_NOT_YET;
                break;
            }
            // enter into station "is already"
            if ((!ge_all && !current_determination->ge) || current_determination->gt)
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
            if (pk->info.code == NULL)
                dot1 = NULL;
            else
            {
                dot1 = pk->info.codep;
                pk->info.code = NULL;
            }
            status_table_debugger_state = DB_ALREADY;
            break;
        // "is already" station
        case DB_ALREADY:
            if (current_determination->le || current_determination->lt)
            {
                //  "isn't already"
                if (current_determination->lt)
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
                if (pk->info.code == NULL)
                    dot2 = NULL;
                else
                {
                    dot2 = pk->info.codep;
                    pk->info.code = NULL;
                }
                // compute call entirely
                bool quit = false;
                while (status_table->dot != NULL)
                {
                    getpf(status_table);
                    if (current_determination->tr)
                    {
                        status_table_debugger_state = DB_TRAP;
                        quit = true;
                        break;
                    }
                    one_step(status_table);
                    if (step_stop < status_table->step)
                    {
                        status_table_debugger_state = DB_ABEND;
                        quit = true;
                        break;
                    }
                    curr_step = status_table->step + 1;
                    if (status_table->state != 1)
                    {
                        status_table_debugger_state = DB_AB;
                        quit = true;
                        break;
                    }
                }
                if (quit)
                    break;
                //  joint
                curr_step = status_table->step;
                status_table->dot = dot2;
                if (was_le)
                    pr_finres(curr_step2, prevk2, nextd2);
            } // for label ALREADY
            else
            { // step in station "is already"
                if (step_stop < status_table->step)
                {
                    status_table_debugger_state = DB_ABEND;
                    break;
                }
                if ((!eq_all && !current_determination->eq) || current_determination->ne)
                    was_eq = false;
                else
                {
                    was_eq = true;
                    pr_euc();
                }
                if (current_determination->tr)
                {
                    status_table_debugger_state = DB_TRAP;
                    break;
                }
                one_step(status_table);
                if (status_table->state != 1)
                {
                    status_table_debugger_state = DB_AB;
                    break;
                }
                if (was_eq)
                    pr_imres();
            }
            if (status_table->dot != NULL)
            {
                getpf(status_table);
                status_table_debugger_state = DB_ALREADY;
                break;
            }
            //  joint
            status_table->dot = dot1;
            if (!ge_all && was_ge)
                pr_finres(curr_step1, prevk1, nextd1);
            status_table_debugger_state = DB_NOT_YET;
            break;
        case DB_TRAP:
            printf("Function name trap\n");
            status_table_debugger_state = DB_ABEND;
            break;
        case DB_ABEND:
            printf("Leading functional term:\n");
            print_expression_m("     ", prevk, nextd, true);
            status_table_debugger_state = DB_EOJ;
            break;
        case DB_EOJ:
            printf("Completed steps number = %u\n", status_table->step);
            printf("View field:\n");
            print_expression_m("     ", status_table->view, status_table->view, true);
            if (status_table->store->next != status_table->store)
            {
                printf("Burried:\n");
                print_expression_m("     ", status_table->store, status_table->store, true);
            }
            if (garbage_collection_number != 0)
                printf("Garbage collection number = %d\n", garbage_collection_number);
            if (refal.timer.mode)
            {
                timespec_get(&refal.timer.stop_time, TIME_UTC);
                long int in = refal.timer.stop_time.tv_nsec - refal.timer.start_time.tv_nsec;
                long long int is = (long long int)difftime(refal.timer.stop_time.tv_sec, refal.timer.start_time.tv_sec);
                if (in < 0)
                {
                    in += 1000000000;
                    is--;
                }
                long long int im = is / 60;
                is %= 60;
                const long long int ih = im / 60;
                im %= 60;
                char s[64];
                sprintf(s, "%02lld:%02lld:%02lld.%09ld", ih, im, is, in);
                printf("Elapsed time = %s\n", s);
            }
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
    for (T_DETERMINATION *det = last_determination; det != NULL;)
    {
        free(det->identifier);
        T_DETERMINATION *det1 = det;
        det = det->next;
        free(det1);
    }
    last_determination = NULL;
    return;
}

static void one_step(T_STATUS_TABLE *status_table)
{
    status_table->stop = status_table->step + 1;
    while (true)
    {
        rfrun(status_table);
        if (status_table->state != 3)
            break;
        if (refal.dynamic_boxes != NULL)
            garbage_collection_number++;
        if (more_free_memory())
            continue;
        break;
    }
    if (e_empty && status_table->state == 2)
    {
        pr_step();
        if (euc_step != curr_step)
        {
            euc_step = curr_step;
            print_expression_m("       Leading term : ", prevk, nextd, true);
        }
        printf("*** Recognition impossible\n");
        printf("*** Change leading term by empty term and continue ***\n");
        status_table->dot = pk->info.codep;
        insert_to_free_memory_list(prevk, nextd);
        status_table->state = 1;
        status_table->step++;
    };
    return;
}

static void pr_step(void)
{
    if (curr_step != printed_step)
    {
        printf("***** Step %u\n", curr_step);
        printed_step = curr_step;
    }
    return;
}

static void pr_euc(void)
{
    if (curr_step > step_upto || curr_step < step_from)
        return;
    if (euc_step != curr_step)
    {
        euc_step = curr_step;
        if (res_step != curr_step - 1 || res_prevk != prevk ||
            res_nextd != nextd)
        {
            pr_step();
            print_expression_m("      Leading term : ", prevk, nextd, true);
        }
    }
    return;
}

static void pr_imres(void)
{
    if (curr_step > step_upto || curr_step < step_from)
        return;
    pr_step();
    print_expression_m("      Result : ", prevk, nextd, true);
    res_step = curr_step;
    res_prevk = prevk;
    res_nextd = nextd;
    return;
}

static void pr_finres(uint32_t xstep, const T_LINKCB *xprevk, const T_LINKCB *xnextd)
{
    if (curr_step > step_upto || curr_step < step_from)
        return;
    pr_step();
    if (curr_step == res_step && res_prevk == xprevk &&
        res_nextd == xnextd)
    {
        if (xstep == curr_step)
            return;
        printf("----- This is result of call on step %u\n", xstep);
    }
    else
    {
        if (xstep == curr_step)
        {
            pr_imres();
            return;
        }
        printf("----- Result of call on step %u :\n", xstep);
        print_expression_m("     ", xprevk, xnextd, true);
        res_step = curr_step;
        res_prevk = xprevk;
        res_nextd = xnextd;
    }
    return;
}

static void getpf(const T_STATUS_TABLE *status_table)
{
    curr_step = status_table->step + 1;
    pk = status_table->dot->info.codep;
    prevk = pk->previous;
    nextd = status_table->dot->next;
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
    current_determination = last_determination;
    if (strlen(buff_id) == 0)
        current_determination = NULL;
    else
        while (current_determination != NULL)
        {
            for (i = 0; buff_id[i] != '\0'; i++)
                if (*(current_determination->identifier + i) != buff_id[i])
                    break;
            if (buff_id[i] == '\0' && *(current_determination->identifier + i) == '\0')
                break;
            current_determination = current_determination->next;
        }
    if (current_determination == NULL)
        current_determination = &dummy_determination;
    return;
}

static char *fgetline(FILE *file, size_t *line_length)
{
    static char *buffer = NULL;
    static size_t buffer_size = 0;
    static size_t buffer_length = 0;
    if (buffer == NULL)
    {
        buffer_size = 128;
        if ((buffer = malloc(buffer_size)) == NULL)
            return NULL;
    }
    buffer_length = 0;
    int ch = fgetc(file);
    while (ch != EOF)
    {
        if (buffer_length >= buffer_size)
        {
            size_t new_buffer_size = buffer_size + 128;
            char *new_buffer = realloc(buffer, new_buffer_size);
            if (new_buffer == NULL)
            {
                int temp_errno = errno;
                free(buffer);
                errno = temp_errno;
                buffer = NULL;
                return NULL;
            }
            buffer = new_buffer;
            buffer_size = new_buffer_size;
        }
        buffer[buffer_length++] = ch;
        if (ch == '\n')
            break;
        ch = fgetc(file);
    }
    *line_length = buffer_length;
    return buffer_length == 0 ? NULL : buffer;
}

static void get_arg(void)
{
    for (l_arg = 0;; l_arg++)
    {
        *(parameter + l_arg) = (char)toupper(*(parameter + l_arg));
        if (*(parameter + l_arg) == '\n' || *(parameter + l_arg) == ' ' || *(parameter + l_arg) == '\0' || *(parameter + l_arg) == ',')
            break;
    }
    for (s_arg = 0; *(parameter + l_arg + s_arg) == ' ' || *(parameter + l_arg + s_arg) == ','; s_arg++)
        ;
    return;
}

static bool get_det(void)
{
    current_determination = last_determination;
    while (current_determination != NULL)
    {
        if (strncmp(current_determination->identifier, parameter, l_arg) == 0)
            if (*(current_determination->identifier + l_arg) == '\0')
                return true;
        current_determination = current_determination->next;
    }
    bool ab = false;
    current_determination = (T_DETERMINATION *)malloc(sizeof(T_DETERMINATION));
    if (current_determination == NULL)
        ab = true;
    else
    {
        current_determination->identifier = malloc(l_arg + 1);
        if (current_determination->identifier == NULL)
            ab = true;
    }
    if (ab)
    {
        printf("\nRefalAB debugger: no storage\n");
        exit(1);
        return false;
    }
    strncpy(current_determination->identifier, parameter, l_arg);
    *(current_determination->identifier + l_arg) = '\0';
    printf("%s ", current_determination->identifier);
    current_determination->next = last_determination;
    last_determination = current_determination;
    current_determination->ge = false;
    current_determination->gt = false;
    current_determination->eq = false;
    current_determination->ne = false;
    current_determination->le = false;
    current_determination->lt = false;
    current_determination->tr = false;
    return true;
}

static bool get_numb(int32_t *numb)
{
    if (sscanf(parameters, "%d", numb) == 0 || *numb < 1)
    {
        printf("\n                        Invalid number; repeat please.\n");
        return false;
    }
    return true;
}

static bool get_yn(const char *b)
{
    if (*b != 'y' && *b != 'n')
    {
        printf("\n                        Answer is \"y/n\"; repeat please.\n");
        return false;
    }
    if (*b == 'y')
        e_empty = true;
    return true;
}

//-----------  end of file D.C -------------
