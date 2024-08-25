//-----------------  file  --  cj.C  -------------------
//             generator of assembler text
//           Last edition date :  17.07.24
//------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include "refal.def"
#include "cj.h"
#include "cerr.h"
#include "refal.h"
#include "clu.h"

typedef struct ent
{ // entry table element
    struct ent *next;
    T_U *p;
    char e[8];
    size_t le;
} T_ENT;

typedef struct ext
{ // external pointer table element
    struct ext *next;
    T_U *p;
    char e[8];
    size_t le;
    // unsigned int noms;
} T_EXT;

typedef struct rl
{
    T_U *point;
    uint16_t delta;
} T_RL;

typedef struct BU_
{
    size_t len;
    size_t tek;
    char *nam;
    char *buf;
    FILE *fil;
} BU;

static BU sysut1 = {0, 0, NULL, NULL, NULL};
static BU sysut2 = {0, 0, NULL, NULL, NULL};

static union
{
    char b[2];
    uint16_t w;
} d;

/*
static int curr_r;
static int new_r;
static int equal_r; // feature that new_r = curr_r
static char new_f[4];
static const char *ccc;
*/

static T_ENT *q, *r;
static T_EXT *qx, *rx;

/*    MODE field value:
   00 - undefined;
   01 - internal,   INFO = shift against begin;
   10 - external,   INFO = external pointer number;
   11 - equivalent, INFO = another label pointer;
   XX1 - entry;
   XXX1 - still defined.
      TYPE field value:
   00 - unknown type;
   01 - function;
   10 - specifier.
*/

static T_ENT *first_ent;
static T_ENT *last_ent;
static size_t mod_length;
static char mod_name[9];
static size_t lnmmod;
static T_EXT *first_ext;
static T_EXT *last_ext;
static size_t curr_addr; // module generation files
static size_t n_ext;
static T_RL rl;
static size_t k;
static uint16_t delta;

static void oshex()
{
    fputs("\nOSHEX: no memory!!!", stdout);
    exit(1);
    return;
}

static void sfop_w(const char *s, BU *b)
{
    if (b->nam != NULL)
    {
        free(b->nam);
#ifdef mdebug
        printf("\nfree(cj) b->nam=%p", b->nam);
#endif
    }
    b->nam = (char *)malloc(strlen(s) + 1);
    if (b->nam == NULL)
        oshex();
#ifdef mdebug
    printf("\nmalloc(cj): b->nam=%p", b->nam);
#endif
    strcpy(b->nam, s);
    size_t un = 0;
    if (b->buf == NULL)
    {
        if (options.mincomp)
        {
            if (b == &sysut2)
                un = 2040; // 2040=340*6
            else
                un = 2040; // 2040=2048-8
        }
        else
        {
            if (b == &sysut2)
                un = 49152; // 8192*6
            else
                un = 65528; // 65536-8 (for bc mojno - 4)
        }
        size_t lon;
        while (true)
        {
            b->buf = (char *)malloc(un);
            if (b->buf != NULL)
            {
#ifdef mdebug
                printf("\nmalloc(cj): b->buf=%p un=%zu", b->buf, un);
#endif
                break;
            }
            else
            {
                lon = un;
                if (b == &sysut2)
                    lon /= 2;
                else
                    lon = (lon + 8) / 2 - 8;
                un = lon;
                if (un < 16)
                    oshex();
            }
        } // while
    }
    b->tek = 0;
    b->len = un;
    b->fil = NULL;
    return;
}

static void sfop_r(BU *b)
{
    if (b->fil != NULL)
    {
        b->fil = fopen(b->nam, Rbin);
        if (b->fil == NULL)
        {
            printf("Can't open for read %s\n", b->nam);
            exit(8);
        }
        if (fread(b->buf, b->len, 1, b->fil) == 0)
        {
            printf("Read i/o error in %s\n", b->nam);
            exit(8);
        }
    }
    b->tek = 0;
    return;
}

