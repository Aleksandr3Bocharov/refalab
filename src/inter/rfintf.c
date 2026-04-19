// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-04-18
// https://github.com/Aleksandr3Bocharov/refalab

//-----------  file  --  RFINTF.C ------------------
//           C-interface functions
//--------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "refalab.h"
#include "rfintf.h"
#include "rfrun.h"

extern uint8_t refalab_feof, refalab_ferror;

T_REFAL refal;

static struct
{
    bool timer_on;
} options = {true};

static size_t gargc = 0;
static char **gargv = NULL;

static T_LINKCB *last_block_free_memory = NULL;
static bool refal_init = true;
static T_LINKCB free_memory_list_head;

static bool collect_garbage(void);
static void add_free_memory_list(T_LINKCB *block_free_memory, size_t size_block_free_memory);

void refal_get_args(int argc, char *argv[])
{
    gargc = (size_t)argc;
    gargv = argv;
    for (size_t i = 1; i < gargc; i++)
        if (strncmp(gargv[i], "--rfinteropt", 12) == 0)
        {
            if (strstr(&gargv[i][12], "-timer_off") != NULL)
                options.timer_on = false;
            break;
        }
    return;
}

void refal_abort_end(const char *abort_message)
{
    printf(" *** refal-abort-end *** %s\n", abort_message);
    exit(1);
    return;
}

bool more_free_memory(void)
{
    size_t collected_garbage_count = 0;
    if (last_block_free_memory != NULL)
    {
        const T_LINKCB *first_linkcb_free_memory = refal.free_memory_list_head->next;
        const bool was_collected_garbage = collect_garbage();
        if (was_collected_garbage)
        {
            const T_LINKCB *linkcb_free_memory = refal.free_memory_list_head->next;
            while (linkcb_free_memory != first_linkcb_free_memory && collected_garbage_count != 1000)
            {
                collected_garbage_count++;
                linkcb_free_memory = linkcb_free_memory->next;
            }
            if (collected_garbage_count == 1000)
                return true;
        }
    }
    T_LINKCB *new_block_free_memory = malloc(1001 * sizeof(T_LINKCB));
#if defined mdebug
    fprintf(stderr, "more_free_memory: collected_garbage_count=%zu after new_block_free_memory=%p\n", collected_garbage_count, (void *)new_block_free_memory);
#endif
    if (new_block_free_memory == NULL)
        return false;
    new_block_free_memory->previous = last_block_free_memory;
    last_block_free_memory = new_block_free_memory;
    add_free_memory_list(new_block_free_memory + 1, 1000);
    return true;
}

//  check a number of items in free items list
bool check_count_free_memory_list(size_t count)
{
    const T_LINKCB *linkcb_free_memory = refal.free_memory_list_head;
    for (size_t i = 0; i < count; i++)
    {
        linkcb_free_memory = linkcb_free_memory->next;
        if (linkcb_free_memory == refal.free_memory_list_head)
            return false;
    }
    return true;
}

bool insert_from_free_memory_list(T_LINKCB *where, size_t count)
{
    if (count == 0)
        return true;
    T_LINKCB *linkcb_free_memory = refal.free_memory_list_head;
    for (size_t i = 0; i < count; i++)
    {
        linkcb_free_memory = linkcb_free_memory->next;
        if (linkcb_free_memory == refal.free_memory_list_head)
            return false;
        linkcb_free_memory->tag = TAGO;
        linkcb_free_memory->info.code = NULL;
    }
    T_LINKCB *next_linkcb_free_memory = linkcb_free_memory->next;
    T_LINKCB *first_linkcb_free_memory = refal.free_memory_list_head->next;
    refal.free_memory_list_head->next = next_linkcb_free_memory;
    next_linkcb_free_memory->previous = refal.free_memory_list_head;
    T_LINKCB *next_where = where->next;
    linkcb_free_memory->next = next_where;
    next_where->previous = linkcb_free_memory;
    where->next = first_linkcb_free_memory;
    first_linkcb_free_memory->previous = where;
    return true;
}

bool extended_insert_from_free_memory_list(T_LINKCB *where, size_t count)
{
    while (!check_count_free_memory_list(count))
        if (!more_free_memory())
        {
            refal.upshot = 3;
            return false;
        }
    return insert_from_free_memory_list(where, count);
}

