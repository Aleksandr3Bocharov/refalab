// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-04-23
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
#include <errno.h>
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
static char *parameters = NULL;
static size_t parameter_delimiters;
static size_t parameter_length;
static char *parameter = NULL;
static char identifier[256];
static uint32_t printed_step;
static uint32_t current_step;
static uint32_t leading_term_step;
static uint32_t result_step;
static T_LINKCB *k;
static const T_LINKCB *next_k;
static T_LINKCB *previous_k;
static T_LINKCB *next_dot;
static const T_LINKCB *result_previous_k;
static const T_LINKCB *result_next_dot;
static uint32_t current_step_ge;
static T_LINKCB *dot_ge;
static const T_LINKCB *previous_k_ge, *next_dot_ge;
static uint32_t current_step_le;
static T_LINKCB *dot_le;
static const T_LINKCB *previous_k_le, *next_dot_le;

static void init_determinations_flags(void);
static char *card(void);
static void get_parameter(void);
static bool get_determination(void);
static bool get_number(int32_t *number);
static bool get_yes_no(const char *answer);
static void debugger_status_table(T_STATUS_TABLE *status_table);
static void get_identifier(const T_STATUS_TABLE *status_table);
static void one_step(T_STATUS_TABLE *status_table);
static void print_leading_term(void);
static void print_final_result(uint32_t x_step, const T_LINKCB *x_previous_k, const T_LINKCB *x_next_dot);
static void print_immediate_result(void);
static void print_step(void);

void (*status_table_debugger)(T_STATUS_TABLE *) = NULL;

