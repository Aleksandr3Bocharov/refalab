// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-04-09
// https://github.com/Aleksandr3Bocharov/refalab

//----------------  file  --  CLU.H  -------------------
//    AVL-tree construction for identifier table
//------------------------------------------------------

#ifndef CLU_H
#define CLU_H

#include <stddef.h>
#include <stdint.h>
#include "refalab.h"

typedef struct usage_list
{
    struct usage_list *next;      // on the next usage list
    uint32_t carriage_numbers[6]; // usage list element
} T_USAGE_LIST;

typedef struct label
{
    union
    {
        size_t infon;
        struct label *infop;
    } info;
    uint8_t mode;
    // mode field :
    //  00 - no defined;
    //  01 - internal; infon = offset from start
    //  10 - external; infon = member or extern
    //       reference;
    //  11 - equivalent; infop =  reference on
    //       other label;
    //  xx1 - entry point;
    //  xxx1 - too many definition;
    //
    uint8_t type;                           // type field : 00 - unknown type
                                            //              01 - function
                                            //              10 - specifier
    T_USAGE_LIST *last_usage_list;          // on the end of usage list
    T_USAGE_LIST usage_list;                // where used
    uint32_t carriage_number_defined;       // where defined
    char identifier[MAX_IDENTIFIER_LENGTH]; // identifier
    uint8_t identifier_length;              // identifier length
    struct label *left_label;               // left reference
    struct label *right_label;              // right reference
    uint8_t balance;                        // balance feature:   '00'B - no
                                            //                    '01'B - left
                                            //                    '10'B - right

} T_LABEL;

extern T_LABEL *lookup_label(const char *identifier, uint8_t identifier_length);
extern void labels_terminate(void);
extern void through_labels(void (*handler)(const T_LABEL *));
extern void error_no_memory_labels(void);

#endif
