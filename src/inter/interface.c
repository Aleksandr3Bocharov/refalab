// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-11-17
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file interface.c  ----------
//           C-interface functions
//----------------------------------------

#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <inttypes.h>
#include <stdbool.h>
#include <errno.h>
#include <time.h>
#include "refalab.h"
#include "interface.h"
#include "interpreter.h"

// Size of the hash table of the open files (the file is found by its number).
#define FILE_HASH_SIZE 256

// number of hex digits to print an address (info field, reference): 8 on x32, 16 on x64
#define PTR_HEX_WIDTH ((int)(2 * sizeof(uintptr_t)))

extern uint8_t refalab_feof, refalab_ferror;

// A node of the chain of the files with the same hash: the identification number
// that a Refal program uses and the C stream.
typedef struct file_node
{
    uint32_t id_number;
    FILE *file;
    struct file_node *next;
} T_FILE_NODE;

// The global state of the Refal machine (see refalab.h).
T_REFAL refal;

// Options of the interpreter (set by the first argument --rfinteropt... of the command line):
//   timer_on - measurement of the run time (-timer_off turns it off);
//   min_list_memory - the size of the first block of list memory in cells (-minsz=);
//   max_list_memory - the limit of the whole list memory, 0 for no limit (-maxsz=);
//   increase_list_memory - the size of the next blocks (-incrsz=);
//   free_memory_count - how many cells garbage collection must free so that
//   the memory is not enlarged (-freesz=).
static struct
{
    bool timer_on;
    uint32_t min_list_memory;
    uint32_t max_list_memory;
    uint32_t increase_list_memory;
    uint32_t free_memory_count;
} options = {true, 500, 0, 200, 200};

// The command line saved by refal_get_args.
static size_t gargc = 0;
static char **gargv = NULL;

// List memory is allocated in blocks; the blocks are chained through their first cell, and
// the cells of a block go to the list of free cells (free_memory_head, a circular list).
static T_LINKCB *last_block_list_memory = NULL;
static size_t list_memory_count = 0;
// The machine is initialized at the first use (refal_initiate).
static bool refal_init = true;
static T_LINKCB free_memory_head;

// Hash table of the open files.
static T_FILE_NODE *file_table[FILE_HASH_SIZE] = {NULL};

// Garbage collection: returns the unreachable cells to the list of free cells.
static bool collect_garbage(void);
// Links the cells of a new block into the list of free cells.
static void add_free_memory(T_LINKCB *block_free_memory, size_t size_block_free_memory);

// Saves the command line and reads the options from the first argument that begins with
// --rfinteropt. Values out of the range 1..INT32_MAX are ignored, a repeated option is
// ignored too.
void refal_get_args(int argc, char *argv[])
{
    gargc = (size_t)argc;
    gargv = argv;
    for (size_t i = 1; i < gargc; i++)
        if (strncmp(gargv[i], "--rfinteropt", 12) == 0)
        {
            if (strstr(gargv[i], "-timer_off") != NULL)
                options.timer_on = false;
            const char *minsz = strstr(gargv[i], "-minsz=");
            if (minsz != NULL)
            {
                char *end_minsz;
                errno = 0;
                long int value = strtol(minsz + 7, &end_minsz, 10);
                if (errno == ERANGE || value < 1 || value > INT32_MAX)
                    ;
                else
                    options.min_list_memory = (uint32_t)value;
            }
            const char *maxsz = strstr(gargv[i], "-maxsz=");
            if (maxsz != NULL)
            {
                char *end_maxsz;
                errno = 0;
                long int value = strtol(maxsz + 7, &end_maxsz, 10);
                if (errno == ERANGE || value < 1 || value > INT32_MAX)
                    ;
                else
                    options.max_list_memory = (uint32_t)value;
            }
            const char *incrsz = strstr(gargv[i], "-incrsz=");
            if (incrsz != NULL)
            {
                char *end_incrsz;
                errno = 0;
                long int value = strtol(incrsz + 8, &end_incrsz, 10);
                if (errno == ERANGE || value < 1 || value > INT32_MAX)
                    ;
                else
                    options.increase_list_memory = (uint32_t)value;
            }
            const char *freesz = strstr(gargv[i], "-freesz=");
            if (freesz != NULL)
            {
                char *end_freesz;
                errno = 0;
                long int value = strtol(freesz + 8, &end_freesz, 10);
                if (errno == ERANGE || value < 1 || value > INT32_MAX)
                    ;
                else
                    options.free_memory_count = (uint32_t)value;
            }
            break;
        }
    return;
}