static void sfcl(const BU *b)
{
    if (b->fil != NULL)
    {
        if (fwrite(b->buf, b->tek, 1, b->fil) == 0)
        {
            printf("Write i/o error in %s\n", b->nam);
            exit(8);
        }
        fclose(b->fil);
    }
    return;
}

static void sfclr(BU *b)
{
    if (b->fil != NULL)
        unlink(b->nam);
    free(b->nam);
    free(b->buf);
#ifdef mdebug
    printf("\nfree(sfclr) b->nam(c 0)=%p", b->nam);
    printf("\n            b->buf(c 0)=%p", b->buf);
#endif
    b->nam = NULL;
    b->buf = NULL;
    return;
}

static void sfwr2()
{
    while (true)
    {
        const size_t ost = sysut2.len - sysut2.tek;
        if (ost >= SMBL)
        {
            memcpy(sysut2.buf + sysut2.tek, &rl, SMBL);
            sysut2.tek += SMBL;
            return;
        }
        if (sysut2.fil == NULL)
        {
            sysut2.fil = fopen(sysut2.nam, Wbin);
            if (sysut2.fil == NULL)
            {
                printf("Can't open for write sysut2\n");
                exit(8);
            }
        }
        if (fwrite(sysut2.buf, sysut2.len, 1, sysut2.fil) == 0)
        {
            printf("Write i/o error in sysut2\n");
            exit(8);
        }
        sysut2.tek = 0;
    } // while
} // sfwr2

/*
static void sfwr(const char *c, size_t n, BU *b)
{
    while (true)
    {
        const size_t ost = b->len - b->tek;
        if (ost >= n)
        {
            memcpy(b->buf + b->tek, c, n);
            b->tek += n;
            return;
        }
        memcpy(b->buf + b->tek, c, ost);
        if (b->fil == NULL)
        {
            b->fil = fopen(b->nam, Wbin)
            if ((b->fil == NULL)
            {
                printf("Can't open for write %s\n", b->nam);
                exit(8);
            }
        }
        if (fwrite(b->buf, b->len, 1, b->fil) == 0)
        {
            printf("Write i/o error in %s\n", b->nam);
            exit(8);
        }
        b->tek = 0;
        n -= ost;
        c += ost;
    } // while
} // sfwr
*/

static void sfrd1(char *c, size_t n)
{
    while (true)
    {
        const size_t ost = sysut1.len - sysut1.tek;
        if (ost >= n)
        {
            memcpy(c, sysut1.buf + sysut1.tek, n);
            sysut1.tek += n;
            return;
        }
        memcpy(c, sysut1.buf + sysut1.tek, ost);
        /*if (fread(sysut1.buf, sysut1.len, 1, sysut1.fil) == 0)
        {
            // printf("Read i/o error in sysut1\n"); exit(8);
        } */
        sysut1.tek = 0;
        n -= ost;
        c += ost;
    } // while
} // sfrd1

static void sfrd2()
{
    while (true)
    {
        const size_t ost = sysut2.len - sysut2.tek;
        if (ost >= SMBL)
        {
            memcpy(&rl, sysut2.buf + sysut2.tek, SMBL);
            sysut2.tek += SMBL;
            return;
        }
        /*if (fread(sysut2.buf, sysut2.len, 1, sysut2.fil) == 0)
        {
            //printf("Read i/o error in sysut2\n");
            //exit(8);
        }*/
        sysut2.tek = 0;
    } // while
} // sfrd2

