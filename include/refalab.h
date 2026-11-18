// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-11-17
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

// Tag values of a list cell (T_LINKCB.tag).
// Even tags mark terms, odd tags mark brackets (see BRACKET below).
// TAGO: a symbol (character in info.infoc)
// TAGF: a function label (pointer to the function in info.codef)
// TAGN: a macrodigit (32-bit number in info.coden)
// TAGR: a reference to a box (pointer in info.codep)
#define TAGO 0
#define TAGF 2
#define TAGN 4
#define TAGR 6
// TAGLB/TAGRB: structure brackets '(' and ')'.
// TAGK/TAGD: call brackets '<' and '>' of a function call.
#define TAGLB 1
#define TAGRB 3
#define TAGK 5
#define TAGD 7

// Padding in front of a function label name.
// A label name is padded with '0' bytes up to 6 characters: Zn is used for a name
// of n letters (Z6 is empty, Z1 gives 5 bytes, Z0 gives 6 bytes for an empty name).
#define Z_0 0
#define Z1 '0', '0', '0', '0', '0',
#define Z2 '0', '0', '0', '0',
#define Z3 '0', '0', '0',
#define Z4 '0', '0',
#define Z5 '0',
#define Z6
#define Z7 '0', '0', '0', '0', '0', '0', '0',
#define Z0 '0', '0', '0', '0', '0', '0',

// Marker of a global (externally visible) label.
#define G_L_B
// Accessors for the macrodigit and the bracket-test macros.
// BRACKET is true for any bracket cell, NOT_BRACKET for any term.
#define gcoden(p) p->info.coden
#define pcoden(p, W) p->info.coden = W
#define BRACKET(b) (b->tag & 0001) != 0
#define NOT_BRACKET(b) (b->tag & 0001) == 0

// Sizes used when laying out a function label:
// NMBL - the length byte of a name, ZBLL - a 32-bit number,
// LBLL - a pointer, SMBL - a number followed by a pointer.
#define NMBL sizeof(uint8_t)
#define ZBLL sizeof(uint32_t)
#define LBLL sizeof(void *)
#define SMBL (ZBLL + LBLL)

// Largest macrodigit.
#define MAX_NUMBER UINT32_MAX

// Step limit meaning "run without stopping".
#define MAX_STOP INT32_MAX

// Maximum length of a path with a file name on the target OS.
#if defined POSIX
#define MAX_PATHFILENAME 4096
#else
#define MAX_PATHFILENAME 260
#endif

// Pragma for 1-byte structure alignment
// Cells are packed to 1 byte so their layout does not depend on the compiler.
#pragma pack(push, 1)

// A cell of the doubly linked list that holds every Refal expression:
// previous/next link the neighbours, tag tells what the cell is,
// info holds the payload selected by the tag.
typedef struct linkcb
{
    struct linkcb *previous;
    struct linkcb *next;
    uint32_t tag;
    union
    {
        char infoc;
        uint32_t coden;
        struct linkcb *codep;
        uint8_t *codef;
        void *code;
    } info;
} T_LINKCB;

#pragma pack(pop)

// End of pragma

// State of one Refal computation (a view field together with its store).
// Tables of nested computations (see Try) are chained by previous/next.
// state: 1 - normal (running or finished), 2 - recognition impossible,
// 3 - out of memory.
// dot: the current call bracket '>'; step/stop: step counter and the limit.
// view: the view field; store: the store of the computation.
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

// Global state of the Refal machine.
// upshot: result code of the last function (1 - success, 2 - FAIL, 3 - no memory)
// previous_argument/next_argument: bounds of the argument of the running function
// previous_result/next_result: where the result is to be placed
// free_memory_head: head of the list of free cells
// static_boxes/dynamic_boxes: lists of boxes of both kinds
// arg: command line arguments; timer: run-time measurement.
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
    T_LINKCB *free_memory_head;
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