// Fatal error: prints the message and ends the program with exit code 1.
void refal_abort_end(const char *abort_message)
{
    printf(" *** refal-abort-end *** %s\n", abort_message);
    exit(1);
    return;
}

// Gets more free cells: first garbage is collected (when blocks already exist);
// if that frees fewer cells than free_memory_count, a new block is allocated (the first
// one has min_list_memory cells, the next ones increase_list_memory, within max_list_memory).
// Returns false when the memory cannot be enlarged.
bool more_free_memory(void)
{
    uint32_t increase_list_memory;
    uint32_t free_memory_count = 0;
    if (last_block_list_memory == NULL)
    {
        if (options.max_list_memory != 0 && options.max_list_memory < options.min_list_memory)
            return false;
        increase_list_memory = options.min_list_memory;
    }
    else
    {
        const T_LINKCB *first_linkcb_free_memory = refal.free_memory_head->next;
        if (collect_garbage())
        {
            const T_LINKCB *linkcb_free_memory = refal.free_memory_head->next;
            while (linkcb_free_memory != first_linkcb_free_memory && free_memory_count != options.free_memory_count)
            {
                free_memory_count++;
                linkcb_free_memory = linkcb_free_memory->next;
            }
#if defined mdebug
            fprintf(stderr, "more_free_memory: free_memory_count=%" PRIu32 " after collect_garbage\n", free_memory_count);
#endif
            if (free_memory_count == options.free_memory_count)
                return true;
        }
        if (options.max_list_memory == 0)
            increase_list_memory = options.increase_list_memory;
        else
        {
            if (list_memory_count == options.max_list_memory)
                return false;
            const uint32_t up_increase_limit_list_memory = options.max_list_memory - (uint32_t)list_memory_count;
            increase_list_memory = up_increase_limit_list_memory >= options.increase_list_memory ? options.increase_list_memory : up_increase_limit_list_memory;
        }
    }
    T_LINKCB *new_block_list_memory = malloc((increase_list_memory + 1) * sizeof(T_LINKCB));
    if (new_block_list_memory == NULL)
        return false;
#if defined mdebug
    fprintf(stderr, "more_free_memory: increase_list_memory=%" PRIu32 " after new_block_list_memory=%p\n", increase_list_memory, (void *)new_block_list_memory);
#endif
    new_block_list_memory->previous = last_block_list_memory;
    last_block_list_memory = new_block_list_memory;
    list_memory_count += increase_list_memory;
    add_free_memory(new_block_list_memory + 1, increase_list_memory);
    return true;
}

//  check a number of items in free items list
// Checks that the list of free cells has at least count cells.
bool check_count_free_memory(size_t count)
{
    const T_LINKCB *linkcb_free_memory = refal.free_memory_head;
    for (size_t i = 0; i < count; i++)
    {
        linkcb_free_memory = linkcb_free_memory->next;
        if (linkcb_free_memory == refal.free_memory_head)
            return false;
    }
    return true;
}

// Takes count free cells and links them into the expression after the cell where.
// The cells become empty symbols. Returns false when there are not enough free cells.
bool insert_from_free_memory(T_LINKCB *where, size_t count)
{
    if (count == 0)
        return true;
    T_LINKCB *linkcb_free_memory = refal.free_memory_head;
    for (size_t i = 0; i < count; i++)
    {
        linkcb_free_memory = linkcb_free_memory->next;
        if (linkcb_free_memory == refal.free_memory_head)
            return false;
        linkcb_free_memory->tag = TAGO;
        linkcb_free_memory->info.code = NULL;
    }
    T_LINKCB *next_linkcb_free_memory = linkcb_free_memory->next;
    T_LINKCB *first_linkcb_free_memory = refal.free_memory_head->next;
    refal.free_memory_head->next = next_linkcb_free_memory;
    next_linkcb_free_memory->previous = refal.free_memory_head;
    T_LINKCB *next_where = where->next;
    linkcb_free_memory->next = next_where;
    next_where->previous = linkcb_free_memory;
    where->next = first_linkcb_free_memory;
    first_linkcb_free_memory->previous = where;
    return true;
}