void refal_debugger(T_STATUS_TABLE *status_table)
{
    // read task for debugging
    init_determinations_flags();
    //----------------------------------
    printf("\n ***** RefalAB debugger ***** \n");
    //----------------------------------
    printf("\n > (function list) : ");
    parameters = card();
    if (parameters == NULL)
    {
        printf("\nRefalAB debugger: no memory or EOF in stdin\n");
        exit(1);
    }
    size_t i;
    for (i = 0; *(parameters + i) == ' ' || *(parameters + i) == ','; i++)
        ;
    if (*(parameters + i) != '\0')
    {
        parameter = parameters + i;
        trace_condition = true;
        ge_all = false;
        while (*parameter != '\0')
        {
            get_parameter();
            get_determination();
            current_determination->gt = true;
            parameter += parameter_length + parameter_delimiters;
        }
    }
    //----------------------------------
    printf("\n >= (function list) : ");
    parameters = card();
    if (parameters == NULL)
    {
        printf("\nRefalAB debugger: no memory or EOF in stdin\n");
        exit(1);
    }
    for (i = 0; *(parameters + i) == ' ' || *(parameters + i) == ','; i++)
        ;
    if (*(parameters + i) != '\0')
    {
        parameter = parameters + i;
        trace_condition = true;
        ge_all = false;
        while (*parameter != '\0')
        {
            get_parameter();
            get_determination();
            current_determination->ge = true;
            parameter += parameter_length + parameter_delimiters;
        }
    }
    //----------------------------------
    printf("\n = (function list) : ");
    parameters = card();
    if (parameters == NULL)
    {
        printf("\nRefalAB debugger: no memory or EOF in stdin\n");
        exit(1);
    }
    for (i = 0; *(parameters + i) == ' ' || *(parameters + i) == ','; i++)
        ;
    if (*(parameters + i) != '\0')
    {
        parameter = parameters + i;
        trace_condition = true;
        eq_all = false;
        while (*parameter != '\0')
        {
            get_parameter();
            get_determination();
            current_determination->eq = true;
            parameter += parameter_length + parameter_delimiters;
        }
    }
    //----------------------------------
    printf("\n != (function list) : ");
    parameters = card();
    if (parameters == NULL)
    {
        printf("\nRefalAB debugger: no memory or EOF in stdin\n");
        exit(1);
    }
    for (i = 0; *(parameters + i) == ' ' || *(parameters + i) == ','; i++)
        ;
    if (*(parameters + i) != '\0')
    {
        parameter = parameters + i;
        trace_condition = true;
        while (*parameter != '\0')
        {
            get_parameter();
            get_determination();
            current_determination->ne = true;
            parameter += parameter_length + parameter_delimiters;
        }
    }
    //----------------------------------
    printf("\n < (function list) : ");
    parameters = card();
    if (parameters == NULL)
    {
        printf("\nRefalAB debugger: no memory or EOF in stdin\n");
        exit(1);
    }
    for (i = 0; *(parameters + i) == ' ' || *(parameters + i) == ','; i++)
        ;
    if (*(parameters + i) != '\0')
    {
        parameter = parameters + i;
        trace_condition = true;
        while (*parameter != '\0')
        {
            get_parameter();
            get_determination();
            current_determination->lt = true;
            parameter += parameter_length + parameter_delimiters;
        }
    }
    //----------------------------------
    printf("\n <= (function list) : ");
    parameters = card();
    if (parameters == NULL)
    {
        printf("\nRefalAB debugger: no memory or EOF in stdin\n");
        exit(1);
    }
    for (i = 0; *(parameters + i) == ' ' || *(parameters + i) == ','; i++)
        ;
    if (*(parameters + i) != '\0')
    {
        parameter = parameters + i;
        trace_condition = true;
        while (*parameter != '\0')
        {
            get_parameter();
            get_determination();
            current_determination->le = true;
            parameter += parameter_length + parameter_delimiters;
        }
    }
    //----------------------------------
    printf("\n TRAP (function list) : ");
    parameters = card();
    if (parameters == NULL)
    {
        printf("\nRefalAB debugger: no memory or EOF in stdin\n");
        exit(1);
    }
    for (i = 0; *(parameters + i) == ' ' || *(parameters + i) == ','; i++)
        ;
    if (*(parameters + i) != '\0')
    {
        parameter = parameters + i;
        while (*parameter != '\0')
        {
            get_parameter();
            get_determination();
            current_determination->tr = true;
            parameter += parameter_length + parameter_delimiters;
        }
    }
    //----------------------------------
    while (true)
    {
        printf("\n STOP (step number) : ");
        parameters = card();
        if (parameters == NULL)
        {
            printf("\nRefalAB debugger: no memory or EOF in stdin\n");
            exit(1);
        }
        for (i = 0; *(parameters + i) == ' '; i++)
            ;
        if (*(parameters + i) != '\0')
            if (!get_number((int32_t *)&step_stop))
            {
                printf("\n                        Invalid number; repeat please.\n");
                continue;
            }
        break;
    }
    //----------------------------------
    while (true)
    {
        printf("\n FROM (step number) : ");
        parameters = card();
        if (parameters == NULL)
        {
            printf("\nRefalAB debugger: no memory or EOF in stdin\n");
            exit(1);
        }
        for (i = 0; *(parameters + i) == ' '; i++)
            ;
        if (*(parameters + i) != '\0')
            if (!get_number((int32_t *)&step_from))
            {
                printf("\n                        Invalid number; repeat please.\n");
                continue;
            }
        break;
    }
    //----------------------------------
    while (true)
    {
        printf("\n TO (step number) : ");
        parameters = card();
        if (parameters == NULL)
        {
            printf("\nRefalAB debugger: no memory or EOF in stdin\n");
            exit(1);
        }
        for (i = 0; *(parameters + i) == ' '; i++)
            ;
        if (*(parameters + i) != '\0')
            if (!get_number((int32_t *)&step_upto))
            {
                printf("\n                        Invalid number; repeat please.\n");
                continue;
            }
        break;
    }
    //----------------------------------
    while (true)
    {
        printf("\n E._= (y/n) : ");
        parameters = card();
        if (parameters == NULL)
        {
            printf("\nRefalAB debugger: no memory or EOF in stdin\n");
            exit(1);
        }
        for (i = 0; *(parameters + i) == ' '; i++)
            ;
        if (*(parameters + i) != '\0')
            if (!get_yes_no(parameters + i))
            {
                printf("\n                        Answer is \"y/n\"; repeat please.\n");
                continue;
            }
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
    status_table_debugger = debugger_status_table;
    printed_step = 0;
    leading_term_step = 0;
    result_step = 0;
    result_next_dot = NULL;
    result_previous_k = result_next_dot;
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
            get_identifier(status_table);
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
                    print_leading_term();
            }
            //  cut
            current_step_ge = current_step;
            previous_k_ge = previous_k;
            next_dot_ge = next_dot;
            if (k->info.code == NULL)
                dot_ge = NULL;
            else
            {
                dot_ge = k->info.codep;
                k->info.code = NULL;
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
                    print_leading_term();
                }
                //   cut
                current_step_le = current_step;
                previous_k_le = previous_k;
                next_dot_le = next_dot;
                if (k->info.code == NULL)
                    dot_le = NULL;
                else
                {
                    dot_le = k->info.codep;
                    k->info.code = NULL;
                }
                // compute call entirely
                bool next_state = false;
                while (status_table->dot != NULL)
                {
                    get_identifier(status_table);
                    if (current_determination->tr)
                    {
                        debugger_state = DBG_TRAP;
                        next_state = true;
                        break;
                    }
                    one_step(status_table);
                    if (step_stop < status_table->step)
                    {
                        debugger_state = DBG_ABEND;
                        next_state = true;
                        break;
                    }
                    current_step = status_table->step + 1;
                    if (status_table->state != 1)
                    {
                        debugger_state = DBG_ABEND;
                        next_state = true;
                        break;
                    }
                }
                if (next_state)
                    break;
                //  joint
                current_step = status_table->step;
                status_table->dot = dot_le;
                if (was_le)
                    print_final_result(current_step_le, previous_k_le, next_dot_le);
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
                    print_leading_term();
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
                    print_immediate_result();
            }
            if (status_table->dot != NULL)
            {
                get_identifier(status_table);
                debugger_state = DBG_ALREADY;
                break;
            }
            //  joint
            status_table->dot = dot_ge;
            if (!ge_all && was_ge)
                print_final_result(current_step_ge, previous_k_ge, next_dot_ge);
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
            get_identifier(status_table);
            debugger_state = DBG_ABEND1;
            break;
        case DBG_ABEND1:
            printf("Leading functional term:\n");
            print_expression_m("     ", previous_k, next_dot, true);
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
                long int nanoseconds = refal.timer.stop_time.tv_nsec - refal.timer.start_time.tv_nsec;
                long long int seconds = (long long int)difftime(refal.timer.stop_time.tv_sec, refal.timer.start_time.tv_sec);
                if (nanoseconds < 0)
                {
                    nanoseconds += 1000000000;
                    seconds--;
                }
                long long int minutes = seconds / 60;
                seconds %= 60;
                const long long int hours = minutes / 60;
                minutes %= 60;
                char string_time[64];
                sprintf(string_time, "%02lld:%02lld:%02lld.%09ld", hours, minutes, seconds, nanoseconds);
                printf("Elapsed time = %s\n", string_time);
            }
            delete_status_table(status_table);
            refal_terminate_memory();
            exit(0);
            return;
        }
}

