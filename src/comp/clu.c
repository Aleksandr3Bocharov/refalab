/*----------------  file  --  CLU.C  -------------------*/
/*    AVL-tree construction for identifier table        */
/*           Last edition date : 15.06.24               */
/*------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "refal.def"
#include "plc.h"
#include "clu.h"

struct refw
{
    struct refw *next; /* on the next usage list */
    int numb[6];       /* usage list element     */
};
typedef struct refw T_REFW;

struct u
{
    union
    {
        int infon;
        struct u *infop;
    } info;
    char mode;
    /* mode field :                      */
    /*  00 - no defined;                          */
    /*  01 - internal; infon = offset from start  */
    /*  10 - external; infon = member or extern   */
    /*       reference;                           */
    /*  11 - equivalent; infop =  reference on    */
    /*       other label;                         */
    /*  xx1 - entry point;                        */
    /*  xxx1 - too many definition;               */
    /*                                            */
    char type;             /* type field : 00 - unknown type  */
                           /*              01 - function      */
                           /*              10 - specifier     */
    int l;                 /* identifier length */
    struct u *i;           /* left reference */
    struct u *j;           /* right reference */
    struct refw *last_ref; /* on the end of using list */
    struct refw ref;       /* where used */
    int def;               /* where defined */
    char k;                /* kren feature:      '00'B - kren no    */
                           /*                    '01'B - left kren  */
                           /*                    '10'B - right kren */
    char *id;              /* identifier */
};
typedef struct u T_U;

extern struct
{
    int nomkar;
    char modname_var[40];
    int modnmlen;
    int curr_stmnmb;
} scn_;

static struct u *korenj = NULL; /* tree koren */

void uns_sto()
{
    printf("\nNo memory for identifier table");
    exit(1);
}

T_U *nov_uzel(char *idp, int lid)
{
    int m;
    T_U *p;
    char *q;
    p = (T_U *)calloc(1, sizeof(T_U));
#ifdef mdebug
    printf("\ncalloc(clu-nov_uzel): p=%lx l=%d t=%o", p, p->l, p->type);
#endif
    if (p == NULL)
        uns_sto();
    p->i = p->j = NULL;
    p->k = '\000';
    p->mode = '\000';
    p->type = '\000';
    p->last_ref = &(p->ref);
    p->ref.next = NULL;
    for (m = 1; m <= 5; m++)
        p->ref.numb[m] = 0;
    p->ref.numb[0] = scn_.nomkar;
    p->def = 0;
    q = calloc(1, lid);
#ifdef mdebug
    printf("\ncalloc(clu-id): q=%lx l=%d", q, lid);
#endif
    if (q == NULL)
        uns_sto();
    p->id = q;
    strncpy(q, idp, lid);
    p->l = lid;
    return p;
}