void jstart(const char *ee, size_t ll)
{
    delta = 0; // kras
    strncpy(mod_name, ee, ll);
    lnmmod = ll;
    sfop_w("sysut1.rf", &sysut1);
    sfop_w("sysut2.rf", &sysut2);
    first_ent = (T_ENT *)malloc(sizeof(T_ENT));
    if (first_ent == NULL)
        oshex();
#ifdef mdebug
    printf("\nmalloc(cj): first_ent=%p", first_ent);
#endif
    last_ent = first_ent;
    first_ent->next = NULL;
    first_ext = (T_EXT *)malloc(sizeof(T_EXT));
    if (first_ext == NULL)
        oshex();
#ifdef mdebug
    printf("\nmalloc(cj): first_ext=%p", first_ext);
#endif
    last_ext = first_ext;
    first_ext->next = NULL;
    curr_addr = 0;
    n_ext = 1;
    return;
} // jstart

size_t jwhere()
{
    if (curr_addr > 65535)
    {
        printf("Module too long\n");
        exit(1);
    }
    return curr_addr;
}

void jbyte(char bb)
{
    // sfwr(&bb,1,&sysut1);
    if (sysut1.tek != sysut1.len)
    {
        *(sysut1.buf + sysut1.tek) = bb;
        sysut1.tek++;
    }
    else
    {
        if (sysut1.fil == NULL)
        {
            sysut1.fil = fopen(sysut1.nam, Wbin);
            if (sysut1.fil == NULL)
            {
                printf("Can't open for write sysut1\n");
                exit(8);
            }
        }
        if (fwrite(sysut1.buf, sysut1.len, 1, sysut1.fil) == 0)
        {
            printf("Write i/o error in sysut1\n");
            exit(8);
        }
        *sysut1.buf = bb;
        sysut1.tek = 1;
    }
    delta++;
    curr_addr++;
    return;
} // jbyte

void j3addr(T_U *pp)
{
    rl.point = pp;
    rl.delta = delta;
    delta = 0;
    sfwr2();
    curr_addr += LBLL;
    return;
}

void jentry(T_U *pp, const char *ee, size_t ll)
// ee label
{
    // label length
    /*if( (lnmmod==ll) && (strncmp(mod_name, ee, ll < lnmmod ? ll : lnmmod)==0) )
    pchosh("520 entry point name is equal module name");*/
    r = first_ent;
    while (r != last_ent)
    {
        r = r->next;
        if (r->le == ll && strncmp(r->e, ee, ll < r->le ? ll : r->le) == 0)
            //{
            // pchose("521 two entry points has single name ", ee, ll);
            return;
        //}
    }
    r = (T_ENT *)malloc(sizeof(T_ENT));
    if (r == NULL)
        oshex();
#ifdef mdebug
    printf("\nmalloc(cj): r(ent)=%p", r);
#endif
    last_ent->next = r;
    last_ent = r;
    r->p = pp;
    r->next = NULL;
    r->le = 8 < ll ? 8 : ll;
    strncpy(r->e, ee, r->le);
    pp->mode |= '\040';
    return;
} // jentry

void jextrn(T_U *pp, const char *ee, size_t ll)
// ee label
{
    //  label length
    rx = (T_EXT *)malloc(sizeof(T_EXT));
    if (rx == NULL)
        oshex();
#ifdef mdebug
    printf("\nmalloc(cj): rx(ext)=%p", rx);
#endif
    last_ext->next = rx;
    last_ext = rx;
    rx->p = pp;
    rx->next = NULL;
    rx->le = 8 < ll ? 8 : ll;
    if (strncmp(ee, "DIV", 3) == 0 && rx->le == 3)
    {
        strcpy(rx->e, "DIV_");
        rx->le = 4;
    }
    else
        strncpy(rx->e, ee, rx->le);
    pp->mode |= '\220';
    n_ext++;
    pp->info.infon = n_ext;
    return;
} // jextrn

void jlabel(T_U *pp)
{
    pp->mode |= '\120';
    pp->info.infon = curr_addr;
    return;
}

void jequ(T_U *pp, T_U *qq)
{
    pp->info.infop = qq;
    pp->mode |= '\320';
    return;
}

static void zakon()
{
    rl.delta = delta;
    rl.point = NULL;
    sfwr2();
    sfcl(&sysut1);
    sfcl(&sysut2);
    mod_length = curr_addr;
    // if (mod_length < 0)
    //     mod_length = 65536 + mod_length;
    return;
} // zakon