static void debugger_status_table(T_STATUS_TABLE *status_table)
{
    T_LINKCB *save_previous_k = previous_k;
    T_LINKCB *save_next_dot = next_dot;
    T_LINKCB *save_k = k;
    const T_LINKCB *save_next_k = next_k;
    uint32_t save_result_step = result_step;
    const T_LINKCB *save_result_previous_k = result_previous_k;
    const T_LINKCB *save_result_next_dot = result_next_dot;
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
            get_identifier(status_table);
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
                    print_leading_term();
            }
            //  cut
            current_step_ge = current_step;
            previous_k_ge = previous_k;
            next_dot_ge = next_dot;
            if (k->info.code == NULL)
                dot_ge = NULL;
            else
            {
                dot_ge = k->info.codep;
                k->info.code = NULL;
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
                    print_leading_term();
                }
                //   cut
                current_step_le = current_step;
                previous_k_le = previous_k;
                next_dot_le = next_dot;
                if (k->info.code == NULL)
                    dot_le = NULL;
                else
                {
                    dot_le = k->info.codep;
                    k->info.code = NULL;
                }
                // compute call entirely
                bool next_state = false;
                while (status_table->dot != NULL)
                {
                    get_identifier(status_table);
                    if (current_determination->tr)
                    {
                        status_table_debugger_state = DB_TRAP;
                        next_state = true;
                        break;
                    }
                    one_step(status_table);
                    if (step_stop < status_table->step)
                    {
                        status_table_debugger_state = DB_ABEND;
                        next_state = true;
                        break;
                    }
                    current_step = status_table->step + 1;
                    if (status_table->state != 1)
                    {
                        status_table_debugger_state = DB_AB;
                        next_state = true;
                        break;
                    }
                }
                if (next_state)
                    break;
                //  joint
                current_step = status_table->step;
                status_table->dot = dot_le;
                if (was_le)
                    print_final_result(current_step_le, previous_k_le, next_dot_le);
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
                    print_leading_term();
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
                    print_immediate_result();
            }
            if (status_table->dot != NULL)
            {
                get_identifier(status_table);
                status_table_debugger_state = DB_ALREADY;
                break;
            }
            //  joint
            status_table->dot = dot_ge;
            if (!ge_all && was_ge)
                print_final_result(current_step_ge, previous_k_ge, next_dot_ge);
            status_table_debugger_state = DB_NOT_YET;
            break;
        case DB_TRAP:
            printf("Function name trap\n");
            status_table_debugger_state = DB_ABEND;
            break;
        case DB_ABEND:
            printf("Leading functional term:\n");
            print_expression_m("     ", previous_k, next_dot, true);
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
                long int nanoseconds = refal.timer.stop_time.tv_nsec - refal.timer.start_time.tv_nsec;
                long long int seconds = (long long int)difftime(refal.timer.stop_time.tv_sec, refal.timer.start_time.tv_sec);
                if (nanoseconds < 0)
                {
                    nanoseconds += 1000000000;
                    seconds--;
                }
                long long int minutes = seconds / 60;
                seconds %= 60;
                const long long int hours = minutes / 60;
                minutes %= 60;
                char string_time[64];
                sprintf(string_time, "%02lld:%02lld:%02lld.%09ld", hours, minutes, seconds, nanoseconds);
                printf("Elapsed time = %s\n", string_time);
            }
            exit(0);
            return;
        case DB_DO:
        case DB_AB:
            previous_k = save_previous_k;
            next_dot = save_next_dot;
            k = save_k;
            next_k = save_next_k;
            result_step = save_result_step;
            result_previous_k = save_result_previous_k;
            result_next_dot = save_result_next_dot;
            return;
        }
}