// The same, but enlarges the memory when necessary; on a lack of memory sets upshot to 3.
bool extended_insert_from_free_memory(T_LINKCB *where, size_t count)
{
    while (!check_count_free_memory(count))
        if (!more_free_memory())
        {
            refal.upshot = 3;
            return false;
        }
    return insert_from_free_memory(where, count);
}

// Makes the view field of an empty process <function argument>: the argument that is
// already in the view field is enclosed by the call brackets, and refalab_function
// is the label that follows '<'.
bool insert_view_k_function_dot(T_STATUS_TABLE *status_table, uint8_t *refalab_function)
{
    if (!exist_status_table(status_table))
        refal_abort_end("insert_view_k_function_dot: process doesn't exist still");
    if (status_table->dot != NULL)
        refal_abort_end("insert_view_k_function_dot: there are '<'-signes in view field");
    if (!extended_insert_from_free_memory(status_table->view, 3))
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

// Initializes the machine: the empty list of status tables, the list of free cells,
// the command line and the timer.
void refal_initiate(void)
{
    refal_init = false;
    refal.last_status_table = (T_STATUS_TABLE *)&refal;
    refal.first_status_table = (T_STATUS_TABLE *)&refal;
    refal.upshot = 1;
    refal.current_status_table = NULL;
    refal.static_boxes = NULL;
    refal.dynamic_boxes = NULL;
    refal.free_memory_head = &free_memory_head;
    refal.free_memory_head->previous = &free_memory_head;
    refal.free_memory_head->next = &free_memory_head;
    refal.free_memory_head->tag = TAGO;
    refal.free_memory_head->info.code = NULL;
    refal.arg.argc = gargc;
    refal.arg.argv = gargv;
    refal.timer.mode = options.timer_on;
    if (refal.timer.mode)
        clock_gettime(CLOCK_MONOTONIC, &refal.timer.start_time);
    return;
}

// Deletes a process: its status table leaves the list of tables and the cells of its
// view field and store go back to the list of free cells. A process that runs (state 4)
// cannot be deleted.
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
    T_LINKCB *last_linkcb_free_memory = refal.free_memory_head->previous;
    T_LINKCB *last_view = status_table->view->previous;
    T_LINKCB *last_store = status_table->store->previous;
    last_linkcb_free_memory->next = status_table->view;
    status_table->view->previous = last_linkcb_free_memory;
    last_view->next = status_table->store;
    status_table->store->previous = last_view;
    last_store->next = refal.free_memory_head;
    refal.free_memory_head->previous = last_store;
    return;
}

//    delete part of list and add it to free memory list
// Cuts the part between before and after out of the expression and
// returns its cells to the list of free cells.
void insert_to_free_memory(T_LINKCB *before, T_LINKCB *after)
{
    T_LINKCB *first = before->next;
    if (first == after)
        return;
    T_LINKCB *last = after->previous;
    T_LINKCB *where = refal.free_memory_head->previous;
    before->next = after;
    after->previous = before;
    last->next = refal.free_memory_head;
    refal.free_memory_head->previous = last;
    where->next = first;
    first->previous = where;
    return;
}

// Gives all the blocks of list memory back to the system.
void refal_terminate_memory(void)
{
    while (last_block_list_memory != NULL)
    {
        T_LINKCB *delete_block_list_memory = last_block_list_memory;
        last_block_list_memory = delete_block_list_memory->previous;
#if defined mdebug
        fprintf(stderr, "refal_terminate_memory: free delete_block_list_memory=%p\n", (void *)delete_block_list_memory);
#endif
        free(delete_block_list_memory);
    }
    list_memory_count = 0;
}

// Runs a program: the process with the view field <refalab_function> is computed to the end.
// Then the final state (the view field, the burried expressions, memory and time) is printed,
// the process is deleted, the memory is released and the function returns.
// "Concretization is executed" means a normal end; "Recognition impossible" and
// "Free memory exhausted" are the two abnormal ends.
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
    const uint32_t step_stop = status_table.stop;
#endif
    // States of the run loop: AGAIN - the next portion of steps, DONE - the view field is
    // computed, ABEND - an abnormal end, EOJ - print the final state, RET - clean up and return.
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
        // In the debug build every step is executed separately and traced; otherwise the interpreter
        // runs until it stops. A stop for lack of memory is followed by an enlargement of the memory,
        // after which the run continues; a stop at the step limit resets the step counter.
        case AGAIN:
