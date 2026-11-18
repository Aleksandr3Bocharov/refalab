// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-11-17
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file avl_identifiers.h  -----------
//    AVL-tree construction for identifier table
//-----------------------------------------------

#ifndef AVL_IDENTIFIERS_H
#define AVL_IDENTIFIERS_H

#include <stddef.h>
#include <stdint.h>
#include "refalab.h"

// Positions of an identifier in the source text, kept in chunks of 6 positions
// (cursor_numbers[] is filled from the beginning, the unused tail is zero).
typedef struct usage_list
{
    struct usage_list *next;  // on the next usage list
    size_t cursor_numbers[6]; // usage list element
} T_USAGE_LIST;

// An identifier (a function or a specifier name) and everything the compiler knows
// about it: a node of the AVL tree that serves as the identifier table.
typedef struct label
{
    union
    {
        size_t infon;
        struct label *infop;
    } info;
    uint8_t mode;
    // mode field (octal masks, the label kind is mode & 0300):
    //  0000 - not defined;
    //  0100 - internal; infon = offset from the start of the module
    //  0200 - external; infon = the external reference (member of an extern chain)
    //  0300 - equivalent (EQU); infop = reference to the other label
    //  0020 - the label is defined (a second definition is error 504)
    //  0040 - entry point
    //
    uint8_t type; // type field (octal masks, set by the first use of the identifier):
                  //  0100 - function
                  //  0200 - specifier
                  //  0300 - used as both (error 502)
    T_USAGE_LIST *last_usage_list;          // on the end of usage list
    T_USAGE_LIST usage_list;                // where used
    size_t cursor_number_defined;           // where defined
    char identifier[MAX_IDENTIFIER_LENGTH]; // identifier
    uint8_t identifier_length;              // identifier length
    struct label *left_label;               // left reference
    struct label *right_label;              // right reference
    uint8_t balance;                        // AVL balance feature: 0000 - balanced,
                                            //                      0100 - right subtree is deeper,
                                            //                      0200 - left subtree is deeper

} T_LABEL;

// Identifier table: lookup (with insertion), traversal and release.
extern T_LABEL *resolve_label_alias(T_LABEL *label);
extern T_LABEL *lookup_label(const char *identifier, uint8_t identifier_length, size_t identifier_cursor_number);
extern void labels_terminate(void);
extern void through_labels(void (*handler)(T_LABEL *));
extern void error_no_memory_labels(void);

#endif
