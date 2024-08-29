//-------------- file -- XVV.C ------------
//           MO: file input/output
//      Last edition date : 11.07.24
//-----------------------------------------
#include <stdio.h>
#include <stdbool.h>
#include "refal.def"
#include "rfintf.h"

#define fmax 5

static FILE *inr, *inw;
static FILE *uniput[fmax], *uniget[fmax];
static size_t junp = 1;
static size_t jung = 1;
static uint32_t jl;

static void opng_(void)
{
    size_t i;
    char namf[40];
    for (i = 0; i < 40; i++)
        namf[i] = '\0';
    const T_LINKCB *p = refal.preva->next;
    if (p->tag != TAGN)
        jl = 0; // jung zamenila na jl
    else
    {
        jl = p->info.coden;
        p = p->next;
    }
    bool heot1 = false;
    do
    {
        if (jl >= fmax)
            break;
        jung = jl;
        bool heot = false;
        for (i = 0; p != refal.nexta; i++)
        {
            if (p->tag != TAGO || i == 39)
            {
                heot = true;
                break;
            }
            namf[i] = p->info.infoc;
            p = p->next;
        }
        if (heot)
            break;
        inr = fopen(namf, "r");
        // printf("\n opn, namf=%s flg=%d fd=%d",namf,inr->flags,inr->fd);
        if (inr == NULL)
        {
            printf("\nopnget: can't open file %s", namf);
            heot1 = true;
            break;
        }
        uniget[jung] = inr;
        return;
    } while (false);
    if (!heot1)
        printf("\nopnget: format error");
    refal.upshot = 2;
    return;
}
void (*opng_1)(void) = opng_;
G_L_B char opnget = '\122';
char opng_0[] = {Z6 'O', 'P', 'N', 'G', 'E', 'T', '\006'};

static void opnp_(void)
{
    size_t i;
    char namf[40];
    for (i = 0; i < 40; i++)
        namf[i] = '\0';
    const T_LINKCB *p = refal.preva->next;
    const char *m = "w";
    if (p->tag == TAGO && p->info.infoc == '*')
    {
        m = "a";
        p = p->next;
    }
    if (p->tag != TAGN)
        jl = 0;
    else
    {
        jl = p->info.coden;
        p = p->next;
    }
    bool heot1 = false;
    do
    {
        if (jl >= fmax)
            break;
        junp = jl;
        bool heot = false;
        for (i = 0; p != refal.nexta; i++)
        {
            if (p->tag != TAGO || i == 40)
            {
                heot = true;
                break;
            }
            namf[i] = p->info.infoc;
            p = p->next;
        }
        if (heot)
            break;
        inw = fopen(namf, m);
        // printf("\n opnput, namf=%s mode=%s",namf,m);
        if (inw == NULL)
        {
            printf("\nopnput: can't open file %s", namf);
            heot1 = true;
            break;
        }
        uniput[junp] = inw;
        return;
    } while (false);
    if (!heot1)
        printf("\nopnput: format error");
    refal.upshot = 2;
    return;
}
void (*opnp_1)(void) = opnp_;
G_L_B char opnput = '\122';
char opnp_0[] = {Z6 'O', 'P', 'N', 'P', 'U', 'T', '\006'};

static void clsg_(void)
{
    const T_LINKCB *p = refal.preva->next;
    if (p->tag != TAGN)
        jl = 0;
    else
    {
        jl = p->info.coden;
        p = p->next;
    }
    if (jl >= fmax)
    {
        printf("\nclsget: format error");
        refal.upshot = 2;
        return;
    }
    jung = jl;
    inr = uniget[jung];
    // printf("\n cls, flg=%d fd=%d",inr->flags,inr->fd);
    fclose(inr);
    return;
}
void (*clsg_1)(void) = clsg_;
G_L_B char clsget = '\122';
char clsg_0[] = {Z6 'C', 'L', 'S', 'G', 'E', 'T', '\006'};

static void clsp_(void)
{
    const T_LINKCB *p = refal.preva->next;
    if (p->tag != TAGN)
        jl = 0;
    else
    {
        jl = p->info.coden;
        p = p->next;
    }
    if (jl >= fmax)
    {
        printf("\nclsput: format error");
        refal.upshot = 2;
        return;
    }
    junp = jl;
    inw = uniput[junp];
    // printf("\n cls, flg=%d fd=%d",inw->flags,inw->fd);
    fclose(inw);
    return;
}
void (*clsp_1)(void) = clsp_;
G_L_B char clsput = '\122';
char clsp_0[] = {Z6 'C', 'L', 'S', 'P', 'U', 'T', '\006'};

