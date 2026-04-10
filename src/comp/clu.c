// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-04-09
// https://github.com/Aleksandr3Bocharov/refalab

//----------------  file  --  CLU.C  -------------------
//    AVL-tree construction for identifier table
//------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "refalab.h"
#include "clu.h"
#include "refal.h"

static T_LABEL *root = NULL; // tree root

void error_no_memory_labels(void)
{
    printf("\nNo memory for identifier table\n");
    exit(1);
    return;
}

static T_LABEL *new_label(const char *identifier, uint8_t identifier_length)
{
    T_LABEL *label = (T_LABEL *)calloc(1, sizeof(T_LABEL));
#if defined mdebug
    fprintf(stderr, "calloc(clu): label=%p identifier_length=%u type=%o\n", (void *)label, label->identifier_length, label->type);
#endif
    if (label == NULL)
        error_no_memory_labels();
    label->left_label = NULL;
    label->right_label = NULL;
    label->balance = 0;
    label->mode = 0;
    label->type = 0;
    label->last_usage_list = &label->usage_list;
    label->usage_list.next = NULL;
    for (uint8_t i = 1; i <= 5; i++)
        label->usage_list.carriage_numbers[i] = 0;
    label->usage_list.carriage_numbers[0] = scanner.carriage_number;
    label->carriage_number_defined = 0;
    strncpy(label->identifier, identifier, identifier_length);
    label->identifier_length = identifier_length;
    return label;
}