void jend()
{
    zakon();
    if (options.multmod)
    {
        strcat(mod_i, ".asm");
        syslin = fopen(mod_i, "w");
        if (syslin == NULL)
        {
            printf("Can't open %s\n", mod_i);
            exit(8);
        }
    }
    d.w = 0;
    // heading generating
    fputs(".data\n", syslin); // BLF
    // BLF fputc('_',syslin); for(i=0;i<lnmmod;i++) fputc(mod_name[i],syslin);
    // BLF fputs ("\tsegment\tbyte public 'CODE'\n",syslin);
    // BLF sprintf(bufs,"_d%d@\tlabel\tbyte\n",nommod); fputs (bufs,syslin);
    // BLF
    char bufs[81];
    sprintf(bufs, "_d%d$:\n", nommod); // BLF
    fputs(bufs, syslin);
    //  empty module test
    if (mod_length != 0)
    {
        // text generating
        sfop_r(&sysut1);
        sfop_r(&sysut2);
        while (true)
        {
            sfrd2();
            delta = rl.delta;
            for (k = 0; k < delta; k++)
            {
                sfrd1(&d.b[0], 1);
                if (k % 60 == 0)
                {
                    if (k != 0)
                        fputc('\n', syslin);
                    fputs("\t.byte\t", syslin);
                }
                sprintf(bufs, "%d", d.w);
                fputs(bufs, syslin);
                if (k % 60 != 59 && k != (size_t)(delta - 1))
                    fputc(',', syslin);
            }
            fputc('\n', syslin);
            const T_U *p = rl.point;
            if (p != NULL)
            {
                while ((p->mode & '\300') == '\300')
                    p = p->info.infop;
                if ((p->mode & '\300') != '\200')
                {
                    //    nonexternal label
                    if (LBLL == 4)
                        sprintf(bufs, "\t.long\t_d%d$+%zu\n", nommod, p->info.infon);
                    else
                        sprintf(bufs, "\t.quad\t_d%d$+%zu\n", nommod, p->info.infon);
                    fputs(bufs, syslin);
                }
                else
                {
//     external   label
// BLF
#ifdef UNIX
                    // begin name without underlining _
                    if (LBLL == 4)
                        fputs("\t.long\t", syslin);
                    else
                        fputs("\t.quad\t", syslin);
#else // Windows - with underlining _
                    if (LBLL == 4)
                        fputs("\t.long\t_", syslin);
                    else
                        fputs("\t.quad\t_", syslin);
#endif
                    qx = first_ext;
                    for (size_t i = 1; i < p->info.infon; i++)
                        qx = qx->next;
#ifdef UNIX
                    // BLF ------- renaming add, sub, mul ... ---------------
                    /* For GCC under UNIX we have the
                    following problem. Variable names have not
                    underscore (_) in begin (as it is in Windows).
                    That is cause for collision names
                        add, sub, mul  ...
                    with corresponding assembler operation.
                    The solution which we propuse is to rename
                    on the fly that refal operation to
                        ad_, su_, mu_ ... accordingly.
                    */
                    const char oper_add[] = "ad_";
                    const char oper_sub[] = "su_";
                    const char oper_mul[] = "mu_";
                    const char oper_div[] = "di_";
                    const char oper_rp[] = "r_";
                    const char oper_ptr[] = "pt_";
                    // BLF - debug printf ("%s\n",qx->e) ;
                    if (strncmp(qx->e, "ADD", qx->le) == 0)
                        for (size_t i = 0; i < qx->le; i++)
                            *(qx->e + i) = oper_add[i];
                    else if (strncmp(qx->e, "SUB", qx->le) == 0)
                        for (size_t i = 0; i < qx->le; i++)
                            *(qx->e + i) = oper_sub[i];
                    else if (strncmp(qx->e, "MUL", qx->le) == 0)
                        for (size_t i = 0; i < qx->le; i++)
                            *(qx->e + i) = oper_mul[i];
                    else if (strncmp(qx->e, "DIV", qx->le) == 0)
                        for (size_t i = 0; i < qx->le; i++)
                            *(qx->e + i) = oper_div[i];
                    else if (strncmp(qx->e, "RP", qx->le) == 0)
                        for (size_t i = 0; i < qx->le; i++)
                            *(qx->e + i) = oper_rp[i];
                    else if (strncmp(qx->e, "PTR", qx->le) == 0)
                        for (size_t i = 0; i < qx->le; i++)
                            *(qx->e + i) = oper_ptr[i];
#endif
                    // BLF ------- end renaming ---------------
                    for (size_t i = 0; i < qx->le; i++)
                        // BLF    fputc (*(qx->e + i),syslin);
                        fputc(tolower(*(qx->e + i)), syslin); // BLF
                    fputs("\n", syslin);
                }
                continue;
            } // if
            break;
        }
        // end text generating
        /* BLF
        fputs("_",syslin); for(i=0;i<lnmmod;i++) fputc(mod_name[i],syslin);
        fputs ("\tends\n",syslin);
        */
        //   external label generating
        qx = first_ext->next;
        while (qx != NULL)
        {
// BLF     fputs ("\textrn\t_",syslin);
// BLF
#ifdef UNIX
            // begin name without underlining _
            fputs("\t.extern\t", syslin); // BLF
#else                                     // Windows
            fputs("\t.extern\t_", syslin); // BLF
#endif
            for (size_t i = 0; i < qx->le; i++)
                // BLF fputc (*((qx->e) + i),syslin);
                fputc(tolower(*(qx->e + i)), syslin); // BLF
            fputs(":byte\n", syslin);
            qx = qx->next;
        } // while
        // BLF  fputc('_',syslin); for(i=0;i<lnmmod;i++) fputc(mod_name[i],syslin);
        // BLF  fputs ("\tsegment byte public 'CODE'\n",syslin);
        fputs(".data\n", syslin); // BLF
        // entry label generating
        q = first_ent->next;
        while (q != NULL)
        {
// BLF
#ifndef UNIX
            // begin name with underlining _
            fputc('_', syslin);
#endif
            for (size_t i = 0; i < q->le; i++)
                // BLF translate name to lower case
                fputc(tolower(*(q->e + i)), syslin);
            const T_U *pp = q->p;
            while ((pp->mode & '\300') == '\300')
                pp = pp->info.infop;
// BLF
#ifdef UNIX
            // begin name without underlining _

            sprintf(bufs, "\t=_d%d$+%zu\n\t.globl\t", nommod, pp->info.infon);
#else // Windows
            sprintf(bufs, "\t=_d%d$+%zu\n\t.globl\t_", nommod, pp->info.infon);
#endif
            fputs(bufs, syslin);
            for (size_t i = 0; i < q->le; i++)
                // BLF translate name to lower case
                fputc(tolower(*(q->e + i)), syslin);
            fputc('\n', syslin);
            q = q->next;
        };
    }
    // termination
    sfclr(&sysut1);
    sfclr(&sysut2);
    if (options.multmod)
    {
        fclose(syslin);
        if (mod_length != 0)
        {
            fputc(' ', systxt);
            fputs(mod_i, systxt);
        }
        else
        {
            nommod--;
            unlink(mod_i);
        }
    }
    q = first_ent;
    while (q != NULL)
    {
        r = q->next;
        free(q);
#ifdef mdebug
        printf("\nfree(cj) q=%p", q);
#endif
        q = r;
    }
    qx = first_ext;
    while (qx != NULL)
    {
        rx = qx->next;
        free(qx);
#ifdef mdebug
        printf("\nfree(cj) qx=%p", qx);
#endif
        qx = rx;
    }
    return;
} // jend