//    procedures
static void init_determinations_flags(void)
{
    for (T_DETERMINATION *determination = last_determination; determination != NULL;)
    {
        free(determination->identifier);
        T_DETERMINATION *temp_determination = determination;
        determination = determination->next;
        free(temp_determination);
    }
    last_determination = NULL;
    return;
}

static void one_step(T_STATUS_TABLE *status_table)
{
    status_table->stop = status_table->step + 1;
    while (true)
    {
        refal_run(status_table);
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
        print_step();
        if (leading_term_step != current_step)
        {
            leading_term_step = current_step;
            print_expression_m("       Leading term : ", previous_k, next_dot, true);
        }
        printf("*** Recognition impossible\n");
        printf("*** Change leading term by empty term and continue ***\n");
        status_table->dot = k->info.codep;
        insert_to_free_memory_list(previous_k, next_dot);
        status_table->state = 1;
        status_table->step++;
    };
    return;
}

static void print_step(void)
{
    if (current_step != printed_step)
    {
        printf("***** Step %u\n", current_step);
        printed_step = current_step;
    }
    return;
}

static void print_leading_term(void)
{
    if (current_step > step_upto || current_step < step_from)
        return;
    if (leading_term_step != current_step)
    {
        leading_term_step = current_step;
        if (result_step != current_step - 1 || result_previous_k != previous_k ||
            result_next_dot != next_dot)
        {
            print_step();
            print_expression_m("      Leading term : ", previous_k, next_dot, true);
        }
    }
    return;
}

static void print_immediate_result(void)
{
    if (current_step > step_upto || current_step < step_from)
        return;
    print_step();
    print_expression_m("      Result : ", previous_k, next_dot, true);
    result_step = current_step;
    result_previous_k = previous_k;
    result_next_dot = next_dot;
    return;
}

