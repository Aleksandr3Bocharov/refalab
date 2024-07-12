//-----------  file  --  RFINTF.C ------------------ 
//           C-interface functions                   
//        Last modification : 11.07.2024             
//-------------------------------------------------- 
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "refal.def"
#include "rfintf.h"
#include "rfrun1.h"

REFAL refal;

static linkcb *last_block = NULL;
static bool rf_init = true;
static unsigned int curr_size = 0;
static linkcb hd;

static bool lgcl();
static void rflist(linkcb *par, unsigned int n);

void rfabe(const char *amsg)
{
    printf("\n *** refal-abend *** %s", amsg);
    exit(1);
}

bool lincrm()
{
    unsigned int n;
    if (last_block != NULL)
    {
        const linkcb *first_free = refal.flhead->next;
        const bool was_coll = lgcl();
        if (was_coll)
        {
            const linkcb *p = refal.flhead->next;
            n = 0;
            while ((p != first_free) && (n != 1000))
            {
                n++;
                p = p->next;
            }
            if (n == 1000)
                return true;
        }
    }
    linkcb *new_block = malloc(1001 * sizeof(linkcb)); // kras 06.12.88  
#ifdef mdebug
    printf("\nLincrm: n=%d after new_block=%lx", n, new_block);
#endif
    if (new_block == NULL)
        return false;
    new_block->prev = last_block;
    last_block = new_block;
    curr_size = curr_size + 1000; // kras 06.12.88  
    rflist(new_block + 1, 1000);  // kras 06.12.88  
    return true;
}

//  check a number of items in free items list  
bool lrqlk(unsigned int l)
{
    const linkcb *p = refal.flhead;
    for (unsigned int n = 0; n < l; n++)
    {
        p = p->next;
        if (p == refal.flhead)
            return false;
    }
    return true;
}

bool lins(linkcb *p, unsigned int l)
{
    if (l < 1)
        return true;
    linkcb *q1 = refal.flhead;
    for (unsigned int n = 0; n < l; n++)
    {
        q1 = q1->next;
        if (q1 == refal.flhead)
            return false;
        q1->tag = TAGO;
        q1->info.codep = NULL;
    }
    linkcb *r = q1->next;
    linkcb *q = refal.flhead->next;
    refal.flhead->next = r;
    r->prev = refal.flhead;
    linkcb *p1 = p->next;
    q1->next = p1;
    p1->prev = q1;
    p->next = q;
    q->prev = p;
    return true;
}

bool slins(linkcb *p, unsigned int k)
{
    while (!lrqlk(k))
    {
        if (!lincrm())
        {
            refal.upshot = 3;
            return false;
        }
    }
    return lins(p, k);
}

bool linskd(st *ast, const unsigned char *f)
{
    if (!lexist(ast))
        rfabe("Linskd: process doesn't exist still");
    if (ast->dot != NULL)
        rfabe("Linskd: there are 'k'-signes in view field");
    if (!slins(ast->view, 3))
        return false;
    linkcb *p = ast->view->next;
    linkcb *r = p->next;
    linkcb *q = ast->view->prev;
    p->tag = TAGK;
    q->tag = TAGD;
    q->info.codep = p;
    r->tag = TAGF;
    r->info.codep = (linkcb *)f;
    ast->dot = q;
    return true;
}

char rfcnv(char cm)
{
    const int j = cm;
    if ((j > 96) && (j < 123))
        return cm - '\40';
    else
        return cm; // perewod  a..z w A..Z  
}

void rfinit()
{
    rf_init = false;
    REFAL *p = &refal;
    p->crprev = (st *)&refal;
    p->crnext = (st *)&refal;
    p->upshot = 1;
    p->currst = NULL;
    p->svar = NULL;
    p->dvar = NULL;
    p->flhead = &hd;
    linkcb *phd = &hd;
    phd->prev = phd;
    phd->next = phd;
    phd->tag = 0;
    phd->info.codep = NULL;
    p->nostm = 0;
    p->stmnmb = 0;
    p->tmmode = 0;
    p->tmintv = 0;
}