#if defined mdebug
            //	step by step execution with full debug trace information
            if (status_table.dot == NULL)
            {
                execute_state = DONE;
                break;
            }
            if (status_table.state != 1 || status_table.step >= step_stop)
            {
                bool abort_end = true;
                if (status_table.state == 3)
                    if (more_free_memory())
                        abort_end = false;
                if (status_table.step >= step_stop)
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
            fprintf(stderr, " Step: %" PRIu32 "\n", status_table.stop);
            debug_print_expression(" Term: ", previous_k, next_dot);
            refal_run(&status_table);
            if (status_table.state == 1)
                debug_print_expression(" Result: ", previous_k, next_dot);
            print_memory_state();
            break;
#else
            // no debug info
            refal_run(&status_table);
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
        // The view field has no call brackets left.
        case DONE:
#if defined mdebug
            printf("Concretization is executed\n");
            execute_state = EOJ;
#else
            execute_state = RET;
#endif
            break;
        // The reason of the stop is the state of the process: 2 - recognition impossible,
        // 3 - the free memory is exhausted.
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
            print_status_table(&status_table, true);
            execute_state = RET;
            break;
        case RET:
            delete_status_table(&status_table);
            refal_terminate_memory();
            return;
        }
}

#if defined mdebug
// debug trace: state of list memory (free cells are counted by walking the free list)
// Debug trace: the state of the list memory.
void print_memory_state(void)
{
    fflush(stdout); // keep the order of stdout and stderr output
    size_t free_cells = 0;
    for (const T_LINKCB *cell = refal.free_memory_head->next; cell != refal.free_memory_head; cell = cell->next)
        free_cells++;
    fprintf(stderr, " Memory: free=%zu used=%zu total=%zu\n", free_cells, list_memory_count - free_cells, list_memory_count);
}
#endif

// Prints the state of a process: the number of completed steps, the view field and the burried
// expressions (the store). With additional_info also the size of the list memory and
// the elapsed time.
void print_status_table(T_STATUS_TABLE *status_table, bool additional_info)
{
    printf("Completed steps number = %" PRIu32 "\n", status_table->step);
    printf("View field:\n");
    print_expression_m("     ", status_table->view, status_table->view, true);
    if (status_table->store->next != status_table->store)
    {
        printf("Burried:\n");
        print_expression_m("     ", status_table->store, status_table->store, true);
    }
    if (additional_info)
    {
        printf("List memory count = %zu (%zu bytes)\n", list_memory_count, list_memory_count * sizeof(T_LINKCB));
        if (refal.timer.mode)
        {
            clock_gettime(CLOCK_MONOTONIC, &refal.timer.stop_time);
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
    }
}

// Prints the expression after the cell `before` up to the cell before `after`, preceded by
// begin_string (print_expression: the form of the function Print). Characters are printed as they
// are, numbers as '123', function labels as 'NAME' and references as '%address'.
// New line at the end when new_line is set.
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
            printf("'%" PRIu32 "'", gcoden(print));
        else if (print->tag == TAGF)
        {
            putchar('\'');
            const uint8_t *label_length = print->info.codef - 1;
            const char *label = (char *)label_length - *label_length;
            for (uint8_t i = 0; i < *label_length; i++, label++)
                putchar(toupper((unsigned char)*label));
            putchar('\'');
        }
        else if (print->tag == TAGR)
            printf("'%%%0*" PRIxPTR "'", PTR_HEX_WIDTH, (uintptr_t)print->info.codep);
        else if (BRACKET(print))
            refal_abort_end("print_expression: unknown bracket type");
        else
            printf("'%" PRIx32 ",%0*" PRIxPTR "'", print->tag, PTR_HEX_WIDTH, (uintptr_t)print->info.code);
    }
    if (new_line)
        printf("\n");
    return;
}