bool insert_view_k_function_dot(T_STATUS_TABLE *status_table, uint8_t *refalab_function)
{
    if (!exist_status_table(status_table))
        refal_abort_end("insert_view_k_function_dot: process doesn't exist still");
    if (status_table->dot != NULL)
        refal_abort_end("insert_view_k_function_dot: there are '<'-signes in view field");
    if (!extended_insert_from_free_memory_list(status_table->view, 3))
        return false;
    T_LINKCB *k = status_table->view->next;
    T_LINKCB *symbol_label = k->next;
    if (symbol_label->next != status_table->view->previous)
        transplantation(status_table->view->previous, symbol_label, symbol_label->next->next);
    T_LINKCB *dot = status_table->view->previous;
    k->tag = TAGK;
    dot->tag = TAGD;
    dot->info.codep = k;
    symbol_label->tag = TAGF;
    symbol_label->info.codef = refalab_function;
    status_table->dot = dot;
    return true;
}

void refal_initiate(void)
{
    refal_init = false;
    refal.last_status_table = (T_STATUS_TABLE *)&refal;
    refal.first_status_table = (T_STATUS_TABLE *)&refal;
    refal.upshot = 1;
    refal.current_status_table = NULL;
    refal.static_boxes = NULL;
    refal.dynamic_boxes = NULL;
    refal.free_memory_list_head = &free_memory_list_head;
    refal.free_memory_list_head->previous = &free_memory_list_head;
    refal.free_memory_list_head->next = &free_memory_list_head;
    refal.free_memory_list_head->tag = TAGO;
    refal.free_memory_list_head->info.code = NULL;
    refal.arg.argc = gargc;
    refal.arg.argv = gargv;
    refal.timer.mode = options.timer_on;
    if (refal.timer.mode)
        timespec_get(&refal.timer.start_time, TIME_UTC);
    return;
}

void delete_status_table(const T_STATUS_TABLE *status_table)
{
    if (refal_init)
        refal_initiate();
    if (!exist_status_table(status_table))
        refal_abort_end("delete_status_table: process doesn't exist");
    if (status_table->state == 4)
        refal_abort_end("delete_status_table: process is in job yet");
    status_table->previous->next = status_table->next;
    status_table->next->previous = status_table->previous;
    T_LINKCB *last_linkcb_free_memory = refal.free_memory_list_head->previous;
    T_LINKCB *last_view = status_table->view->previous;
    T_LINKCB *last_store = status_table->store->previous;
    last_linkcb_free_memory->next = status_table->view;
    status_table->view->previous = last_linkcb_free_memory;
    last_view->next = status_table->store;
    status_table->store->previous = last_view;
    last_store->next = refal.free_memory_list_head;
    refal.free_memory_list_head->previous = last_store;
    return;
}

//    delete part of list and add it to free memory list
void insert_to_free_memory_list(T_LINKCB *before, T_LINKCB *after)
{
    T_LINKCB *first = before->next;
    if (first == after)
        return;
    T_LINKCB *last = after->previous;
    T_LINKCB *where = refal.free_memory_list_head->previous;
    before->next = after;
    after->previous = before;
    last->next = refal.free_memory_list_head;
    refal.free_memory_list_head->previous = last;
    where->next = first;
    first->previous = where;
    return;
}

void refal_terminate_memory(void)
{
    while (last_block_free_memory != NULL)
    {
        T_LINKCB *delete_block_free_memory = last_block_free_memory;
        last_block_free_memory = delete_block_free_memory->previous;
#if defined mdebug
        fprintf(stderr, "refal_terminate_memory: free delete_block_free_memory=%p\n", (void *)delete_block_free_memory);
#endif
        free(delete_block_free_memory);
    }
}

