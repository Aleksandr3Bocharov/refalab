// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-04-14
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
    if (!lexist(status_table))
        refal_abort_end("insert_view_k_function_dot: process doesn't exist still");
    if (status_table->dot != NULL)
        refal_abort_end("insert_view_k_function_dot: there are '<'-signes in view field");
    if (!extended_insert_from_free_memory_list(status_table->view, 3))
        return false;
    T_LINKCB *k = status_table->view->next;
    T_LINKCB *symbol_label = k->next;
    if (symbol_label->next != status_table->view->previous)
        rftpl(status_table->view->previous, symbol_label, symbol_label->next->next);
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
    if (!lexist(status_table))
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
void rfdel(T_LINKCB *p, T_LINKCB *q)
{
    T_LINKCB *p1 = p->next;
    if (p1 == q)
        return;
    T_LINKCB *q1 = q->previous;
    T_LINKCB *r = refal.free_memory_list_head->previous;
    p->next = q;
    q->previous = p;
    q1->next = refal.free_memory_list_head;
    refal.free_memory_list_head->previous = q1;
    r->next = p1;
    p1->previous = r;
    return;
}

void rftermm(void)
{
    while (last_block_free_memory != NULL)
    {
        T_LINKCB *new_block = last_block_free_memory;
        last_block_free_memory = new_block->previous;
#if defined mdebug
        fprintf(stderr, "more_free_memory: free new_block=%p\n", (void *)new_block);
#endif
        free(new_block);
    }
}

void rfexec(uint8_t *func)
{
    T_STATUS_TABLE s_st;
    if (refal_init)
        refal_initiate();
    bool lack = false;
    if (!more_free_memory())
        lack = true;
    else if (!lcre(&s_st))
        lack = true;
    else if (!insert_view_k_function_d(&s_st, func))
        lack = true;
    if (lack)
    {
        printf("\nNo enough memory for initialization\n");
        rftermm();
        return;
    }
    s_st.stop = MAX_STOP;
#if defined mdebug
    const uint32_t s_stop = s_st.stop;
#endif
    enum
    {
        AGAIN,
        DONE,
        ABEND,
        EOJ,
        RET
    } ex_state = AGAIN;
    while (true)
        switch (ex_state)
        {
        case AGAIN:
#if defined mdebug
            //	step by step execution with full debug trace information
            if (s_st.dot == NULL)
            {
                ex_state = DONE;
                break;
            }
            if (s_st.state != 1 || s_st.step >= s_stop)
            {
                bool go = false;
                if (s_st.state == 3)
                    if (more_free_memory())
                        go = true;
                if (s_st.step >= s_stop)
                {
                    s_st.step = 0;
                    go = true;
                }
                if (!go)
                {
                    ex_state = ABEND;
                    break;
                }
            }
            s_st.stop = s_st.step + 1;
            const T_LINKCB *pk = s_st.dot->info.codep;
            const T_LINKCB *prevk = pk->previous;
            const T_LINKCB *nextd = s_st.dot->next;
            printf(" Step: %d\n", s_st.stop);
            rfpexm(" Term: ", prevk, nextd, true);
            rfrun(&s_st);
            if (s_st.state == 1)
                rfpexm(" Result: ", prevk, nextd, true);
            break;
#else
            // no debug info
            rfrun(&s_st);
            if (s_st.state == 3)
                if (more_free_memory())
                    break;
            if (s_st.state == 1 && s_st.dot != NULL)
            {
                s_st.step = 0;
                break;
            }
            if (s_st.dot != NULL)
            {
                ex_state = ABEND;
                break;
            }
            ex_state = DONE;
            break;
#endif
        case DONE:
#if defined mdebug
            printf("Concretization is executed\n");
            ex_state = EOJ;
#else
            ex_state = RET;
#endif
            break;
        case ABEND:
            switch (s_st.state)
            {
            case 2:
                printf("Recognition impossible\n");
                break;
            case 3:
                printf("Free memory exhausted\n");
            }
            ex_state = EOJ;
            break;
        case EOJ:
            printf("Total steps number = %u\n", s_st.step);
            if (s_st.view->next != s_st.view)
            {
                printf("View field:\n");
                rfpexm("            ", s_st.view, s_st.view, true);
            }
            if (s_st.store->next != s_st.store)
            {
                printf("Burried:\n");
                rfpexm("         ", s_st.store, s_st.store, true);
            }
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
            ex_state = RET;
            break;
        case RET:
            delete_status_table(&s_st);
            rftermm();
            return;
        }
}

