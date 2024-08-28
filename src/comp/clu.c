//----------------  file  --  CLU.C  -------------------
//    AVL-tree construction for identifier table
//           Last edition date : 11.07.24
//------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "refal.def"
#include "clu.h"
#include "plc.h"
#include "refal.h"

static T_U *korenj = NULL; // tree koren

void uns_sto()
{
    printf("\nNo memory for identifier table");
    exit(1);
    return;
}

static T_U *nov_uzel(const char *idp, size_t lid)
{
    T_U *p = (T_U *)calloc(1, sizeof(T_U));
#ifdef mdebug
    printf("\ncalloc(clu-nov_uzel): p=%p l=%zu t=%o", (void *)p, p->l, p->type);
#endif
    if (p == NULL)
        uns_sto();
    p->i = NULL;
    p->j = NULL;
    p->k = '\000';
    p->mode = '\000';
    p->type = '\000';
    p->last_ref = &p->ref;
    p->ref.next = NULL;
    for (size_t m = 1; m <= 5; m++)
        p->ref.numb[m] = 0;
    p->ref.numb[0] = scn_.nomkar;
    p->def = 0;
    char *q = calloc(1, lid);
#ifdef mdebug
    printf("\ncalloc(clu-id): q=%p l=%zu", q, lid);
#endif
    if (q == NULL)
        uns_sto();
    p->id = q;
    strncpy(q, idp, lid);
    p->l = lid;
    return p;
}

T_U *lookup(const char *idp, size_t lid)
// lid identifier length
{
    T_U *isk_uz;
    if (korenj == NULL)
    { // empty tree
        korenj = nov_uzel(idp, lid);
        isk_uz = korenj;
        return isk_uz;
    }
    // tree is't empty,begin push.
    // remember path in stack
    size_t tgld = 0; // current  tree depth
    T_U *p = korenj;
    T_U *q;
    char kren;
    T_U *adruz[36]; // stack for tree work
    char otnuz[36];
    while (true)
    { // search step
        do
        {
            if (strncmp(idp, p->id, lid < p->l ? lid : p->l) == 0)
            {
                if (lid == p->l)
                { // include usage number to list
                    T_REFW *q1 = p->last_ref;
                    size_t k = 5;
                    while (q1->numb[k] == 0)
                        k--;
                    if (q1->numb[k] != scn_.nomkar)
                    {
                        // include number to list
                        if (q1->numb[5] == 0)
                            // it's free field in current item
                            q1->numb[k + 1] = scn_.nomkar;
                        else
                        { // create new item
                            T_REFW *r1 = (T_REFW *)calloc(1, sizeof(T_REFW));
#ifdef mdebug
                            printf("\ncalloc(clu-lookup): r1=%p", (void *)r1);
#endif
                            if (r1 == NULL)
                                uns_sto();
                            q1->next = r1;
                            p->last_ref = q1->next;
                            r1->next = NULL;
                            for (k = 0; k <= 5; k++)
                                r1->numb[k] = 0;
                            r1->numb[0] = scn_.nomkar;
                        };
                    }
                    while ((p->mode & '\300') == '\300')
                        p = p->info.infop;
                    return p;
                }
                else
                {
                    if (lid > p->l)
                        kren = '\100';
                    else
                        kren = '\200';
                    break;
                }
            }
            if (strncmp(idp, p->id, lid < p->l ? lid : p->l) > 0)
                kren = '\100';
            else
                kren = '\200';
        } while (false);
        adruz[tgld] = p;
        otnuz[tgld] = kren;
        tgld++;
        // step down in tree
        if (kren == '\100')
            q = p->j;
        else
            q = p->i;
        if (q != NULL)
        {
            p = q;
            continue;
        };
        break;
    }
    // include new node to tree
    isk_uz = nov_uzel(idp, lid);
    q = isk_uz;
    if (kren == '\100')
        p->j = q;
    else
        p->i = q;
    // necessary node is new
    while (true)
    { // move up and correct
        // balance  features
        tgld--;
        p = adruz[tgld];
        kren = p->k;
        if (kren == '\000')
        {
            p->k = otnuz[tgld];
            if (tgld != 0)
                continue;
            return isk_uz;
        };
        break;
    }
    // in this point kren != '\000'
    if (kren != otnuz[tgld])
    {
        p->k = '\000';
        return isk_uz;
    };
    // tree turn
    // if kren = '\100' -- left turn
    // if kren = '\200' -- right turn
    if (kren == '\100')
        q = p->j;
    else
        q = p->i;
    T_U *verquz;
    if (kren == q->k)
    {
        if (kren == '\100')
        { // once turn
            p->j = q->i;
            q->i = p;
        }
        else
        {
            p->i = q->j;
            q->j = p;
        };
        q->k = '\000';
        p->k = q->k;
        verquz = q;
    }
    else
    { // twos turn
        T_U *r;
        if (kren == '\100')
        {
            r = q->i;
            p->j = r->i;
            q->i = r->j;
            r->i = p;
            r->j = q;
        }
        else
        {
            r = q->j;
            p->i = r->j;
            q->j = r->i;
            r->j = p;
            r->i = q;
        };
        const char nruk = (r->k == '\0') & '\300';
        if (r->k == '\000')
        {
            p->k = '\000';
            q->k = p->k;
        }
        else if (nruk == kren)
        {
            p->k = '\000';
            q->k = nruk;
        }
        else
        {
            q->k = '\000';
            p->k = nruk;
        };
        r->k = '\000';
        verquz = r;
    }; // end of twos turn
    // correct upper reference
    if (tgld == 0)
        korenj = verquz;
    else
    {
        tgld--;
        if (otnuz[tgld] == '\100')
            adruz[tgld]->j = verquz;
        else
            adruz[tgld]->i = verquz;
    };
    return isk_uz;
}

static void traverse(const T_U *ptr, void (*prog)(const T_U *))
{
    const T_U *q = ptr;
    do
    {
        const T_U *r = q->i;
        if (r != NULL)
            traverse(r, prog);
        (*prog)(q);
        q = q->j;
    } while (q != NULL);
    return;
}

void through(void (*prog)(const T_U *))
{
    if (korenj != NULL)
        traverse(korenj, prog);
    return;
}

static void kil_tree(T_U *p)
{
    T_U *q = p;
    do
    {
        T_U *r = q->i;
        if (r != NULL)
            kil_tree(r);
        T_REFW *r2 = q->ref.next;
        while (r2 != NULL)
        {
            T_REFW *r1 = r2->next;
#ifdef mdebug
            printf("\nfree(clu): r2=%p", (void *)r2);
#endif
            free(r2);
            r2 = r1;
        }
        r = q->j;
#ifdef mdebug
        printf("\nfree(clu):id=%p", q->id);
        printf("\n           q=%p", (void *)q);
#endif
        free(q->id);
        free(q);
        q = r;
    } while (q != NULL);
    return;
}

void luterm()
{
    if (korenj != NULL)
    {
        kil_tree(korenj);
        korenj = NULL;
    }
    return;
}
//-----------  end of file CLU.C  ----------