// A long integer read from a Refal expression: sign, the range of cells
// holding its digits and the number of digits.
typedef struct big_number
{
    char sign;
    T_LINKCB *begin;
    T_LINKCB *end;
    size_t length;
} T_BIG_NUMBER;

// Interface functions implemented by the interpreter core (interface.c).
extern void refal_abort_end(const char *abort_message);
extern void delete_status_table(const T_STATUS_TABLE *status_table);
extern void insert_to_free_memory(T_LINKCB *before, T_LINKCB *after);
extern void refal_execute(uint8_t *refalab_function);
extern void refal_get_args(int argc, char *argv[]);
extern T_LINKCB *get_string_expression(char *string, size_t max_string_size, T_LINKCB *begin, const T_LINKCB *max_after);
extern void refal_initiate(void);
extern void print_status_table(T_STATUS_TABLE *status_table, bool additional_info);
extern void print_expression(const char *begin_string, const T_LINKCB *before, const T_LINKCB *after, const bool new_line);
extern void print_expression_m(const char *begin_string, const T_LINKCB *before, const T_LINKCB *after, const bool new_line);
extern void print_expression_s(const char *begin_string, const T_LINKCB *before, const T_LINKCB *after, const bool new_line);
extern bool set_eof_linkcb(int file_result, FILE *file, T_LINKCB *linkcb);
extern T_LINKCB *set_string_expression(const char *string, T_LINKCB *before);
extern void refal_terminate_memory(void);
extern void transplantation(T_LINKCB *where, T_LINKCB *before, T_LINKCB *after);
extern bool read_big_number_expression(T_BIG_NUMBER *big_number, const T_LINKCB *before, const T_LINKCB *after);
extern bool read_big_numbers_expression(T_BIG_NUMBER *big_number1, T_BIG_NUMBER *big_number2, const T_LINKCB *before, const T_LINKCB *middle, const T_LINKCB *after);
extern void exchange_big_numbers(T_BIG_NUMBER *big_number1, T_BIG_NUMBER *big_number2);
extern int8_t compare_big_numbers(const T_BIG_NUMBER *big_number1, const T_BIG_NUMBER *big_number2);
extern int8_t compare_big_numbers_absolute(const T_BIG_NUMBER *big_number1, const T_BIG_NUMBER *big_number2);
extern int8_t compare_expressions_lexicographic(const T_LINKCB *before, const T_LINKCB *middle, const T_LINKCB *after);
extern FILE *file_get(uint32_t id_number);
extern bool file_add(uint32_t id_number, FILE *file);
extern bool file_remove(uint32_t id_number);

// Functions for module writers: work with status tables, free memory
// and duplicate search.
extern bool copy_expression(T_LINKCB *where, T_LINKCB *before, T_LINKCB *after);
extern bool create_status_table(T_STATUS_TABLE *status_table);
extern bool exist_status_table(const T_STATUS_TABLE *status_table);
extern bool more_free_memory(void);
extern bool insert_from_free_memory(T_LINKCB *where, size_t count);
extern bool extended_insert_from_free_memory(T_LINKCB *where, size_t count);
extern bool insert_view_k_function_dot(T_STATUS_TABLE *status_table, uint8_t *refalab_function);
extern T_LINKCB *find_duplicate(const T_LINKCB *before, const T_LINKCB *after, const T_LINKCB *before_duplicate);
extern bool check_count_free_memory(size_t count);

// Runs the interpreter on the given status table.
extern void refal_run(T_STATUS_TABLE *status_table);

// Debug-build only helpers (compiled with -Dmdebug).
#if defined mdebug
extern void print_memory_state(void);
extern void debug_print_expression(const char *begin_string, const T_LINKCB *before, const T_LINKCB *after);
#endif

// Debugger hook: NULL when no debugger is attached.
extern void (*status_table_debugger)(T_STATUS_TABLE *);
extern void refal_debugger(T_STATUS_TABLE *status_table);

#endif

//-------  end of file REFALAB.H ---------