// Like print_expression, but numbers and labels are printed without apostrophes
// (the form of the function Prints).
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
            printf("%" PRIu32, gcoden(prints));
        else if (prints->tag == TAGF)
        {
            const uint8_t *label_length = prints->info.codef - 1;
            const char *label = (char *)label_length - *label_length;
            for (uint8_t i = 0; i < *label_length; i++, label++)
                putchar(toupper((unsigned char)*label));
        }
        else if (prints->tag == TAGR)
            printf("%%%0*" PRIxPTR, PTR_HEX_WIDTH, (uintptr_t)prints->info.codep);
        else if (BRACKET(prints))
            refal_abort_end("print_expression_s: unknown bracket type");
        else
            printf("%" PRIx32 ",%0*" PRIxPTR, prints->tag, PTR_HEX_WIDTH, (uintptr_t)prints->info.code);
    }
    if (new_line)
        printf("\n");
    return;
}

// Like print_expression, but in the form of a source program (the function Printm):
// neighbouring characters are put into one pair of apostrophes, numbers and labels are separated
// by blanks, the call bracket is printed as '< ' and references as /%address/.
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
                printf("%" PRIu32, gcoden(printm));
                if (printm->next->tag == TAGN)
                    putchar(' ');
            }
            else if (printm->tag == TAGF)
            {
                const uint8_t *label_length = printm->info.codef - 1;
                const char *label = (char *)label_length - *label_length;
                for (uint8_t i = 0; i < *label_length; i++, label++)
                    putchar(toupper((unsigned char)*label));
                if (printm->next->tag == TAGN || printm->next->tag == TAGF)
                    putchar(' ');
            }
            else if (printm->tag == TAGR)
                printf("/%%%0*" PRIxPTR "/", PTR_HEX_WIDTH, (uintptr_t)printm->info.codep);
            else if (BRACKET(printm))
                refal_abort_end("print_expression_m: unknown bracket type");
            else
                printf("/%" PRIx32 ",%0*" PRIxPTR "/", printm->tag, PTR_HEX_WIDTH, (uintptr_t)printm->info.code);
        }
    }
    if (tago)
        putchar('\'');
    if (new_line)
        printf("\n");
    return;
}

// Moves the part of an expression between `before` and `after` (not including them) to the place
// after the cell `where`. Nothing is copied; only the links are changed. Nothing happens
// when the part is empty or already at the place.
void transplantation(T_LINKCB *where, T_LINKCB *before, T_LINKCB *after)
{
    T_LINKCB *first = before->next;
    if (first == after)
        return;
    T_LINKCB *last = after->previous;
    if (where == before || where == last)
        return;
    T_LINKCB *next_where = where->next;
    before->next = after;
    after->previous = before;
    last->next = next_where;
    next_where->previous = last;
    where->next = first;
    first->previous = where;
    return;
}

// Copies the expression between `before` and `after` (not including them) after the cell `where`.
// The copy is made of free cells, the brackets are paired in the copy.
// Returns false when the free cells are not enough (nothing is changed then).
bool copy_expression(T_LINKCB *where, T_LINKCB *before, T_LINKCB *after)
{
    T_LINKCB *linkcb_free_memory = refal.free_memory_head;
    const T_LINKCB *current = before->next;
    T_LINKCB *last_bracket = NULL;
    while (current != after)
    {
        linkcb_free_memory = linkcb_free_memory->next;
        if (linkcb_free_memory == refal.free_memory_head)
            return false;
        switch (current->tag)
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
            linkcb_free_memory->tag = current->tag;
            linkcb_free_memory->info.code = current->info.code;
        }
        current = current->next;
    }
    if (refal.free_memory_head == linkcb_free_memory)
        return true;
    T_LINKCB *first_linkcb_free_memory = refal.free_memory_head->next;
    T_LINKCB *next_linkcb_free_memory = linkcb_free_memory->next;
    refal.free_memory_head->next = next_linkcb_free_memory;
    next_linkcb_free_memory->previous = refal.free_memory_head;
    T_LINKCB *next_where = where->next;
    linkcb_free_memory->next = next_where;
    next_where->previous = linkcb_free_memory;
    where->next = first_linkcb_free_memory;
    first_linkcb_free_memory->previous = where;
    return true;
}

// Checks that the status table is in the list of the tables of the machine.
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