T_LABEL *lookup_label(const char *identifier, uint8_t identifier_length)
// identifier_length identifier length
{
    if (root == NULL)
    { // empty tree
        root = new_label(identifier, identifier_length);
        return root;
    }
    // tree is't empty,begin push.
    // remember path in stack
    T_LABEL *search_label;
    uint8_t current_tree_depth = 0; // current  tree depth
    T_LABEL *label = root;
    T_LABEL *temp_label;
    uint8_t balance;
    T_LABEL *adruz[45]; // stack for tree work
    uint8_t otnuz[45];
    while (true)
    { // search step
        do
        {
            if (strncmp(identifier, label->identifier, identifier_length < label->identifier_length ? identifier_length : label->identifier_length) == 0)
            {
                if (identifier_length == label->identifier_length)
                { // include usage number to list
                    T_USAGE_LIST *q1 = label->last_usage_list;
                    size_t k = 5;
                    while (q1->carriage_numbers[k] == 0)
                        k--;
                    if (q1->carriage_numbers[k] != scanner.carriage_number)
                    {
                        // include number to list
                        if (q1->carriage_numbers[5] == 0)
                            // it's free field in current item
                            q1->carriage_numbers[k + 1] = scanner.carriage_number;
                        else
                        { // create new item
                            T_USAGE_LIST *r1 = (T_USAGE_LIST *)calloc(1, sizeof(T_USAGE_LIST));
#if defined mdebug
                            fprintf(stderr, "calloc(clu): r1=%p\n", (void *)r1);
#endif
                            if (r1 == NULL)
                                error_no_memory_labels();
                            q1->next = r1;
                            label->last_usage_list = q1->next;
                            r1->next = NULL;
                            for (k = 0; k <= 5; k++)
                                r1->carriage_numbers[k] = 0;
                            r1->carriage_numbers[0] = scanner.carriage_number;
                        };
                    }
                    while ((label->mode & 0300) == 0300)
                        label = label->info.infop;
                    return label;
                }
                else
                {
                    if (identifier_length > label->identifier_length)
                        balance = 0100;
                    else
                        balance = 0200;
                    break;
                }
            }
            if (strncmp(identifier, label->identifier, identifier_length < label->identifier_length ? identifier_length : label->identifier_length) > 0)
                balance = 0100;
            else
                balance = 0200;
        } while (false);
        adruz[current_tree_depth] = label;
        otnuz[current_tree_depth] = balance;
        current_tree_depth++;
        // step down in tree
        if (balance == 0100)
            temp_label = label->right_label;
        else
            temp_label = label->left_label;
        if (temp_label != NULL)
        {
            label = temp_label;
            continue;
        };
        break;
    }
    // include new node to tree
    search_label = new_label(identifier, identifier_length);
    temp_label = search_label;
    if (balance == 0100)
        label->right_label = temp_label;
    else
        label->left_label = temp_label;
    // necessary node is new
    while (true)
    { // move up and correct
        // balance  features
        current_tree_depth--;
        label = adruz[current_tree_depth];
        balance = label->balance;
        if (balance == 0)
        {
            label->balance = otnuz[current_tree_depth];
            if (current_tree_depth != 0)
                continue;
            return search_label;
        };
        break;
    }
    // in this point balance != 0
    if (balance != otnuz[current_tree_depth])
    {
        label->balance = 0;
        return search_label;
    };
    // tree turn
    // if balance = 0100 -- left turn
    // if balance = 0200 -- right turn
    if (balance == 0100)
        temp_label = label->right_label;
    else
        temp_label = label->left_label;
    T_LABEL *verquz;
    if (balance == temp_label->balance)
    {
        if (balance == 0100)
        { // once turn
            label->right_label = temp_label->left_label;
            temp_label->left_label = label;
        }
        else
        {
            label->left_label = temp_label->right_label;
            temp_label->right_label = label;
        };
        temp_label->balance = 0;
        label->balance = temp_label->balance;
        verquz = temp_label;
    }
    else
    { // twos turn
        T_LABEL *r;
        if (balance == 0100)
        {
            r = temp_label->left_label;
            label->right_label = r->left_label;
            temp_label->left_label = r->right_label;
            r->left_label = label;
            r->right_label = temp_label;
        }
        else
        {
            r = temp_label->right_label;
            label->left_label = r->right_label;
            temp_label->right_label = r->left_label;
            r->right_label = label;
            r->left_label = temp_label;
        };
        const uint8_t nruk = (r->balance == 0) & 0300;
        if (r->balance == 0)
        {
            label->balance = 0;
            temp_label->balance = label->balance;
        }
        else if (nruk == balance)
        {
            label->balance = 0;
            temp_label->balance = nruk;
        }
        else
        {
            temp_label->balance = 0;
            label->balance = nruk;
        };
        r->balance = 0;
        verquz = r;
    }; // end of twos turn
    // correct upper reference
    if (current_tree_depth == 0)
        root = verquz;
    else
    {
        current_tree_depth--;
        if (otnuz[current_tree_depth] == 0100)
            adruz[current_tree_depth]->right_label = verquz;
        else
            adruz[current_tree_depth]->left_label = verquz;
    };
    return search_label;
}

static void traverse(const T_LABEL *ptr, void (*prog)(const T_LABEL *))
{
    const T_LABEL *q = ptr;
    do
    {
        const T_LABEL *r = q->left_label;
        if (r != NULL)
            traverse(r, prog);
        (*prog)(q);
        q = q->right_label;
    } while (q != NULL);
    return;
}

void through_labels(void (*handler)(const T_LABEL *))
{
    if (root != NULL)
        traverse(root, handler);
    return;
}

static void kil_tree(T_LABEL *p)
{
    T_LABEL *q = p;
    do
    {
        T_LABEL *r = q->left_label;
        if (r != NULL)
            kil_tree(r);
        T_USAGE_LIST *r2 = q->usage_list.next;
        while (r2 != NULL)
        {
            T_USAGE_LIST *r1 = r2->next;
#if defined mdebug
            fprintf(stderr, "free(clu): r2=%p\n", (void *)r2);
#endif
            free(r2);
            r2 = r1;
        }
        r = q->right_label;
#if defined mdebug
        fprintf(stderr, "free(clu): q=%p\n", (void *)q);
#endif
        free(q);
        q = r;
    } while (q != NULL);
    return;
}

void labels_terminate(void)
{
    if (root != NULL)
    {
        kil_tree(root);
        root = NULL;
    }
    return;
}

//-----------  end of file CLU.C  ----------