void rfcanc(const st *ast)
{
    if (rf_init)
        rfinit();
    if (!lexist(ast))
        rfabe("Rfcanc: process doesn't exist");
    if (ast->state == 4)
        rfabe("Rfcanc: process is in job yet");
    ast->stprev->stnext = ast->stnext;
    ast->stnext->stprev = ast->stprev;
    linkcb *flhead1 = refal.flhead->prev;
    linkcb *view1 = ast->view->prev;
    linkcb *store1 = ast->store->prev;
    flhead1->next = ast->view;
    ast->view->prev = flhead1;
    view1->next = ast->store;
    ast->store->prev = view1;
    store1->next = refal.flhead;
    refal.flhead->prev = store1;
}

//    delete part of list and add it to free memory list  
void rfdel(linkcb *p, linkcb *q)
{
    linkcb *p1 = p->next;
    if (p1 == q)
        return;
    linkcb *q1 = q->prev;
    linkcb *r = refal.flhead->prev;
    p->next = q;
    q->prev = p;
    q1->next = refal.flhead;
    refal.flhead->prev = q1;
    r->next = p1;
    p1->prev = r;
}

void rftermm()
{
    while (last_block != NULL)
    {
        linkcb *new_block = last_block;
        last_block = new_block->prev;
        free(new_block);
#ifdef mdebug
        printf("\nLincrm: free new_block=%lx", new_block);
#endif
    }
}

void rfexec(const unsigned char *func)
{
    st s_st;
    if (rf_init)
        rfinit();
    if (!lincrm())
        goto LACK;
    if (!lcre(&s_st))
        goto LACK;
    if (!linskd(&s_st, func))
        goto LACK;
    s_st.stop = 0x7FFFFFFFL;
AGAIN:

// BLF 17.07.2004  
#ifdef mdebug
    /*	step by step execution with full debug trace information
       see refal-2 user manual (3.14 - example of processing control) */
    while (s_st.state == 1 && s_st.dot != NULL)
    {
        s_st.stop = s_st.step + 1;
        const linkcb *pk = s_st.dot->info.codep;
        const linkcb *prevk = pk->prev;
        const linkcb *nextd = s_st.dot->next;
        printf("\n step: %d", s_st.stop);
        rfpexm(" Term: ", prevk, nextd);

        rfrun(&s_st);

        if (s_st.state == 1)
            rfpexm(" Result: ", prevk, nextd);
    }
#else
    // no debug info  
    rfrun(&s_st);
    if (s_st.state == 3)
        if (lincrm())
            goto AGAIN;
    if (s_st.dot != NULL)
        goto ABEND;
#endif
    // BLF -- end correction 17.07.2004  

DONE:
    printf("\nConcretization is executed");
    goto EOJ;
ABEND:
    switch (s_st.state)
    {
    case 1:
        printf("\nStop on step number");
        break;
    case 2:
        printf("\nRecognition impossible");
        break;
    case 3:
        printf("\nFree memory exhausted");
    }
EOJ:
    printf("\nTotal steps number = %ld", s_st.step);
    if (s_st.view->next != s_st.view)
    {
        printf("\nView field:");
        rfpexm("            ", s_st.view, s_st.view);
    }
    if (s_st.store->next != s_st.store)
    {
        printf("\nBurried:");
        rfpexm("         ", s_st.store, s_st.store);
    }
    rfcanc(&s_st);
    rftermm();

    // BLF  
    printf("\n");

    return;
LACK:
    printf("\nNo ehough memory for initialization");
    rftermm();
}