// Creates a process: takes two cells from the list of free cells (the heads of the view field
// and of the store, both empty circular lists) and puts the table into the list of the tables.
// The process starts in the state 1 with no call brackets and no step limit.
// Returns false when there are not enough free cells.
bool create_status_table(T_STATUS_TABLE *status_table)
{
    if (refal_init)
        refal_initiate();
    if (exist_status_table(status_table))
        refal_abort_end("create_status_table: process already exists");
    status_table->view = refal.free_memory_head->next;
    if (status_table->view == refal.free_memory_head)
        return false;
    status_table->store = status_table->view->next;
    if (status_table->store == refal.free_memory_head)
        return false;
    T_LINKCB *next_free_memory_head = status_table->store->next;
    refal.free_memory_head->next = next_free_memory_head;
    next_free_memory_head->previous = refal.free_memory_head;
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

// Garbage collection, the marking phase: marks (tag UINT32_MAX) every dynamic box that
// can be reached from the expression `root`, including the boxes reached through other boxes.
// There is no recursion and no stack: the way back is kept inside the list itself by reversing
// the pointers (the reference cell points to the box head that is being walked through,
// the box head remembers the reference cell), and everything is restored when the walk ends.
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
            dynamic_box_head->tag = UINT32_MAX;
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

// Garbage collection. The boxes reachable from the view fields and the stores of all
// processes and from the static boxes are marked, then the dynamic boxes that are not marked
// (nobody can name them any more) are removed from the list of the dynamic boxes and their
// cells go to the list of free cells. Returns true when something was collected.
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
            T_LINKCB *first_linkcb_free_memory = refal.free_memory_head->next;
            last_linkcb_dynamic_box->next = first_linkcb_free_memory;
            first_linkcb_free_memory->previous = last_linkcb_dynamic_box;
            refal.free_memory_head->next = dynamic_box_head;
            dynamic_box_head->previous = refal.free_memory_head;
        }
        dynamic_box_head = temp_dynamic_box_head->info.codep;
    } while (dynamic_box_head != NULL);
    if (virtual_dynamic_box_head->info.codep == NULL)
        refal.dynamic_boxes = NULL;
    else
        refal.dynamic_boxes = virtual_dynamic_box_head->info.codep;
    return was_collected_garbage;
}

// Links the cells of a new block, as empty symbols, to the end of the list of free cells.
static void add_free_memory(T_LINKCB *block_free_memory, size_t size_block_free_memory)
{
    if (refal_init)
        refal_initiate();
    T_LINKCB *linkcb_block_free_memory = block_free_memory;
    T_LINKCB *last_linkcb_free_memory = refal.free_memory_head->previous;
    for (size_t i = 0; i < size_block_free_memory; i++)
    {
        last_linkcb_free_memory->next = linkcb_block_free_memory;
        linkcb_block_free_memory->previous = last_linkcb_free_memory;
        linkcb_block_free_memory->tag = TAGO;
        linkcb_block_free_memory->info.code = NULL;
        last_linkcb_free_memory = linkcb_block_free_memory;
        linkcb_block_free_memory++;
    }
    last_linkcb_free_memory->next = refal.free_memory_head;
    refal.free_memory_head->previous = last_linkcb_free_memory;
    return;
}

// Compares the expression between before and after with the same number of cells that follow
// before_duplicate. Returns the cell after the equal expression or NULL when they differ
// (brackets are equal if their tags are equal).
T_LINKCB *find_duplicate(const T_LINKCB *before, const T_LINKCB *after, const T_LINKCB *before_duplicate)
{
    const T_LINKCB *current = before->next;
    T_LINKCB *current_duplicate = before_duplicate->next;
    while (current != after)
    {
        if (current->tag != current_duplicate->tag)
            return NULL;
        if (current->info.code != current_duplicate->info.code)
            if (current->tag != TAGLB && current->tag != TAGRB)
                return NULL;
        current = current->next;
        current_duplicate = current_duplicate->next;
    }
    return current_duplicate;
}

// Writes a C string as a chain of characters into the cells that follow `before`
// (the cells must be there already). Returns the last cell, NULL for an empty string.
T_LINKCB *set_string_expression(const char *string, T_LINKCB *before)
{
    if (*string == '\0')
        return NULL;
    T_LINKCB *current = before;
    for (size_t i = 0; *(string + i) != '\0'; i++)
    {
        current = current->next;
        current->tag = TAGO;
        current->info.code = NULL;
        current->info.infoc = *(string + i);
    }
    return current;
}

