// Copyright 2024 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2024-10-19
// https://github.com/Aleksandr3Bocharov/RefalAB

//----------------  file  --  CLU.H  -------------------
//    AVL-tree construction for identifier table
//------------------------------------------------------

#ifndef CLU_H
#define CLU_H

#include <stddef.h>
#include <stdint.h>

typedef struct refw
{
    struct refw *next; // on the next usage list
    uint32_t numb[6];  // usage list element
} T_REFW;

typedef struct u
{
    union
    {
        size_t infon;
        struct u *infop;
    } info;
    char mode;
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
    char type;        // type field : 00 - unknown type
                      //              01 - function
                      //              10 - specifier
    size_t l;         // identifier length
    struct u *i;      // left reference
    struct u *j;      // right reference
    T_REFW *last_ref; // on the end of using list
    T_REFW ref;       // where used
    uint32_t def;     // where defined
    char k;           // kren feature:      '00'B - kren no
                      //                    '01'B - left kren
                      //                    '10'B - right kren
    char *id;         // identifier
} T_U;

extern T_U *lookup(const char *idp, size_t lid);
extern void luterm(void);
extern void through(void (*prog)(const T_U *));
extern void uns_sto(void);

#endif