void rfpexm(const char *pt, const linkcb *pr, const linkcb *pn)
{
    printf("\n%s", pt);
    bool fr = false;
    while (pr != pn->prev)
    {
        const linkcb *pr1 = pr;
        pr = pr->next;
        if (pr->prev != pr1)
            rfabe("rfpexm: list structure violation");
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
                putchar('k');
            else if (pr->tag == TAGD)
                putchar('.');
            else if (pr->tag == TAGLB)
                putchar('(');
            else if (pr->tag == TAGRB)
                putchar(')');
            else if (pr->tag == TAGN)
                printf("/%ld/", gcoden(pr));
            else if (pr->tag == TAGF)
            {
                putchar('/');
                const char *f = (char *)(pr->info.codef - 1);
                const unsigned char l = *f;
                f -= l;
                for (unsigned int k = 1; k <= l; k++, f++)
                    putchar(rfcnv(*f));
                putchar('/');
            }
            else if (pr->tag == TAGR)
                printf("/%%%lx/", pr->info.codep);
            else if ((pr->tag & 0001) != 0)
                rfabe("rfpexm: unknown bracket type ");
            else
                printf("/<%x>,%lx/", pr->tag, pr->info.codep);
        }
    }
    if (fr)
        putchar('\'');
    return;
}

void rftpl(linkcb *r, linkcb *p, linkcb *q)
{
    linkcb *p1 = p->next;
    if (p1 == q)
        return;
    linkcb *r1 = r->next;
    linkcb *q1 = q->prev;
    p->next = q;
    q->prev = p;
    q1->next = r1;
    r1->prev = q1;
    r->next = p1;
    p1->prev = r;
}

//  copy expression and add it to nessecary place   
bool lcopy(linkcb *r, const linkcb *p, const linkcb *q)
{
    linkcb *f = refal.flhead;
    linkcb *f0 = p->next;
    linkcb *f1;
    while (f0 != q)
    {
        f = f->next;
        if (f == refal.flhead)
            return false;
        switch (f0->tag)
        {
        case TAGLB:
            linkcb *lastb = NULL;
            f->info.codep = lastb;
            lastb = f;
            break;
        case TAGRB:
            f->info.codep = lastb;
            f->tag = TAGRB;
            if (lastb != NULL)
                f1 = lastb->info.codep;
            else
                return false;
            lastb->info.codep = f;
            lastb->tag = TAGLB;
            lastb = f1;
            break;
        default:
            f->tag = f0->tag;
            //   pcoden(f,gcoden(f0));  
        }
        f0 = f0->next;
    }
    if (refal.flhead == f)
        return true;
    f0 = refal.flhead->next;
    f1 = f->next;
    refal.flhead->next = f1;
    f1->prev = refal.flhead;
    linkcb *r1 = r->next;
    f->next = r1;
    r1->prev = f;
    r->next = f0;
    f0->prev = r;
    return true;
}

bool lexist(const st *ast)
{
    const REFAL *p = &refal;
    do
    {
        p = (REFAL *)(p->crnext);
        if (p == (REFAL *)ast)
            return true;
    } while (p != &refal);
    return false;
}

bool lcre(st *ast)
{
    if (rf_init)
        rfinit();
    if (lexist(ast))
        rfabe("Lcre: process already exists");
    ast->view = refal.flhead->next;
    if (ast->view == refal.flhead)
        return false;
    ast->store = ast->view->next;
    if (ast->store == refal.flhead)
        return false;
    linkcb *flhead1 = ast->store->next;
    refal.flhead->next = flhead1;
    flhead1->prev = refal.flhead;
    (ast->view->next) = (ast->view);
    (ast->view->prev) = (ast->view);
    (ast->store->next) = (ast->store);
    (ast->store->prev) = (ast->store);
    st *q = refal.crprev;
    ast->stnext = (st *)&refal;
    refal.crprev = ast;
    q->stnext = ast;
    ast->stprev = q;
    ast->state = 1;
    ast->dot = NULL;
    ast->step = 0L;
    ast->stop = -1L;
    return true;
}

static void mark(linkcb *root)
{
    linkcb *p = root;
    linkcb *h = p;
MRK:
    if (p->next == h)
        goto UP;
    p = p->next;
    if (p->tag != TAGR)
        goto MRK;
    linkcb *q = p->info.codep;
    if (q->tag != 0)
        goto MRK;
    q->tag = 0xFFFF;
    p->info.codep = h;
    q->prev = p;
    h = p = q;
    goto MRK;
UP:
    if (h == root)
        return;
    q = h->prev;
    h->prev = p;
    linkcb *r = h;
    h = q->info.codep;
    q->info.codep = r;
    q->tag = TAGR;
    p = q;
    goto MRK;
}