void refal_execute(uint8_t *refalab_function)
{
    T_STATUS_TABLE status_table;
    if (refal_init)
        refal_initiate();
    bool lack_memory = false;
    if (!more_free_memory())
        lack_memory = true;
    else if (!create_status_table(&status_table))
        lack_memory = true;
    else if (!insert_view_k_function_dot(&status_table, refalab_function))
        lack_memory = true;
    if (lack_memory)
    {
        printf("\nNo enough memory for initialization\n");
        refal_terminate_memory();
        return;
    }
    status_table.stop = MAX_STOP;
#if defined mdebug
    const uint32_t status_table_stop = status_table.stop;
#endif
    enum
    {
        AGAIN,
        DONE,
        ABEND,
        EOJ,
        RET
    } execute_state = AGAIN;
    while (true)
        switch (execute_state)
        {
        case AGAIN:
#if defined mdebug
            //	step by step execution with full debug trace information
            if (status_table.dot == NULL)
            {
                execute_state = DONE;
                break;
            }
            if (status_table.state != 1 || status_table.step >= status_table_stop)
            {
                bool abort_end = true;
                if (status_table.state == 3)
                    if (more_free_memory())
                        abort_end = false;
                if (status_table.step >= status_table_stop)
                {
                    status_table.step = 0;
                    abort_end = false;
                }
                if (abort_end)
                {
                    execute_state = ABEND;
                    break;
                }
            }
            status_table.stop = status_table.step + 1;
            const T_LINKCB *k = status_table.dot->info.codep;
            const T_LINKCB *previous_k = k->previous;
            const T_LINKCB *next_dot = status_table.dot->next;
            printf(" Step: %d\n", status_table.stop);
            print_expression_m(" Term: ", previous_k, next_dot, true);
            rfrun(&status_table);
            if (status_table.state == 1)
                print_expression_m(" Result: ", previous_k, next_dot, true);
            break;
#else
            // no debug info
            rfrun(&status_table);
            if (status_table.state == 3)
                if (more_free_memory())
                    break;
            if (status_table.state == 1 && status_table.dot != NULL)
            {
                status_table.step = 0;
                break;
            }
            if (status_table.dot != NULL)
            {
                execute_state = ABEND;
                break;
            }
            execute_state = DONE;
            break;
#endif
        case DONE:
#if defined mdebug
            printf("Concretization is executed\n");
            execute_state = EOJ;
#else
            execute_state = RET;
#endif
            break;
        case ABEND:
            switch (status_table.state)
            {
            case 2:
                printf("Recognition impossible\n");
                break;
            case 3:
                printf("Free memory exhausted\n");
            }
            execute_state = EOJ;
            break;
        case EOJ:
            printf("Total steps number = %u\n", status_table.step);
            if (status_table.view->next != status_table.view)
            {
                printf("View field:\n");
                print_expression_m("            ", status_table.view, status_table.view, true);
            }
            if (status_table.store->next != status_table.store)
            {
                printf("Burried:\n");
                print_expression_m("         ", status_table.store, status_table.store, true);
            }
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
            execute_state = RET;
            break;
        case RET:
            delete_status_table(&status_table);
            refal_terminate_memory();
            return;
        }
}

void print_expression(const char *begin_string, const T_LINKCB *before, const T_LINKCB *after, const bool new_line)
{
    printf("%s", begin_string);
    const T_LINKCB *print = before;
    while (print != after->previous)
    {
        const T_LINKCB *temp_print = print;
        print = print->next;
        if (temp_print != print->previous)
            refal_abort_end("print_expression: list structure violation");
        if (print->tag == TAGO)
            putchar(print->info.infoc);
        else if (print->tag == TAGK)
            putchar('<');
        else if (print->tag == TAGD)
            putchar('>');
        else if (print->tag == TAGLB)
            putchar('(');
        else if (print->tag == TAGRB)
            putchar(')');
        else if (print->tag == TAGN)
            printf("'%u'", gcoden(print));
        else if (print->tag == TAGF)
        {
            putchar('\'');
            const uint8_t *label_length = print->info.codef - 1;
            const char *label = (char *)label_length - *label_length;
            for (uint8_t i = 1; i <= *label_length; i++, label++)
                putchar(toupper(*label));
            putchar('\'');
        }
        else if (print->tag == TAGR)
            printf("'%%%p'", (void *)print->info.codep);
        else if (BRACKET(print))
            refal_abort_end("print_expression: unknown bracket type");
        else
            printf("'%x,%p'", print->tag, print->info.code);
    }
    if (new_line)
        printf("\n");
    return;
}