void rfpex(const char *pt, const T_LINKCB *pr, const T_LINKCB *pn, const bool nl)
{
    printf("%s", pt);
    while (pr != pn->previous)
    {
        const T_LINKCB *pr1 = pr;
        pr = pr->next;
        if (pr1 != pr->previous)
            refal_abort_end("rfpex: list structure violation");
        if (pr->tag == TAGO)
            putchar(pr->info.infoc);
        else if (pr->tag == TAGK)
            putchar('<');
        else if (pr->tag == TAGD)
            putchar('>');
        else if (pr->tag == TAGLB)
            putchar('(');
        else if (pr->tag == TAGRB)
            putchar(')');
        else if (pr->tag == TAGN)
            printf("'%u'", gcoden(pr));
        else if (pr->tag == TAGF)
        {
            putchar('\'');
            const uint8_t *lp = pr->info.codef - 1;
            const uint8_t l = *lp;
            const char *f = (char *)lp - l;
            for (uint8_t k = 1; k <= l; k++, f++)
                putchar(toupper(*f));
            putchar('\'');
        }
        else if (pr->tag == TAGR)
            printf("'%%%p'", (void *)pr->info.codep);
        else if (BRACKET(pr))
            refal_abort_end("rfpex: unknown bracket type");
        else
            printf("'%x,%p'", pr->tag, pr->info.code);
    }
    if (nl)
        printf("\n");
    return;
}

void rfpexs(const char *pt, const T_LINKCB *pr, const T_LINKCB *pn, const bool nl)
{
    printf("%s", pt);
    while (pr != pn->previous)
    {
        const T_LINKCB *pr1 = pr;
        pr = pr->next;
        if (pr1 != pr->previous)
            refal_abort_end("rfpexs: list structure violation");
        if (pr->tag == TAGO)
            putchar(pr->info.infoc);
        else if (pr->tag == TAGK)
            putchar('<');
        else if (pr->tag == TAGD)
            putchar('>');
        else if (pr->tag == TAGLB)
            putchar('(');
        else if (pr->tag == TAGRB)
            putchar(')');
        else if (pr->tag == TAGN)
            printf("%u", gcoden(pr));
        else if (pr->tag == TAGF)
        {
            const uint8_t *lp = pr->info.codef - 1;
            const uint8_t l = *lp;
            const char *f = (char *)lp - l;
            for (uint8_t k = 1; k <= l; k++, f++)
                putchar(toupper(*f));
        }
        else if (pr->tag == TAGR)
            printf("%%%p", (void *)pr->info.codep);
        else if (BRACKET(pr))
            refal_abort_end("rfpexs: unknown bracket type");
        else
            printf("%x,%p", pr->tag, pr->info.code);
    }
    if (nl)
        printf("\n");
    return;
}

void rfpexm(const char *pt, const T_LINKCB *pr, const T_LINKCB *pn, const bool nl)
{
    printf("%s", pt);
    bool fr = false;
    while (pr != pn->previous)
    {
        const T_LINKCB *pr1 = pr;
        pr = pr->next;
        if (pr->previous != pr1)
            refal_abort_end("rfpexm: list structure violation");
        if (pr->tag == TAGO)
        {
            if (!fr)
            {
                fr = true;
                putchar('\'');
            };
            putchar(pr->info.infoc);
        }
        else
        {
            if (fr)
            {
                fr = false;
                putchar('\'');
            };
            if (pr->tag == TAGK)
                printf("< ");
            else if (pr->tag == TAGD)
                putchar('>');
            else if (pr->tag == TAGLB)
                putchar('(');
            else if (pr->tag == TAGRB)
                putchar(')');
            else if (pr->tag == TAGN)
            {
                printf("%u", gcoden(pr));
                if (pr->next->tag == TAGN)
                    putchar(' ');
            }
            else if (pr->tag == TAGF)
            {
                putchar('&');
                const uint8_t *lp = pr->info.codef - 1;
                const uint8_t l = *lp;
                const char *f = (char *)lp - l;
                for (uint8_t k = 1; k <= l; k++, f++)
                    putchar(toupper(*f));
                if (pr->next->tag == TAGN)
                    putchar(' ');
            }
            else if (pr->tag == TAGR)
                printf("/%%%p/", (void *)pr->info.codep);
            else if (BRACKET(pr))
                refal_abort_end("rfpexm: unknown bracket type");
            else
                printf("/%x,%p/", pr->tag, pr->info.code);
        }
    }
    if (fr)
        putchar('\'');
    if (nl)
        printf("\n");
    return;
}