static void libg_(void)
{
    T_LINKCB *p = refal.preva->next;
    bool new = false;
    if (p->tag != TAGN)
    {
        jl = 0;
        new = true;
    }
    else
    {
        jl = p->info.coden;
        p = p->next;
    }
    if (jl >= fmax)
    {
        printf("\nlibget: format error");
        refal.upshot = 2;
        return;
    }
    jung = jl;
    inr = uniget[jung];
    char s[128];
    s[0] = ' ';
    int c = getc(inr);
    size_t i;
    for (i = 0; c != '\n' && c != EOF && i < 128; i++)
    {
        s[i] = (char)c;
        c = getc(inr);
    }
    if (c == EOF)
        return;
    size_t j;
    if (new)
    { // sovmestimost s ES
        p = refal.prevr;
        if (!slins(p, 80))
            return;
        for (j = 0; j < i; j++)
        {
            p = p->next;
            p->tag = TAGO;
            p->info.codep = NULL;
            p->info.infoc = s[j];
        }
        for (j = i; j < 80; j++)
        {
            p = p->next;
            p->tag = TAGO;
            p->info.codep = NULL;
            p->info.infoc = ' ';
        }
        return;
    }
    if (i < 128)
        i--;
    for (; i > 1; i--)
        if (s[i] != ' ')
        {
            j = i + 1;
            if (j > 128)
                j = 128;
            p = refal.prevr;
            for (i = 0; i < j; i++)
            {
                if (!slins(p, 1))
                    return;
                p = p->next;
                p->info.codep = NULL;
                if (s[0] == EOF)
                {
                    p->tag = TAGN;
                    return;
                }
                p->tag = TAGO;
                p->info.infoc = s[i];
            }
            return;
        }
}
void (*libg_1)(void) = libg_;
G_L_B char libget = '\122';
char libg_0[] = {Z6 'L', 'I', 'B', 'G', 'E', 'T', '\006'};

static void libp_(void)
{
    const T_LINKCB *p = refal.preva->next;
    if (p->tag != TAGN)
        jl = 0;
    else
    {
        jl = p->info.coden;
        p = p->next;
    }
    do
    {
        if (jl >= fmax)
            break;
        junp = jl;
        inw = uniput[junp];
        int c;
        size_t i;
        bool heot = false;
        for (i = 0; p != refal.nexta; i++)
        {
            if (p->tag != TAGO && p->tag != TAGLB && p->tag != TAGRB)
            {
                heot = true;
                break;
            }
            if (i == 128)
            {
                c = '\n';
                putc(c, inw);
                i = 0;
            }
            switch (p->tag)
            {
            case TAGLB:
                c = '(';
                break;
            case TAGRB:
                c = ')';
                break;
            default:
                c = p->info.infoc;
            }
            p = p->next;
            putc(c, inw);
        }
        if (heot)
            break;
        if (i != 0)
        {
            c = '\n';
            putc(c, inw);
        }
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
void (*libp_1)(void) = libp_;
G_L_B char libput = '\122';
char libp_0[] = {Z6 'L', 'I', 'B', 'P', 'U', 'T', '\006'};

static void card_(void)
{
    if (refal.preva->next != refal.nexta) // refal.upshot = 2;
        rfpex("", refal.preva, refal.nexta);
    T_LINKCB *p = refal.prevr;
    int c = getchar();
    while (c != '\n')
    {
        if  (!slins(p, 1))
            return;
        p = p->next;
        p->info.codep = NULL;
        if (c == EOF)
        {
            p->tag = TAGN;
            return;
        }
        p->tag = TAGO;
        p->info.infoc = (char)c;
        c = getchar();
    }
    return;
}
void (*card_1)(void) = card_;
G_L_B char card = '\122';
char card_0[] = {Z4 'C', 'A', 'R', 'D', '\004'};

static void pr1_(void)
{
    rfpex("", refal.preva, refal.nexta);
    rftpl(refal.prevr, refal.preva, refal.nexta);
    return;
}
void (*pr1_1)(void) = pr1_;
G_L_B char print = '\122';
char pr1_0[] = {Z5 'P', 'R', 'I', 'N', 'T', '\005'};

static void pr2_(void)
{
    rfpexm("", refal.preva, refal.nexta);
    rftpl(refal.prevr, refal.preva, refal.nexta);
    return;
}
void (*pr2_1)(void) = pr2_;
G_L_B char printm = '\122';
char pr2_0[] = {Z6 'P', 'R', 'I', 'N', 'T', 'M', '\006'};

static void pr3_(void)
{
    rfpex("", refal.preva, refal.nexta);
    return;
}
void (*pr3_1)(void) = pr3_;
G_L_B char prout = '\122';
char pr3_0[] = {Z5 'P', 'R', 'O', 'U', 'T', '\005'};

static void pr4_(void)
{
    rfpexm("", refal.preva, refal.nexta);
    return;
}
void (*pr4_1)(void) = pr4_;
G_L_B char proutm = '\122';
char pr4_0[] = {Z6 'P', 'R', 'O', 'U', 'T', 'M', '\006'};

//------------------ end of file  XVV.C ----------------