void print_expression_s(const char *begin_string, const T_LINKCB *before, const T_LINKCB *after, const bool new_line)
{
    printf("%s", begin_string);
    const T_LINKCB *prints = before;
    while (prints != after->previous)
    {
        const T_LINKCB *temp_prints = prints;
        prints = prints->next;
        if (temp_prints != prints->previous)
            refal_abort_end("print_expression_s: list structure violation");
        if (prints->tag == TAGO)
            putchar(prints->info.infoc);
        else if (prints->tag == TAGK)
            putchar('<');
        else if (prints->tag == TAGD)
            putchar('>');
        else if (prints->tag == TAGLB)
            putchar('(');
        else if (prints->tag == TAGRB)
            putchar(')');
        else if (prints->tag == TAGN)
            printf("%u", gcoden(prints));
        else if (prints->tag == TAGF)
        {
            const uint8_t *label_length = prints->info.codef - 1;
            const char *label = (char *)label_length - *label_length;
            for (uint8_t i = 1; i <= *label_length; i++, label++)
                putchar(toupper(*label));
        }
        else if (prints->tag == TAGR)
            printf("%%%p", (void *)prints->info.codep);
        else if (BRACKET(prints))
            refal_abort_end("print_expression_s: unknown bracket type");
        else
            printf("%x,%p", prints->tag, prints->info.code);
    }
    if (new_line)
        printf("\n");
    return;
}

void print_expression_m(const char *begin_string, const T_LINKCB *before, const T_LINKCB *after, const bool new_line)
{
    printf("%s", begin_string);
    const T_LINKCB *printm = before;
    bool tago = false;
    while (printm != after->previous)
    {
        const T_LINKCB *temp_printm = printm;
        printm = printm->next;
        if (temp_printm != printm->previous)
            refal_abort_end("print_expression_m: list structure violation");
        if (printm->tag == TAGO)
        {
            if (!tago)
            {
                tago = true;
                putchar('\'');
            };
            putchar(printm->info.infoc);
        }
        else
        {
            if (tago)
            {
                tago = false;
                putchar('\'');
            };
            if (printm->tag == TAGK)
                printf("< ");
            else if (printm->tag == TAGD)
                putchar('>');
            else if (printm->tag == TAGLB)
                putchar('(');
            else if (printm->tag == TAGRB)
                putchar(')');
            else if (printm->tag == TAGN)
            {
                printf("%u", gcoden(printm));
                if (printm->next->tag == TAGN)
                    putchar(' ');
            }
            else if (printm->tag == TAGF)
            {
                putchar('&');
                const uint8_t *label_length = printm->info.codef - 1;
                const char *label = (char *)label_length - *label_length;
                for (uint8_t i = 1; i <= *label_length; i++, label++)
                    putchar(toupper(*label));
                if (printm->next->tag == TAGN)
                    putchar(' ');
            }
            else if (printm->tag == TAGR)
                printf("/%%%p/", (void *)printm->info.codep);
            else if (BRACKET(printm))
                refal_abort_end("print_expression_m: unknown bracket type");
            else
                printf("/%x,%p/", printm->tag, printm->info.code);
        }
    }
    if (tago)
        putchar('\'');
    if (new_line)
        printf("\n");
    return;
}

void transplantation(T_LINKCB *where, T_LINKCB *before, T_LINKCB *after)
{
    T_LINKCB *first = before->next;
    if (first == after)
        return;
    T_LINKCB *next_where = where->next;
    T_LINKCB *last = after->previous;
    before->next = after;
    after->previous = before;
    last->next = next_where;
    next_where->previous = last;
    where->next = first;
    first->previous = where;
    return;
}

//  copy expression and add it to nessecary place
bool copy_expression(T_LINKCB *where, T_LINKCB *before, T_LINKCB *after)
{
    T_LINKCB *linkcb_free_memory = refal.free_memory_list_head;
    const T_LINKCB *current_linkcb = before->next;
    T_LINKCB *last_bracket = NULL;
    while (current_linkcb != after)
    {
        linkcb_free_memory = linkcb_free_memory->next;
        if (linkcb_free_memory == refal.free_memory_list_head)
            return false;
        switch (current_linkcb->tag)
        {
        case TAGLB:
            linkcb_free_memory->info.codep = last_bracket;
            last_bracket = linkcb_free_memory;
            break;
        case TAGRB:
            linkcb_free_memory->info.codep = last_bracket;
            linkcb_free_memory->tag = TAGRB;
            T_LINKCB *temp_last_bracket = last_bracket->info.codep;
            last_bracket->info.codep = linkcb_free_memory;
            last_bracket->tag = TAGLB;
            last_bracket = temp_last_bracket;
            break;
        default:
            linkcb_free_memory->tag = current_linkcb->tag;
            linkcb_free_memory->info.code = current_linkcb->info.code;
        }
        current_linkcb = current_linkcb->next;
    }
    if (refal.free_memory_list_head == linkcb_free_memory)
        return true;
    T_LINKCB *first_linkcb_free_memory = refal.free_memory_list_head->next;
    T_LINKCB *next_linkcb_free_memory = linkcb_free_memory->next;
    refal.free_memory_list_head->next = next_linkcb_free_memory;
    next_linkcb_free_memory->previous = refal.free_memory_list_head;
    T_LINKCB *next_where = where->next;
    linkcb_free_memory->next = next_where;
    next_where->previous = linkcb_free_memory;
    where->next = first_linkcb_free_memory;
    first_linkcb_free_memory->previous = where;
    return true;
}