// After a failed input or output operation: when the end of the file or an error is the reason,
// the cell is made the label EOF or ERROR respectively and true is returned.
bool set_eof_linkcb(int file_result, FILE *file, T_LINKCB *linkcb)
{
    enum
    {
        OK,
        FEOF,
        FERROR
    } file_status = OK;
    if (file_result == EOF)
    {
        if (feof(file) != 0)
            file_status = FEOF;
        else if (ferror(file) != 0)
            file_status = FERROR;
    }
    if (file_status != OK)
    {
        linkcb->tag = TAGF;
        if (file_status == FEOF)
            linkcb->info.codef = &refalab_feof;
        else
            linkcb->info.codef = &refalab_ferror;
        return true;
    }
    return false;
}

// Reads characters from the expression into a C string, up to a term that is not a character,
// the end of the expression or max_string_size characters. Returns the cell where the reading stopped.
T_LINKCB *get_string_expression(char *string, size_t max_string_size, T_LINKCB *begin, const T_LINKCB *max_after)
{
    T_LINKCB *current = begin;
    size_t i;
    for (i = 0; current != max_after; i++)
    {
        if (current->tag != TAGO || i == max_string_size)
            break;
        *(string + i) = current->info.infoc;
        current = current->next;
    }
    *(string + i) = '\0';
    return current;
}

// Reads a long integer from an expression: an optional sign and macrodigits. Leading zero
// macrodigits are skipped; an empty number is zero. Returns false when the expression
// is not a number (a lone sign or a term that is not a macrodigit).
bool read_big_number_expression(T_BIG_NUMBER *big_number, const T_LINKCB *before, const T_LINKCB *after)
{
    big_number->sign = '+';
    if (before == after->previous)
    { // an empty number
        big_number->length = 0;
        return true;
    }
    T_LINKCB *current = before->next;
    if (current->tag == TAGO && (current->info.infoc == '+' || current->info.infoc == '-'))
    {
        big_number->sign = current->info.infoc;
        current = current->next;
        if (current == after)
            return false; //  the number consists of a sign only
    }
    for (; current != after && current->tag == TAGN && gcoden(current) == 0; current = current->next)
        ;
    if (current == after)
        big_number->length = 0; //  all digits are zero
    else
    {
        for (big_number->length = 0, big_number->begin = current; current != after && current->tag == TAGN; current = current->next, big_number->length++)
            ;
        if (current != after)
            return false; // not a macrodigit
        big_number->end = after->previous;
    }
    return true;
}

// Reads two long integers: the first is between before and middle, the second between middle and after.
bool read_big_numbers_expression(T_BIG_NUMBER *big_number1, T_BIG_NUMBER *big_number2, const T_LINKCB *before, const T_LINKCB *middle, const T_LINKCB *after)
{
    if (!read_big_number_expression(big_number1, before, middle))
        return false;
    if (!read_big_number_expression(big_number2, middle, after))
        return false;
    return true;
}

// Swaps two numbers.
void exchange_big_numbers(T_BIG_NUMBER *big_number1, T_BIG_NUMBER *big_number2)
{
    T_BIG_NUMBER temp_big_number = *big_number1;
    *big_number1 = *big_number2;
    *big_number2 = temp_big_number;
    return;
}

// Compares long integers: -1, 0 or 1 for less, equal or greater.
int8_t compare_big_numbers(const T_BIG_NUMBER *big_number1, const T_BIG_NUMBER *big_number2)
{
    if (big_number1->length == 0 && big_number2->length == 0)
        return 0; // X=Y
    if (big_number1->sign == '-' && big_number2->sign == '+')
        return -1; // X<Y
    if (big_number1->sign == '+' && big_number2->sign == '-')
        return 1; // X>Y
    const int8_t compare_absolute = compare_big_numbers_absolute(big_number1, big_number2);
    if (compare_absolute == 0)
        return 0; // X=Y;
    if ((big_number1->sign == '-' && compare_absolute == 1) || (big_number1->sign == '+' && compare_absolute == -1))
        return -1; // X<Y
    return 1;      // X>Y
}