void rftpl(T_LINKCB *r, T_LINKCB *p, T_LINKCB *q)
{
    T_LINKCB *p1 = p->next;
    if (p1 == q)
        return;
    T_LINKCB *r1 = r->next;
    T_LINKCB *q1 = q->previous;
    p->next = q;
    q->previous = p;
    q1->next = r1;
    r1->previous = q1;
    r->next = p1;
    p1->previous = r;
    return;
}

//  copy expression and add it to nessecary place
bool lcopy(T_LINKCB *r, const T_LINKCB *p, const T_LINKCB *q)
{
    T_LINKCB *f = refal.free_memory_list_head;
    T_LINKCB *f0 = p->next;
    T_LINKCB *f1, *lastb = NULL;
    while (f0 != q)
    {
        f = f->next;
        if (f == refal.free_memory_list_head)
            return false;
        switch (f0->tag)
        {
        case TAGLB:
            f->info.codep = lastb;
            lastb = f;
            break;
        case TAGRB:
            f->info.codep = lastb;
            f->tag = TAGRB;
            f1 = lastb->info.codep;
            lastb->info.codep = f;
            lastb->tag = TAGLB;
            lastb = f1;
            break;
        default:
            f->tag = f0->tag;
            f->info.code = f0->info.code;
        }
        f0 = f0->next;
    }
    if (refal.free_memory_list_head == f)
        return true;
    f0 = refal.free_memory_list_head->next;
    f1 = f->next;
    refal.free_memory_list_head->next = f1;
    f1->previous = refal.free_memory_list_head;
    T_LINKCB *r1 = r->next;
    f->next = r1;
    r1->previous = f;
    r->next = f0;
    f0->previous = r;
    return true;
}

bool lexist(const T_STATUS_TABLE *ast)
{
    const T_REFAL *p = &refal;
    do
    {
        p = (T_REFAL *)(p->first_status_table);
        if (p == (T_REFAL *)ast)
            return true;
    } while (p != &refal);
    return false;
}

bool lcre(T_STATUS_TABLE *ast)
{
    if (refal_init)
        refal_initiate();
    if (lexist(ast))
        refal_abort_end("lcre: process already exists");
    ast->view = refal.free_memory_list_head->next;
    if (ast->view == refal.free_memory_list_head)
        return false;
    ast->store = ast->view->next;
    if (ast->store == refal.free_memory_list_head)
        return false;
    T_LINKCB *flhead1 = ast->store->next;
    refal.free_memory_list_head->next = flhead1;
    flhead1->previous = refal.free_memory_list_head;
    ast->view->next = ast->view;
    ast->view->previous = ast->view;
    ast->store->next = ast->store;
    ast->store->previous = ast->store;
    T_STATUS_TABLE *q = refal.last_status_table;
    ast->next = (T_STATUS_TABLE *)&refal;
    refal.last_status_table = ast;
    q->next = ast;
    ast->previous = q;
    ast->state = 1;
    ast->dot = NULL;
    ast->step = 0;
    ast->stop = MAX_STOP;
    return true;
}

static void mark_dynamic_box_heads(T_LINKCB *root)
{
    T_LINKCB *linkcb = root;
    T_LINKCB *head = linkcb;
    while (true)
    {
        T_LINKCB *dynamic_box_head;
        if (linkcb->next != head)
        {
            linkcb = linkcb->next;
            if (linkcb->tag != TAGR)
                continue;
            dynamic_box_head = linkcb->info.codep;
            if (dynamic_box_head->tag != TAGO)
                continue;
            dynamic_box_head->tag = 0xFFFF;
            linkcb->info.codep = head;
            dynamic_box_head->previous = linkcb;
            linkcb = dynamic_box_head;
            head = linkcb;
            continue;
        }
        if (head == root)
            return;
        T_LINKCB *symbol_reference = head->previous;
        head->previous = linkcb;
        dynamic_box_head = head;
        head = symbol_reference->info.codep;
        symbol_reference->info.codep = dynamic_box_head;
        symbol_reference->tag = TAGR;
        linkcb = symbol_reference;
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
    T_LINKCB *linkcb_free_memory_list = refal.free_memory_list_head->previous;
    for (size_t i = 0; i < size_block_free_memory; i++)
    {
        linkcb_free_memory_list->next = linkcb_block_free_memory;
        linkcb_block_free_memory->previous = linkcb_free_memory_list;
        linkcb_block_free_memory->tag = TAGO;
        linkcb_block_free_memory->info.code = NULL;
        linkcb_free_memory_list = linkcb_block_free_memory;
        linkcb_block_free_memory++;
    }
    linkcb_free_memory_list->next = refal.free_memory_list_head;
    refal.free_memory_list_head->previous = linkcb_free_memory_list;
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