struct u *lookup(char *idp, int lid)
/* lid identifier length */
{
    struct refw *r1, *q1;
    struct u *p, *q, *r, *isk_uz;
    int k;
    struct u *verquz;
    char kren, nruk;
    struct u *adruz[36]; /* stack for tree work */
    int otnuz[36];
    int tgld; /* current  tree depth */
    if (korenj == NULL)
    { /* empty tree */
        korenj = nov_uzel(idp, lid);
        isk_uz = korenj;
        return isk_uz;
    }
    /* tree is't empty,begin push. */
    /* remember path in stack */
    tgld = 0;
    p = korenj;
SHAG: /* search step */
    if (strncmp(idp, p->id, (lid < p->l) ? lid : p->l) == 0)
    {
        if (lid == p->l)
        { /* include usage number to list */
            q1 = (*p).last_ref;
            k = 5;
            while ((*q1).numb[k] == 0)
                k--;
            if ((*q1).numb[k] != scn_.nomkar)
            {
                /* include number to list */
                if ((*q1).numb[5] == 0)
                    /* it's free field in current item */
                    (*q1).numb[k + 1] = scn_.nomkar;
                else
                { /* create new item */
                    r1 = (T_REFW *)calloc(1, sizeof(T_REFW));
#ifdef mdebug
                    printf("\ncalloc(clu-lookup): r1=%lx", r1);
#endif
                    if (r1 == NULL)
                        uns_sto();
                    (*p).last_ref = (*q1).next = r1;
                    (*r1).next = NULL;
                    for (k = 0; k <= 5; k++)
                        (*r1).numb[k] = 0;
                    (*r1).numb[0] = scn_.nomkar;
                };
            }
            while ((((*p).mode) & '\300') == '\300')
                p = (*p).info.infop;
            return p;
        }
        else
        {
            if (lid > p->l)
                kren = '\100';
            else
                kren = '\200';
            goto FINT;
        }
    }
    if (cmpstr(lid < p->l ? lid : p->l, idp, p->id) > 0)
        kren = '\100';
    else
        kren = '\200';
FINT:
    adruz[tgld] = p;
    otnuz[tgld] = kren;
    tgld++;
    /* step down in tree */
    if (kren == '\100')
        q = (*p).j;
    else
        q = (*p).i;
    if (q != NULL)
    {
        p = q;
        goto SHAG;
    };
    /* include new node to tree */
    isk_uz = nov_uzel(idp, lid);
    q = isk_uz;
    if (kren == '\100')
        (*p).j = q;
    else
        (*p).i = q;
    /* necessary node is new */
ISPRB: /* move up and correct */
    /* balance  features */
    tgld--;
    p = adruz[tgld];
    kren = (*p).k;
    if (kren == '\000')
    {
        (*p).k = otnuz[tgld];
        if (tgld != 0)
            goto ISPRB;
        return isk_uz;
    };
    /* in this point kren != '\000' */
    if (kren != otnuz[tgld])
    {
        (*p).k = '\000';
        return isk_uz;
    };
    /* tree turn */
    /* if kren = '\100' -- left turn */
    /* if ( kren = '\200' -- right turn */
    if (kren == '\100')
        q = (*p).j;
    else
        q = (*p).i;
    if (kren == (*q).k)
    {
        if (kren == '\100')
        { /* once turn */
            (*p).j = (*q).i;
            (*q).i = p;
        }
        else
        {
            (*p).i = (*q).j;
            (*q).j = p;
        };
        (*p).k = (*q).k = '\000';
        verquz = q;
    }
    else
    { /* twos turn */
        if (kren == '\100')
        {
            r = (*q).i;
            (*p).j = (*r).i;
            (*q).i = (*r).j;
            (*r).i = p;
            (*r).j = q;
        }
        else
        {
            r = (*q).j;
            (*p).i = (*r).j;
            (*q).j = (*r).i;
            (*r).j = p;
            (*r).i = q;
        };
        nruk = (!((*r).k)) & '\300';
        if ((*r).k == '\000')
            (*q).k = (*p).k = '\000';
        else if (nruk == kren)
        {
            (*p).k = '\000';
            (*q).k = nruk;
        }
        else
        {
            (*q).k = '\000';
            (*p).k = nruk;
        };
        (*r).k = '\000';
        verquz = r;
    }; /* end of twos turn */
    /* correct upper reference */
    if (tgld == 0)
        korenj = verquz;
    else
    {
        tgld--;
        if (otnuz[tgld] == '\100')
            (*adruz[tgld]).j = verquz;
        else
            (*adruz[tgld]).i = verquz;
    };
    return isk_uz;
}

static void traverse(struct u *ptr, void (*prog)(struct u *))
{
    struct u *q, *r;
    q = ptr;
    do
    {
        r = (*q).i;
        if (r != NULL)
            traverse(r, prog);
        (*prog)(q);
        q = (*q).j;
    } while (q != NULL);
    return;
}

void through(void (*prog)(struct u *))
{
    if (korenj != NULL)
        traverse(korenj, prog);
    return;
}

static void kil_tree(struct u *p)
{
    struct u *r, *q;
    struct refw *r1, *r2;
    q = p;
    do
    {
        r = (*q).i;
        if (r != NULL)
            kil_tree(r);
        r2 = (*q).ref.next;
        while (r2 != NULL)
        {
            r1 = (*r2).next;
#ifdef mdebug
            printf("\nfree(clu): r2=%lx", r2);
#endif
            free(r2);
            r2 = r1;
        }
        r = (*q).j;
        free(q->id);
        free(q);
#ifdef mdebug
        printf("\nfree(clu):id=%lx", q->id);
        printf("\n           q=%lx", q);
#endif
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
/*-----------  end of file CLU.C  ----------*/