// Compares the absolute values: by the number of macrodigits first (leading zeros are
// already dropped), then digit by digit from the most significant one.
int8_t compare_big_numbers_absolute(const T_BIG_NUMBER *big_number1, const T_BIG_NUMBER *big_number2)
{
    if (big_number1->length == 0 && big_number2->length == 0)
        return 0; // |X|=|Y|
    if (big_number1->length < big_number2->length)
        return -1; // |X|<|Y|
    if (big_number1->length > big_number2->length)
        return 1; // |X|>|Y|
    for (T_LINKCB *big_number1_current = big_number1->begin, *big_number2_current = big_number2->begin; big_number1_current != big_number1->end->next; big_number1_current = big_number1_current->next, big_number2_current = big_number2_current->next)
    {
        if (gcoden(big_number1_current) < gcoden(big_number2_current))
            return -1; // |X|<|Y|
        if (gcoden(big_number1_current) > gcoden(big_number2_current))
            return 1; // |X|>|Y|
    }
    return 0; // |X|=|Y|
}

// Lexicographic comparison of the expression between before and middle with the one between
// middle and after: -1, 0 or 1. Terms are compared by the tag and then by the value (the address
// of a label or a reference, the code of a character or of a number). The order of the brackets is such
// that any term is smaller than a left bracket and greater than a right one, so a shorter expression
// is smaller and an expression that ends earlier at a closing bracket precedes a longer one.
int8_t compare_expressions_lexicographic(const T_LINKCB *before, const T_LINKCB *middle, const T_LINKCB *after)
{
    const T_LINKCB *first_current = before->next;
    const T_LINKCB *second_current = middle->next;
    for (; first_current != middle && second_current != after; first_current = first_current->next, second_current = second_current->next)
        if ((first_current->tag == TAGLB && second_current->tag == TAGLB) || (first_current->tag == TAGRB && second_current->tag == TAGRB))
            continue;
        else if (first_current->tag == TAGLB || second_current->tag == TAGRB)
            return 1; // X>Y
        else if (first_current->tag == TAGRB || second_current->tag == TAGLB)
            return -1; // X<Y
        else if (first_current->tag > second_current->tag)
            return 1; // X>Y
        else if (first_current->tag < second_current->tag)
            return -1; // X<Y
        else if ((size_t)first_current->info.code > (size_t)second_current->info.code)
            return 1; // X>Y
        else if ((size_t)first_current->info.code < (size_t)second_current->info.code)
            return -1; // X<Y
    if (first_current == middle && second_current != after)
        return -1; // X<Y
    if (second_current == after && first_current != middle)
        return 1; // X>Y
    return 0;     // X=Y
}

// MurmurHash3 finalizer
// Hash of a file number (the same mixing as in the final step of MurmurHash3),
// reduced to the size of the table.
static inline uint32_t file_hash(uint32_t id_number)
{
    uint32_t hash = id_number;
    hash ^= hash >> 16;
    hash *= 0x85ebca6bU;
    hash ^= hash >> 13;
    hash *= 0xc2b2ae35U;
    hash ^= hash >> 16;
    return hash & (FILE_HASH_SIZE - 1);
}

// Table of the open files, a hash table with chains. Returns the stream with the number or NULL.
FILE *file_get(uint32_t id_number)
{
    for (T_FILE_NODE *file_node = file_table[file_hash(id_number)]; file_node != NULL; file_node = file_node->next)
        if (file_node->id_number == id_number)
            return file_node->file;
    return NULL;
}

// Enters a stream under a number; false when the number is used or there is no memory.
bool file_add(uint32_t id_number, FILE *file)
{
    if (file_get(id_number) != NULL)
        return false;
    T_FILE_NODE *file_node = malloc(sizeof(T_FILE_NODE));
    if (file_node == NULL)
        return false;
    file_node->id_number = id_number;
    file_node->file = file;
    const uint32_t hash = file_hash(id_number);
    file_node->next = file_table[hash];
    file_table[hash] = file_node;
    return true;
}

// Removes the number from the table (the stream is not closed); false when it is unknown.
bool file_remove(uint32_t id_number)
{
    const uint32_t hash = file_hash(id_number);
    T_FILE_NODE *file_node = file_table[hash];
    T_FILE_NODE *previous_node = NULL;
    while (file_node != NULL)
    {
        if (file_node->id_number == id_number)
        {
            if (previous_node == NULL)
                file_table[hash] = file_node->next;
            else
                previous_node->next = file_node->next;
            free(file_node);
            return true;
        }
        previous_node = file_node;
        file_node = file_node->next;
    }
    return false;
}

//----------- end of file interface.c ------------