bool exist_status_table(const T_STATUS_TABLE *status_table)
{
    const T_STATUS_TABLE *current_status_table = (T_STATUS_TABLE *)&refal;
    do
    {
        current_status_table = current_status_table->next;
        if (current_status_table == status_table)
            return true;
    } while (current_status_table != (T_STATUS_TABLE *)&refal);
    return false;
}

bool create_status_table(T_STATUS_TABLE *status_table)
{
    if (refal_init)
        refal_initiate();
    if (exist_status_table(status_table))
        refal_abort_end("create_status_table: process already exists");
    status_table->view = refal.free_memory_list_head->next;
    if (status_table->view == refal.free_memory_list_head)
        return false;
    status_table->store = status_table->view->next;
    if (status_table->store == refal.free_memory_list_head)
        return false;
    T_LINKCB *next_free_memory_list_head = status_table->store->next;
    refal.free_memory_list_head->next = next_free_memory_list_head;
    next_free_memory_list_head->previous = refal.free_memory_list_head;
    status_table->view->next = status_table->view;
    status_table->view->previous = status_table->view;
    status_table->store->next = status_table->store;
    status_table->store->previous = status_table->store;
    T_STATUS_TABLE *last_status_table = refal.last_status_table;
    status_table->next = (T_STATUS_TABLE *)&refal;
    refal.last_status_table = status_table;
    last_status_table->next = status_table;
    status_table->previous = last_status_table;
    status_table->state = 1;
    status_table->dot = NULL;
    status_table->step = 0;
    status_table->stop = MAX_STOP;
    return true;
}

static void mark_dynamic_box_heads(T_LINKCB *root)
{
    T_LINKCB *current_linkcb = root;
    T_LINKCB *head = current_linkcb;
    while (true)
    {
        T_LINKCB *dynamic_box_head;
        if (current_linkcb->next != head)
        {
            current_linkcb = current_linkcb->next;
            if (current_linkcb->tag != TAGR)
                continue;
            dynamic_box_head = current_linkcb->info.codep;
            if (dynamic_box_head->tag != TAGO)
                continue;
            dynamic_box_head->tag = 0xFFFF;
            current_linkcb->info.codep = head;
            dynamic_box_head->previous = current_linkcb;
            current_linkcb = dynamic_box_head;
            head = current_linkcb;
            continue;
        }
        if (head == root)
            return;
        T_LINKCB *symbol_reference = head->previous;
        head->previous = current_linkcb;
        dynamic_box_head = head;
        head = symbol_reference->info.codep;
        symbol_reference->info.codep = dynamic_box_head;
        symbol_reference->tag = TAGR;
        current_linkcb = symbol_reference;
    }
}