static void print_final_result(uint32_t x_step, const T_LINKCB *x_previous_k, const T_LINKCB *x_next_dot)
{
    if (current_step > step_upto || current_step < step_from)
        return;
    print_step();
    if (current_step == result_step && result_previous_k == x_previous_k &&
        result_next_dot == x_next_dot)
    {
        if (x_step == current_step)
            return;
        printf("----- This is result of call on step %u\n", x_step);
    }
    else
    {
        if (x_step == current_step)
        {
            print_immediate_result();
            return;
        }
        printf("----- Result of call on step %u :\n", x_step);
        print_expression_m("     ", x_previous_k, x_next_dot, true);
        result_step = current_step;
        result_previous_k = x_previous_k;
        result_next_dot = x_next_dot;
    }
    return;
}

static void get_identifier(const T_STATUS_TABLE *status_table)
{
    current_step = status_table->step + 1;
    k = status_table->dot->info.codep;
    previous_k = k->previous;
    next_dot = status_table->dot->next;
    next_k = k->next;
    if (next_k->tag != TAGF)
    {
        identifier[0] = '%';
        identifier[1] = '\0';
    }
    else
    {
        const uint8_t *label_length = next_k->info.codef - 1;
        const char *label = (char *)label_length - *label_length;
        for (uint8_t i = 0; i < *label_length; i++)
            identifier[i] = (char)toupper(*(label + i));
        identifier[*label_length] = '\0';
    }
    current_determination = last_determination;
    if (strlen(identifier) == 0)
        current_determination = NULL;
    else
        while (current_determination != NULL)
        {
            if (strcmp(identifier, current_determination->identifier) == 0)
                break;
            current_determination = current_determination->next;
        }
    if (current_determination == NULL)
        current_determination = &dummy_determination;
    return;
}

static char *card(void)
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
    int ch = getchar();
    while (ch != EOF)
    {
        if (buffer_length + 1 == buffer_size)
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
        if (ch < ' ' && ch > '\0')
            buffer[buffer_length++] = ' ';
        else
            buffer[buffer_length++] = (char)ch;
        if (ch == '\n')
            break;
        ch = getchar();
    }
    buffer[buffer_length] = '\0';
    return buffer_length == 0 ? NULL : buffer;
}

static void get_parameter(void)
{
    for (parameter_length = 0;; parameter_length++)
    {
        *(parameter + parameter_length) = (char)toupper(*(parameter + parameter_length));
        if (*(parameter + parameter_length) == '\0' || *(parameter + parameter_length) == ' ' || *(parameter + parameter_length) == ',')
            break;
    }
    for (parameter_delimiters = 0; *(parameter + parameter_length + parameter_delimiters) == ' ' || *(parameter + parameter_length + parameter_delimiters) == ','; parameter_delimiters++)
        ;
    return;
}

static bool get_determination(void)
{
    current_determination = last_determination;
    while (current_determination != NULL)
    {
        if (strncmp(current_determination->identifier, parameter, parameter_length) == 0)
            if (*(current_determination->identifier + parameter_length) == '\0')
                return true;
        current_determination = current_determination->next;
    }
    bool no_memory = false;
    current_determination = (T_DETERMINATION *)malloc(sizeof(T_DETERMINATION));
    if (current_determination == NULL)
        no_memory = true;
    else
    {
        current_determination->identifier = malloc(parameter_length + 1);
        if (current_determination->identifier == NULL)
            no_memory = true;
    }
    if (no_memory)
    {
        printf("\nRefalAB debugger: no memory\n");
        exit(1);
        return false;
    }
    strncpy(current_determination->identifier, parameter, parameter_length);
    *(current_determination->identifier + parameter_length) = '\0';
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

static bool get_number(int32_t *number)
{
    if (sscanf(parameters, "%d", number) == 0 || *number < 1)
        return false;
    return true;
}

static bool get_yes_no(const char *answer)
{
    if (*answer != 'y' && *answer != 'n')
        return false;
    if (*answer == 'y')
        e_empty = true;
    return true;
}

//-----------  end of file D.C -------------
