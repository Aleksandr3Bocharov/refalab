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

static bool lgcl(void);
static void rflist(T_LINKCB *par, size_t n);

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
        const T_LINKCB *first_free = refal.free_memory_list_head->next;
        const bool was_collected_garbage = lgcl();
        if (was_collected_garbage)
        {
            const T_LINKCB *linkcb_free_memory = refal.free_memory_list_head->next;
            while (linkcb_free_memory != first_free && collected_garbage_count != 1000)
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
    rflist(new_block_free_memory + 1, 1000);
    return true;
}

//  check a number of items in free items list
bool lrqlk(size_t l)
{
    const T_LINKCB *p = refal.free_memory_list_head;
    for (size_t n = 0; n < l; n++)
    {
        p = p->next;
        if (p == refal.free_memory_list_head)
            return false;
    }
    return true;
}

bool lins(T_LINKCB *p, size_t l)
{
    if (l == 0)
        return true;
    T_LINKCB *q1 = refal.free_memory_list_head;
    for (size_t n = 0; n < l; n++)
    {
        q1 = q1->next;
        if (q1 == refal.free_memory_list_head)
            return false;
        q1->tag = TAGO;
        q1->info.code = NULL;
    }
    T_LINKCB *r = q1->next;
    T_LINKCB *q = refal.free_memory_list_head->next;
    refal.free_memory_list_head->next = r;
    r->previous = refal.free_memory_list_head;
    T_LINKCB *p1 = p->next;
    q1->next = p1;
    p1->previous = q1;
    p->next = q;
    q->previous = p;
    return true;
}

bool slins(T_LINKCB *p, size_t k)
{
    while (!lrqlk(k))
        if (!more_free_memory())
        {
            refal.upshot = 3;
            return false;
        }
    return lins(p, k);
}

bool linskd(T_STATUS_TABLE *ast, uint8_t *f)
{
    if (!lexist(ast))
        refal_abort_end("linskd: process doesn't exist still");
    if (ast->dot != NULL)
        refal_abort_end("linskd: there are '<'-signes in view field");
    if (!slins(ast->view, 3))
        return false;
    T_LINKCB *p = ast->view->next;
    T_LINKCB *r = p->next;
    T_LINKCB *q = ast->view->previous;
    p->tag = TAGK;
    q->tag = TAGD;
    q->info.codep = p;
    r->tag = TAGF;
    r->info.codef = f;
    ast->dot = q;
    return true;
}

void rfinit(void)
{
    refal_init = false;
    T_REFAL *p = &refal;
    p->last_status_table = (T_STATUS_TABLE *)&refal;
    p->first_status_table = (T_STATUS_TABLE *)&refal;
    p->upshot = 1;
    p->current_status_table = NULL;
    p->static_boxes = NULL;
    p->dynamic_boxes = NULL;
    p->free_memory_list_head = &free_memory_list_head;
    T_LINKCB *phd = &free_memory_list_head;
    phd->previous = phd;
    phd->next = phd;
    phd->tag = TAGO;
    phd->info.code = NULL;
    p->arg.argc = gargc;
    p->arg.argv = gargv;
    p->timer.mode = options.timer_on;
    if (p->timer.mode)
        timespec_get(&p->timer.start_time, TIME_UTC);
    return;
}

void rfcanc(const T_STATUS_TABLE *ast)
{
    if (refal_init)
        rfinit();
    if (!lexist(ast))
        refal_abort_end("rfcanc: process doesn't exist");
    if (ast->state == 4)
        refal_abort_end("rfcanc: process is in job yet");
    ast->previous->next = ast->next;
    ast->next->previous = ast->previous;
    T_LINKCB *flhead1 = refal.free_memory_list_head->previous;
    T_LINKCB *view1 = ast->view->previous;
    T_LINKCB *store1 = ast->store->previous;
    flhead1->next = ast->view;
    ast->view->previous = flhead1;
    view1->next = ast->store;
    ast->store->previous = view1;
    store1->next = refal.free_memory_list_head;
    refal.free_memory_list_head->previous = store1;
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
        rfinit();
    bool lack = false;
    if (!more_free_memory())
        lack = true;
    else if (!lcre(&s_st))
        lack = true;
    else if (!linskd(&s_st, func))
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
            rfcanc(&s_st);
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
        rfinit();
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

static void mark(T_LINKCB *root)
{
    T_LINKCB *p = root;
    T_LINKCB *h = p;
    while (true)
    {
        T_LINKCB *q;
        if (p->next != h)
        {
            p = p->next;
            if (p->tag != TAGR)
                continue;
            q = p->info.codep;
            if (q->tag != TAGO)
                continue;
            q->tag = 0xFFFF;
            p->info.codep = h;
            q->previous = p;
            p = q;
            h = p;
            continue;
        }
        if (h == root)
            return;
        q = h->previous;
        h->previous = p;
        T_LINKCB *r = h;
        h = q->info.codep;
        q->info.codep = r;
        q->tag = TAGR;
        p = q;
    }
}

static bool lgcl(void)
{
    T_LINKCB hdvar;
    T_LINKCB *hdp = &hdvar;
    if (refal.dynamic_boxes == NULL)
        return false;
    // mark boxes achieved from view field & burriage
    bool was_coll = false;
    const T_LINKCB *pzero = NULL;
    const T_STATUS_TABLE *p = refal.first_status_table;
    while (p != (T_STATUS_TABLE *)&refal)
    {
        mark(p->view);
        mark(p->store);
        p = p->next;
    }
    // mark boxes achieved from static boxes
    if (refal.static_boxes != NULL)
    {
        T_LINKCB *r = refal.static_boxes;
        do
        {
            mark(r);
            r = r->info.codep;
        } while (r != pzero);
    }
    //   remove garbage
    hdp->info.codep = refal.dynamic_boxes;
    T_LINKCB *p1 = hdp;
    T_LINKCB *q = refal.dynamic_boxes;
    do
    {
        if (q->tag != TAGO)
        { // box isn't removed
            q->tag = TAGO;
            p1 = q;
        }
        else
        { // remove box
            was_coll = true;
            p1->info.code = q->info.code;
            p1->tag = q->tag;
            r = q->previous;
            T_LINKCB *flhead1 = refal.free_memory_list_head->next;
            r->next = flhead1;
            flhead1->previous = r;
            refal.free_memory_list_head->next = q;
            q->previous = refal.free_memory_list_head;
        }
        q = p1->info.codep;
    } while (q != pzero);
    if (hdp->info.codep == pzero)
        refal.dynamic_boxes = NULL;
    else
        refal.dynamic_boxes = hdp->info.codep;
    return was_coll;
}

static void rflist(T_LINKCB *par, size_t n)
{
    if (refal_init)
        rfinit();
    T_LINKCB *q = par;
    T_LINKCB *p = refal.free_memory_list_head->previous;
    for (size_t k = 0; k < n; k++)
    {
        p->next = q;
        q->previous = p;
        q->tag = TAGO;
        q->info.code = NULL;
        p = q;
        q++;
    }
    p->next = refal.free_memory_list_head;
    refal.free_memory_list_head->previous = p;
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