static bool collect_garbage(void)
{
    if (refal.dynamic_boxes == NULL)
        return false;
    // mark boxes achieved from view field & burriage
    const T_STATUS_TABLE *status_table = refal.first_status_table;
    while (status_table != (T_STATUS_TABLE *)&refal)
    {
        mark_dynamic_box_heads(status_table->view);
        mark_dynamic_box_heads(status_table->store);
        status_table = status_table->next;
    }
    // mark boxes achieved from static boxes
    if (refal.static_boxes != NULL)
    {
        T_LINKCB *static_box_head = refal.static_boxes;
        do
        {
            mark_dynamic_box_heads(static_box_head);
            static_box_head = static_box_head->info.codep;
        } while (static_box_head != NULL);
    }
    //   remove garbage
    T_LINKCB virtual_linkcb;
    T_LINKCB *virtual_dynamic_box_head = &virtual_linkcb;
    virtual_dynamic_box_head->info.codep = refal.dynamic_boxes;
    T_LINKCB *temp_dynamic_box_head = virtual_dynamic_box_head;
    T_LINKCB *dynamic_box_head = refal.dynamic_boxes;
    bool was_collected_garbage = false;
    do
    {
        if (dynamic_box_head->tag != TAGO)
        { // box isn't removed
            dynamic_box_head->tag = TAGO;
            temp_dynamic_box_head = dynamic_box_head;
        }
        else
        { // remove box
            was_collected_garbage = true;
            temp_dynamic_box_head->info.code = dynamic_box_head->info.code;
            temp_dynamic_box_head->tag = dynamic_box_head->tag;
            T_LINKCB *last_linkcb_dynamic_box = dynamic_box_head->previous;
            T_LINKCB *first_linkcb_free_memory = refal.free_memory_list_head->next;
            last_linkcb_dynamic_box->next = first_linkcb_free_memory;
            first_linkcb_free_memory->previous = last_linkcb_dynamic_box;
            refal.free_memory_list_head->next = dynamic_box_head;
            dynamic_box_head->previous = refal.free_memory_list_head;
        }
        dynamic_box_head = temp_dynamic_box_head->info.codep;
    } while (dynamic_box_head != NULL);
    if (virtual_dynamic_box_head->info.codep == NULL)
        refal.dynamic_boxes = NULL;
    else
        refal.dynamic_boxes = virtual_dynamic_box_head->info.codep;
    return was_collected_garbage;
}

static void add_free_memory_list(T_LINKCB *block_free_memory, size_t size_block_free_memory)
{
    if (refal_init)
        refal_initiate();
    T_LINKCB *linkcb_block_free_memory = block_free_memory;
    T_LINKCB *last_linkcb_free_memory = refal.free_memory_list_head->previous;
    for (size_t i = 0; i < size_block_free_memory; i++)
    {
        last_linkcb_free_memory->next = linkcb_block_free_memory;
        linkcb_block_free_memory->previous = last_linkcb_free_memory;
        linkcb_block_free_memory->tag = TAGO;
        linkcb_block_free_memory->info.code = NULL;
        last_linkcb_free_memory = linkcb_block_free_memory;
        linkcb_block_free_memory++;
    }
    last_linkcb_free_memory->next = refal.free_memory_list_head;
    refal.free_memory_list_head->previous = last_linkcb_free_memory;
    return;
}

T_LINKCB *lldupl(const T_LINKCB *p, const T_LINKCB *q, const T_LINKCB *u)
{
    const T_LINKCB *x = p->next;
    T_LINKCB *y = u->next;
    while (x != q)
    {
        if (x->tag != y->tag)
            return NULL;
        if (x->info.code != y->info.code)
            if (x->tag != TAGLB && x->tag != TAGRB)
                return NULL;
        x = x->next;
        y = y->next;
    }
    return y;
}

T_LINKCB *rfrstr(const char *str, T_LINKCB *p)
{
    if (*str == '\0')
        return NULL;
    for (size_t i = 0; *(str + i) != '\0'; i++)
    {
        p = p->next;
        p->tag = TAGO;
        p->info.code = NULL;
        p->info.infoc = *(str + i);
    }
    return p;
}

bool rfreof(int c, FILE *f, T_LINKCB *p)
{
    enum
    {
        OK,
        FEOF,
        FERROR
    } err = OK;
    if (c == EOF)
    {
        if (feof(f) != 0)
            err = FEOF;
        else if (ferror(f) != 0)
            err = FERROR;
    }
    if (err != OK)
    {
        p->tag = TAGF;
        if (err == FEOF)
            p->info.codef = &refalab_feof;
        else
            p->info.codef = &refalab_ferror;
        return true;
    }
    return false;
}

T_LINKCB *rfgstr(char *str, size_t l, T_LINKCB *p)
{
    size_t i;
    for (i = 0; p != refal.next_argument; i++)
    {
        if (p->tag != TAGO || i == l)
            break;
        *(str + i) = p->info.infoc;
        p = p->next;
    }
    *(str + i) = '\0';
    return p;
}

//----------- end of file  RFINTF.C ------------