static bool lgcl()
{
    linkcb hdvar;
    linkcb *hd = &hdvar;
    if (refal.dvar == NULL)
        return false;
    // mark boxes achieved from view field & burriage  
    bool was_coll = false;
    const linkcb *pzero = NULL;
    const st *p = refal.crnext;
    while (p != (st *)&refal)
    {
        mark(p->view);
        mark(p->store);
        p = p->stnext;
    }
    // mark boxes achieved from static boxes     
    if (refal.svar != NULL)
    {
        linkcb *r = refal.svar;
        do
        {
            mark(r);
            r = r->info.codep;
        } while (r != pzero);
        //   remove garbage    
        hd->info.codep = refal.dvar;
        linkcb *p1 = hd;
        linkcb *q = refal.dvar;
        do
        {
            if (q->tag != 0)
            { // box isn't removed  
                q->tag = 0;
                p1 = q;
            }
            else
            { // remove box      
                was_coll = true;
                p1->info.codep = q->info.codep;
                p1->tag = q->tag;
                r = q->prev;
                linkcb *flhead1 = refal.flhead->next;
                r->next = flhead1;
                flhead1->prev = r;
                refal.flhead->next = q;
                q->prev = refal.flhead;
            }
            q = p1->info.codep;
        } while (q != pzero);
        if (hd->info.codep == pzero)
            refal.dvar = NULL;
        else
            refal.dvar = hd->info.codep;
    }
    return was_coll;
}

static void rflist(linkcb *par, unsigned int n)
{
    if (rf_init)
        rfinit();
    linkcb *q = par;
    linkcb *p = refal.flhead->prev;
    for (unsigned int k = 0; k < n; k++)
    {
        p->next = q;
        q->prev = p;
        q->tag = 0;
        q->info.codep = NULL;
        p = q;
        q++;
    }
    p->next = refal.flhead;
    refal.flhead->prev = p;
    return;
}

void rfpex(const char *pt, const linkcb *pr, const linkcb *pn)
{
    printf("\n%s", pt);
    while (pr != pn->prev)
    {
        const linkcb *pr1 = pr;
        pr = pr->next;
        if (pr1 != pr->prev)
            rfabe("rfpex: list structure violation");
        if (pr->tag == TAGO)
            putchar(pr->info.infoc);
        else if (pr->tag == TAGK)
            putchar('k');
        else if (pr->tag == TAGD)
            putchar('.');
        else if (pr->tag == TAGLB)
            putchar('(');
        else if (pr->tag == TAGRB)
            putchar(')');
        else if (pr->tag == TAGN)
            printf("'%ld'", gcoden(pr));
        else if (pr->tag == TAGF)
        {
            putchar('\'');
            const char *f = (char *)(pr->info.codef - 1);
            const unsigned char l = *f;
            f -= l;
            for (unsigned char k = 1; k <= l; k++, f++)
                putchar(rfcnv(*f));
            putchar('\'');
        }
        else if (pr->tag == TAGR)
            printf("'%%%lx'", pr->info.codep);
        else if ((pr->tag & 0001) != 0)
            rfabe("rfpex: unknown bracket type ");
        else
            printf("'%x,%lx'", pr->tag, pr->info.codep);
    }
    return;
}

linkcb *lldupl(const linkcb *p, const linkcb *q, const linkcb *u)
{
    const linkcb *x = p->next;
    linkcb *y = u->next;
    while (x != q)
    {
        if (x->tag != y->tag)
            return 0;
        if (x->info.codef != y->info.codef)
            if ((x->tag != TAGLB) && (x->tag != TAGRB))
                return 0;
        x = x->next;
        y = y->next;
    }
    return y;
}
//----------- end of file  RFINTF.C ------------ 
