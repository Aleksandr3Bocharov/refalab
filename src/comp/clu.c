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

static T_LABEL *korenj = NULL; // tree koren

void error_no_memory_labels(void)
{
    printf("\nNo memory for identifier table\n");
    exit(1);
    return;
}

static T_LABEL *nov_uzel(const char *idp, size_t lid)
{
    T_LABEL *p = (T_LABEL *)calloc(1, sizeof(T_LABEL));
#if defined mdebug
    fprintf(stderr, "calloc(clu): p=%p identifier_length=%u type=%o\n", (void *)p, p->identifier_length, p->type);
#endif
    if (p == NULL)
        error_no_memory_labels();
    p->left_label = NULL;
    p->right_label = NULL;
    p->balance = 0;
    p->mode = 0;
    p->type = 0;
    p->last_usage_list = &p->usage_list;
    p->usage_list.next = NULL;
    for (size_t m = 1; m <= 5; m++)
        p->usage_list.carriage_numbers[m] = 0;
    p->usage_list.carriage_numbers[0] = scanner.carriage_number;
    p->carriage_number_defined = 0;
    strncpy(p->identifier, idp, lid);
    p->identifier_length = lid;
    return p;
}

T_LABEL *lookup_label(const char *identifier, uint8_t identifier_length)
// identifier_length identifier length
{
    T_LABEL *isk_uz;
    if (korenj == NULL)
    { // empty tree
        korenj = nov_uzel(identifier, identifier_length);
        isk_uz = korenj;
        return isk_uz;
    }
    // tree is't empty,begin push.
    // remember path in stack
    size_t tgld = 0; // current  tree depth
    T_LABEL *p = korenj;
    T_LABEL *q;
    char kren;
    T_LABEL *adruz[36]; // stack for tree work
    char otnuz[36];
    while (true)
    { // search step
        do
        {
            if (strncmp(identifier, p->identifier, identifier_length < p->identifier_length ? identifier_length : p->identifier_length) == 0)
            {
                if (identifier_length == p->identifier_length)
                { // include usage number to list
                    T_USAGE_LIST *q1 = p->last_usage_list;
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
                            p->last_usage_list = q1->next;
                            r1->next = NULL;
                            for (k = 0; k <= 5; k++)
                                r1->carriage_numbers[k] = 0;
                            r1->carriage_numbers[0] = scanner.carriage_number;
                        };
                    }
                    while ((p->mode & 0300) == 0300)
                        p = p->info.infop;
                    return p;
                }
                else
                {
                    if (identifier_length > p->identifier_length)
                        kren = 0100;
                    else
                        kren = 0200;
                    break;
                }
            }
            if (strncmp(identifier, p->identifier, identifier_length < p->identifier_length ? identifier_length : p->identifier_length) > 0)
                kren = 0100;
            else
                kren = 0200;
        } while (false);
        adruz[tgld] = p;
        otnuz[tgld] = kren;
        tgld++;
        // step down in tree
        if (kren == 0100)
            q = p->right_label;
        else
            q = p->left_label;
        if (q != NULL)
        {
            p = q;
            continue;
        };
        break;
    }
    // include new node to tree
    isk_uz = nov_uzel(identifier, identifier_length);
    q = isk_uz;
    if (kren == 0100)
        p->right_label = q;
    else
        p->left_label = q;
    // necessary node is new
    while (true)
    { // move up and correct
        // balance  features
        tgld--;
        p = adruz[tgld];
        kren = p->balance;
        if (kren == 0)
        {
            p->balance = otnuz[tgld];
            if (tgld != 0)
                continue;
            return isk_uz;
        };
        break;
    }
    // in this point kren != 0
    if (kren != otnuz[tgld])
    {
        p->balance = 0;
        return isk_uz;
    };
    // tree turn
    // if kren = 0100 -- left turn
    // if kren = 0200 -- right turn
    if (kren == 0100)
        q = p->right_label;
    else
        q = p->left_label;
    T_LABEL *verquz;
    if (kren == q->balance)
    {
        if (kren == 0100)
        { // once turn
            p->right_label = q->left_label;
            q->left_label = p;
        }
        else
        {
            p->left_label = q->right_label;
            q->right_label = p;
        };
        q->balance = 0;
        p->balance = q->balance;
        verquz = q;
    }
    else
    { // twos turn
        T_LABEL *r;
        if (kren == 0100)
        {
            r = q->left_label;
            p->right_label = r->left_label;
            q->left_label = r->right_label;
            r->left_label = p;
            r->right_label = q;
        }
        else
        {
            r = q->right_label;
            p->left_label = r->right_label;
            q->right_label = r->left_label;
            r->right_label = p;
            r->left_label = q;
        };
        const char nruk = (r->balance == 0) & 0300;
        if (r->balance == 0)
        {
            p->balance = 0;
            q->balance = p->balance;
        }
        else if (nruk == kren)
        {
            p->balance = 0;
            q->balance = nruk;
        }
        else
        {
            q->balance = 0;
            p->balance = nruk;
        };
        r->balance = 0;
        verquz = r;
    }; // end of twos turn
    // correct upper reference
    if (tgld == 0)
        korenj = verquz;
    else
    {
        tgld--;
        if (otnuz[tgld] == 0100)
            adruz[tgld]->right_label = verquz;
        else
            adruz[tgld]->left_label = verquz;
    };
    return isk_uz;
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
    if (korenj != NULL)
        traverse(korenj, handler);
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
        fprintf(stderr, "free(clu): identifier=%p\n", (void *)q->identifier);
        fprintf(stderr, "           q=%p\n", (void *)q);
#endif
        free(q->identifier);
        free(q);
        q = r;
    } while (q != NULL);
    return;
}

void labels_terminate(void)
{
    if (korenj != NULL)
    {
        kil_tree(korenj);
        korenj = NULL;
    }
    return;
}

//-----------  end of file CLU.C  ----------
