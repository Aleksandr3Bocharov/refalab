/*-----------------  file  --  cj_blf.C  ---------------*/
/*             generator of object module               */
/*                (or assembler text)                   */
/*           Last edition date :  14.06.24              */
/*------------------------------------------------------*/

/* BLF - changes was made for Windows - COFF or GNU and
    for  Unix - ELF or GNU format */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "refal.def"
#include "cj.h"
#include "cerr.h"
#include "refal.h"

/* BLF - for renaming add,sub,mul,div */
char *oper_add;
char *oper_sub;
char *oper_mul;
char *oper_div;
char *oper_rp;
char *oper_ptr;

struct refw
{
    struct refw *next;
    int numb[6];
};

struct u
{
    union
    {
        unsigned infon;
        struct u *infop;
    } info;

    char mode;
    char type;
    int l;
    struct u *i;
    struct u *j;
    struct refw *last_ref;
    struct refw ref;
    int def;
    char k;
    char *id;
};
typedef struct u T_U;

struct ent
{ /* entry table element */
    struct ent *next;
    struct u *p;
    char e[8];
    int le;
};
typedef struct ent T_ENT;

struct ext
{ /* external pointer table element */
    struct ext *next;
    struct u *p;
    char e[8];
    int le;
    int noms;
};
typedef struct ext T_EXT;

typedef struct T_RL
{
    T_U *point;
    int delta;
} T_RL;

static union
{
    char b[2];
    unsigned short int w;
} d;

struct BU_
{
    unsigned len;
    unsigned tek;
    char *nam;
    char *buf;
    FILE *fil;
};
typedef struct BU_ BU;

static BU sysut1 = {0, 0, NULL, NULL, NULL};
static BU sysut2 = {0, 0, NULL, NULL, NULL};
BU sysl = {0, 0, NULL, NULL, NULL};
int curr_r;
int new_r;
int equal_r; /* feature that new_r = curr_r */
char new_f[4];
T_ENT *q, *r;
T_EXT *qx, *rx;
char *ccc;

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
static unsigned long int mod_length;
static char mod_name[9];
static int lnmmod;
static T_EXT *first_ext;
static T_EXT *last_ext;
static unsigned long int curr_addr; /* module generation files  */
static int n_ext;
static T_RL rl;
static int k;
static int delta;

static void ksmn(char *b, int n);

static void oshi1()
{
    fputs("Can't open (in) file sysut1\n", stdout);
    exit(8);
    return;
}

static void oshi2()
{
    fputs("Can't open (in) file sysut2\n", stdout);
    exit(8);
    return;
}

static void osho1()
{
    fputs("Can't open (out) file sysut1\n", stdout);
    exit(8);
    return;
}

static void osho2()
{
    fputs("Can't open (out) file sysut2\n", stdout);
    exit(8);
    return;
}

static void oshd1()
{
    fputs("Unsufficient disk space for sysut1\n", stdout);
    exit(8);
    return;
}

static void oshd2()
{
    fputs("Unsufficient disk space for sysut2\n", stdout);
    exit(8);
    return;
}

static void oshdl()
{
    fputs("Unsufficient disk space for syslin\n", stdout);
    exit(8);
    return;
}

static void osher1()
{
    fputs("I/o error in file sysut1\n", stdout);
    exit(8);
    return;
}

static void osher2()
{
    fputs("I/o error in file sysut2\n", stdout);
    exit(8);
    return;
}

static void oshex()
{
    fputs("\nOSHEX: no memory!!!", stdout);
    exit(1);
}

extern void sfop_w(char *s, BU *b)
{
    unsigned int un = 0;
    unsigned long int lon;
    if (b->nam != NULL)
    {
        free(b->nam);
#ifdef mdebug
        printf("\nfree(cj) b->nam=%lx", b->nam);
#endif
    }
    if ((b->nam = (char *)malloc(strlen(s) + 1)) == NULL)
        oshex();
#ifdef mdebug
    printf("\nmalloc(cj): b->nam=%lx", b->nam);
#endif
    strcpy(b->nam, s);
    if (b->buf == NULL)
    {
        if (options.mincomp == 1)
        {
            if (b == &sysut2)
                un = 2040; /* 2040=340*6  */
            else
                un = 2040; /* 2040=2048-8 */
        }
        else
        {
            if (b == &sysut2)
                un = 49152; /* 8192*6  */
            else
                un = 65528; /* 65536-8 (for bc mojno - 4) */
        }
        while (TRUE)
        {
            if ((b->buf = (char *)malloc(un)) != NULL)
            {
#ifdef mdebug
                printf("\nmalloc(cj): b->buf=%lx un=%u", b->buf, un);
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
        } /*while*/
    }
    b->tek = 0;
    b->len = un;
    b->fil = NULL;
}

static void sfop_r(BU *b)
{
    if (b->fil != NULL)
    {
        if ((b->fil = fopen(b->nam, Rbin)) == NULL)
        {
            printf("Can't open for read %s\n", b->nam);
            exit(8);
        }
        if (fread(b->buf, b->len, 1, b->fil) <= 0)
        {
            printf("Read i/o error in %s\n", b->nam);
            exit(8);
        }
    }
    b->tek = 0;
}

static void sfcl(BU *b)
{
    if (b->fil != NULL)
    {
        if (fwrite(b->buf, b->tek, 1, b->fil) <= 0)
        {
            printf("Write i/o error in %s\n", b->nam);
            exit(8);
        }
        fclose(b->fil);
    }
}

static void sfclr(BU *b)
{
    if (b->fil != NULL)
        unlink(b->nam);
    free(b->nam);
    free(b->buf);
#ifdef mdebug
    printf("\nfree(sfclr) b->nam(c 0)=%lx", b->nam);
    printf("\n            b->buf(c 0)=%lx", b->buf);
#endif
    b->nam = NULL;
    b->buf = NULL;
}

extern void sfclose(BU *b)
{
    if (b->fil == NULL)
    {
        if ((b->fil = fopen(b->nam, Wbin)) == NULL)
        {
            printf("Can't open for write %s\n", b->nam);
            exit(8);
        }
    }
    if (fwrite(b->buf, b->tek, 1, b->fil) <= 0)
    {
        printf("Write i/o error in %s\n", b->nam);
        exit(8);
    }
    fclose(b->fil);
    free(b->nam);
    free(b->buf);
#ifdef mdebug
    printf("\nfree(sfclose) b->nam(c 0)=%lx", b->nam);
    printf("\n              b->buf(c 0)=%lx", b->buf);
#endif
    b->nam = NULL;
    b->buf = NULL;
}

static void sfwr2()
{
    unsigned int ost;
    while (TRUE)
    {
        ost = sysut2.len - sysut2.tek;
        if (ost >= 6)
        {
            memcpy(sysut2.buf + sysut2.tek, &rl, 6);
            sysut2.tek += 6;
            break;
        }
        if (sysut2.fil == NULL)
        {
            if ((sysut2.fil = fopen(sysut2.nam, Wbin)) == NULL)
            {
                printf("Can't open for write sysut2\n");
                exit(8);
            }
        }
        if (fwrite(sysut2.buf, sysut2.len, 1, sysut2.fil) <= 0)
        {
            printf("Write i/o error in sysut2\n");
            exit(8);
        }
        sysut2.tek = 0;
    } /*while*/
} /*sfwr2*/

static void sfwr(char *c, unsigned int n, BU *b)
{
    unsigned int ost;
    while (TRUE)
    {
        ost = b->len - b->tek;
        if (ost >= n)
        {
            memcpy(b->buf + b->tek, c, n);
            b->tek += n;
            break;
        }
        memcpy(b->buf + b->tek, c, ost);
        if (b->fil == NULL)
        {
            if ((b->fil = fopen(b->nam, Wbin)) == NULL)
            {
                printf("Can't open for write %s\n", b->nam);
                exit(8);
            }
        }
        if (fwrite(b->buf, b->len, 1, b->fil) <= 0)
        {
            printf("Write i/o error in %s\n", b->nam);
            exit(8);
        }
        b->tek = 0;
        n -= ost;
        c += ost;
    } /*while*/
} /*sfwr*/

static void sfrd1(char *c, unsigned int n)
{
    unsigned int ost;
    while (TRUE)
    {
        ost = sysut1.len - sysut1.tek;
        if (ost >= n)
        {
            memcpy(c, sysut1.buf + sysut1.tek, n);
            sysut1.tek += n;
            break;
        }
        memcpy(c, sysut1.buf + sysut1.tek, ost);
        if (fread(sysut1.buf, sysut1.len, 1, sysut1.fil) <= 0)
        {
            /*printf("Read i/o error in sysut1\n"); exit(8);*/
        }
        sysut1.tek = 0;
        n -= ost;
        c += ost;
    } /*while*/
} /*sfrd1*/

static void sfrd2()
{
    unsigned int ost;
    while (TRUE)
    {
        ost = sysut2.len - sysut2.tek;
        if (ost >= 6)
        {
            memcpy(&rl, sysut2.buf + sysut2.tek, 6);
            sysut2.tek += 6;
            break;
        }
        if (fread(sysut2.buf, sysut2.len, 1, sysut2.fil) <= 0)
        {
            /*printf("Read i/o error in sysut2\n");
            exit(8);*/
        }
        sysut2.tek = 0;
    } /*while*/
} /*sfrd2*/

static void sfobmen(int n)
{
    unsigned int ost1, ost2;
    while (TRUE)
    {
        ost1 = sysut1.len - sysut1.tek;
        /*printf("\obmen: n=%d ost1=%d",n,ost1);*/
        if (n > ost1)
        {
            n -= ost1;
        }
        else
        {
            ost1 = n;
            n = 0;
        }
        while (ost1 > 0)
        {
            ost2 = sysl.len - sysl.tek;
            if (ost2 >= ost1)
            {
                memcpy(sysl.buf + sysl.tek, sysut1.buf + sysut1.tek, ost1);
                ksmn(sysut1.buf + sysut1.tek, ost1);
                sysl.tek += ost1;
                sysut1.tek += ost1;
                break;
            }
            memcpy(sysl.buf + sysl.tek, sysut1.buf + sysut1.tek, ost2);
            ksmn(sysut1.buf + sysut1.tek, ost2);
            if (sysl.fil == NULL)
            {
                if ((sysl.fil = fopen(sysl.nam, Wbin)) == NULL)
                {
                    printf("Can't open for write syslin\n");
                    exit(8);
                }
            }
            if (fwrite(sysl.buf, sysl.len, 1, sysl.fil) <= 0)
            {
                printf("Write i/o error in syslin\n");
                exit(8);
            }
            ost1 -= ost2;
            sysl.tek = 0;
            sysut1.tek += ost2;
        } /*while*/
        if (n != 0)
        {
            if (fread(sysut1.buf, sysut1.len, 1, sysut1.fil) <= 0)
            {
                /*printf("Read i/o error in sysut1\n");
                exit(8);*/
            }
            sysut1.tek = 0;
        }
        else
            break;
    } /*while*/
} /*sfomen*/

extern void jstart(char *ee, int ll)
{
    delta = 0; /* kras */
    strncpy(mod_name, ee, ll);
    lnmmod = ll;
    sfop_w("sysut1.rf", &sysut1);
    sfop_w("sysut2.rf", &sysut2);
    if ((first_ent = (T_ENT *)malloc(sizeof(T_ENT))) == NULL)
        oshex();
#ifdef mdebug
    printf("\nmalloc(cj): first_ent=%lx", first_ent);
#endif
    last_ent = first_ent;
    first_ent->next = NULL;
    if ((first_ext = (T_EXT *)malloc(sizeof(T_EXT))) == NULL)
        oshex();
#ifdef mdebug
    printf("\nmalloc(cj): first_ext=%lx", first_ext);
#endif
    last_ext = first_ext;
    first_ext->next = NULL;
    curr_addr = 0;
    n_ext = 1;
} /*jstart*/

extern unsigned int jwhere()
{
    if (curr_addr > 65535)
    {
        printf("Module too long\n");
        exit(1);
    }
    return curr_addr;
}

extern void jbyte(char bb)
{
    /* sfwr(&bb,1,&sysut1);   */
    if (sysut1.tek != sysut1.len)
    {
        *(sysut1.buf + sysut1.tek) = bb;
        sysut1.tek++;
    }
    else
    {
        if (sysut1.fil == NULL)
        {
            if ((sysut1.fil = fopen(sysut1.nam, Wbin)) == NULL)
            {
                printf("Can't open for write sysut1\n");
                exit(8);
            }
        }
        if (fwrite(sysut1.buf, sysut1.len, 1, sysut1.fil) <= 0)
        {
            printf("Write i/o error in sysut1\n");
            exit(8);
        }
        *(sysut1.buf) = bb;
        sysut1.tek = 1;
    }
    delta++;
    curr_addr++;
} /*jbyte*/

extern void j3addr(T_U *pp)
{
    rl.point = pp;
    rl.delta = delta;
    delta = 0;
    sfwr2();
    curr_addr += 4;
}

extern void jentry(struct u *pp, char *ee, int ll)
/* ee label  */
{
    /* label length  */
    /*if( (lnmmod==ll) && (strncmp(mod_name, ee, ll < lnmmod ? ll : lnmmod)==0) )
    pchosh("520 entry point name is equal module name");*/
    r = first_ent;
    while (r != last_ent)
    {
        r = r->next;
        if ((r->le == ll) && (strncmp(r->e, ee, ll < r->le ? ll : r->le) == 0))
        {
            /*pchose("521 two entry points has single name ", ee, ll);*/
            return;
        }
    }
    if ((r = (T_ENT *)malloc(sizeof(T_ENT))) == NULL)
        oshex();
#ifdef mdebug
    printf("\nmalloc(cj): r(ent)=%lx", r);
#endif
    last_ent->next = r;
    last_ent = r;
    r->p = pp;
    r->next = NULL;
    r->le = 8 < ll ? 8 : ll;
    strncpy(r->e, ee, r->le);
    pp->mode |= '\040';
} /*jentry*/

extern void jextrn(struct u *pp, char *ee, int ll)
/* ee label  */
{
    /*  label length  */
    if ((rx = (T_EXT *)malloc(sizeof(T_EXT))) == NULL)
        oshex();
#ifdef mdebug
    printf("\nmalloc(cj): rx(ext)=%lx", rx);
#endif
    last_ext->next = rx;
    last_ext = rx;
    rx->p = pp;
    rx->next = NULL;
    rx->le = 8 < ll ? 8 : ll;
    if (strncmp(ee, "DIV", 3) == 0 && (rx->le == 3))
    {
        strcpy(rx->e, "DIV_");
        rx->le = 4;
    }
    else
        strncpy(rx->e, ee, rx->le);
    pp->mode |= '\220';
    n_ext++;
    pp->info.infon = n_ext;
} /*jextrn*/

extern void jlabel(struct u *pp)
{
    pp->mode |= '\120';
    pp->info.infon = curr_addr;
}

extern void jequ(struct u *pp, struct u *qq)
{
    pp->info.infop = qq;
    pp->mode |= '\320';
}

static void zakon()
{
    rl.delta = delta;
    rl.point = NULL;
    sfwr2();
    sfcl(&sysut1);
    sfcl(&sysut2);
    mod_length = curr_addr;
    if (mod_length < 0)
        mod_length = 65536L + mod_length;
} /*zakon*/

extern void jend()
{
    struct u *p, *pp;
    char bufs[81];
    int i;
    zakon();
    if (options.multmod == 1)
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

    /* heading generating */

/* BLF */
#ifdef UNIX /*3*/

/* BLF */
#ifdef FASM
    fputs("format ELF\n", syslin);
#endif

#else /*3*/

#ifdef FASM
    fputs("format COFF\n", syslin);
#endif

#endif /*3*/

/* BLF */
#ifdef FASM
    fputs("section '.data'\n", syslin); /* BLF */
#else
    fputs(".data\n", syslin); /* BLF */
#endif

/* BLF fputc('_',syslin); for(i=0;i<lnmmod;i++) fputc(mod_name[i],syslin); */
/* BLF fputs ("\tsegment\tbyte public 'CODE'\n",syslin); */
/* BLF sprintf(bufs,"_d%d@\tlabel\tbyte\n",nommod); fputs (bufs,syslin); */
/* BLF */
#ifdef FASM
    sprintf(bufs, "_d%d@:\n", nommod); /* BLF */
#else
    sprintf(bufs, "_d%d$:\n", nommod); /* BLF */
#endif

    fputs(bufs, syslin);

    /*  empty module test    */
    if (mod_length == 0)
        goto JTERM;

    /* text generating */

    sfop_r(&sysut1);
    sfop_r(&sysut2);

GEN_TXT:
    sfrd2();
    delta = rl.delta;
    for (k = 0; k < delta; k++)
    {
        sfrd1(&d.b[0], 1);
        if ((k % 60) == 0)
        {
            if (k != 0)
                fputc('\n', syslin);

/* BLF */
#ifdef FASM
            fputs("\tdb\t", syslin);
#else
            fputs("\t.byte\t", syslin);
#endif

        }
        sprintf(bufs, "%d", d.w);
        fputs(bufs, syslin);
        if (((k % 60) != 59) && (k != (delta - 1)))
            fputc(',', syslin);
    }
    fputc('\n', syslin);
    p = rl.point;
    if (p != NULL)
    {
        while (((p->mode) & '\300') == '\300')
            p = p->info.infop;
        if (((p->mode) & '\300') != '\200')
        {
/*    nonexternal label   */
/* BLF */
#ifdef FASM
            sprintf(bufs, "\tdd\t_d%d@+%u\n", nommod, p->info.infon);
#else
            sprintf(bufs, "\t.long\t_d%d$+%u\n", nommod, p->info.infon);
#endif
            fputs(bufs, syslin);
        }
        else
        {
/*     external   label   */
/* BLF */
#ifdef UNIX
            /* begin name without underlining _ */
/* BLF */
#ifdef FASM
            fputs("\tdd\t", syslin);
#else
            fputs("\t.long\t", syslin);
#endif
#else /* Windows - with underlining _*/
/* BLF */
#ifdef FASM
            fputs("\tdd\t_", syslin);
#else
            fputs("\t.long\t_", syslin);
#endif
#endif
            qx = first_ext;
            for (i = 1; i < p->info.infon; i++)
                qx = qx->next;

#ifdef UNIX
            /* BLF ------- renaming add, sub, mul ... --------------- */
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
            oper_add = "ad_";
            oper_sub = "su_";
            oper_mul = "mu_";
            oper_div = "di_";
            oper_rp = "r_";
            oper_ptr = "pt_";

            /* BLF - debug printf ("%s\n",qx->e) ; */
            if (cmpstr(qx->le, qx->e, "ADD") == 0)
                for (i = 0; i < (qx->le); i++)
                    *((qx->e) + i) = oper_add[i];

            else if (cmpstr(qx->le, qx->e, "SUB") == 0)
                for (i = 0; i < (qx->le); i++)
                    *((qx->e) + i) = oper_sub[i];

            else if (cmpstr(qx->le, qx->e, "MUL") == 0)
                for (i = 0; i < (qx->le); i++)
                    *((qx->e) + i) = oper_mul[i];

            else if (cmpstr(qx->le, qx->e, "DIV") == 0)
                for (i = 0; i < (qx->le); i++)
                    *((qx->e) + i) = oper_div[i];

            else if (cmpstr(qx->le, qx->e, "RP") == 0)
                for (i = 0; i < (qx->le); i++)
                    *((qx->e) + i) = oper_rp[i];

            else if (cmpstr(qx->le, qx->e, "PTR") == 0)
                for (i = 0; i < (qx->le); i++)
                    *((qx->e) + i) = oper_ptr[i];

#endif
            /* BLF ------- end renaming --------------- */

            for (i = 0; i < qx->le; i++)
                /* BLF    fputc (*((qx->e) + i),syslin);*/
                fputc(tolower(*((qx->e) + i)), syslin); /* BLF */
            fputs("\n", syslin);
        }
        goto GEN_TXT;
    } /*if*/

    /* end text generating */

/* BLF
fputs("_",syslin); for(i=0;i<lnmmod;i++) fputc(mod_name[i],syslin);
fputs ("\tends\n",syslin);
*/

    /*   external label generating    */

    qx = first_ext->next;
    while (qx != NULL)
    {

/* BLF     fputs ("\textrn\t_",syslin);*/
/* BLF */
#ifdef UNIX
/* begin name without underlining _ */
/* BLF */
#ifdef FASM
        fputs("\textrn\t", syslin); /* BLF */
#else
        fputs("\t.extern\t", syslin); /* BLF */
#endif
#else          /* Windows */
/* BLF */
#ifndef FASM /* then GNU format */
        fputs("\t.extern\t_", syslin); /* BLF */
#else          /* fasm format */
        fputs("\textrn\t_", syslin); /* BLF */
#endif
#endif
        for (i = 0; i < qx->le; i++)
            /* BLF fputc (*((qx->e) + i),syslin);*/
            fputc(tolower(*((qx->e) + i)), syslin); /* BLF */
        fputs(":byte\n", syslin);
        qx = qx->next;
    } /*while*/

    /* BLF  fputc('_',syslin); for(i=0;i<lnmmod;i++) fputc(mod_name[i],syslin); */
    /* BLF  fputs ("\tsegment byte public 'CODE'\n",syslin); */

/* BLF */
#ifdef FASM
    fputs("section '.data'\n", syslin); /* BLF */
#else
    fputs(".data\n", syslin); /* BLF */
#endif

    /* entry label generating */

    q = first_ent->next;
    while (q != NULL)
    {

/* BLF */
#ifndef UNIX
        /* begin name with underlining _ */
        fputc('_', syslin);
#endif

        for (i = 0; i < q->le; i++)
            /* BLF translate name to lower case */
            fputc(tolower(*((q->e) + i)), syslin);
        pp = q->p;
        while (((pp->mode) & '\300') == '\300')
            pp = pp->info.infop;
/* BLF */
#ifdef UNIX
            /* begin name without underlining _ */
/* BLF */
#ifdef FASM
        sprintf(bufs, "\t=_d%d@+%d\n\tpublic\t", nommod, pp->info.infon);
#else
        sprintf(bufs, "\t=_d%d$+%d\n\t.globl\t", nommod, pp->info.infon);
#endif
#else /* Windows */
/* BLF */
#ifdef FASM
        sprintf(bufs, "\t=_d%d@+%d\n\tpublic\t_", nommod, pp->info.infon);
#else
        sprintf(bufs, "\t=_d%d$+%d\n\t.globl\t_", nommod, pp->info.infon);
#endif
#endif
        fputs(bufs, syslin);
        for (i = 0; i < q->le; i++)
            /* BLF translate name to lower case */
            fputc(tolower(*((q->e) + i)), syslin);
        fputc('\n', syslin);
        q = q->next;
    };

    /* termination */

JTERM:
    /* BLF - the old following how (with fasm) not needed
    fputs("_",syslin); for(i=0;i<lnmmod;i++) fputc(mod_name[i],syslin);
    fputs ("\tends\n",syslin);
    fputs ("\tend\n",syslin);
    */

    sfclr(&sysut1);
    sfclr(&sysut2);
    if (options.multmod == 1)
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
        printf("\nfree(cj) q=%lx", q);
#endif
        q = r;
    }
    qx = first_ext;
    while (qx != NULL)
    {
        rx = qx->next;
        free(qx);
#ifdef mdebug
        printf("\nfree(cj) qx=%lx", qx);
#endif
        qx = rx;
    }
} /*jend*/

/*--------------------- OBJ -------------------*/
static union
{
    char cc[2];
    unsigned short int ww;
} stm, ksm;

static char c;

static void sfwrc()
{
    /*  sfwr(&c,1,&sysl);  */
    if (sysl.tek != sysl.len)
    {
        *(sysl.buf + sysl.tek) = c;
        sysl.tek++;
    }
    else
    {
        if (sysl.fil == NULL)
        {
            if ((sysl.fil = fopen(sysl.nam, Wbin)) == NULL)
            {
                printf("Can't open for write %s\n", sysl.nam);
                exit(8);
            }
        }
        if (fwrite(sysl.buf, sysl.len, 1, sysl.fil) <= 0)
        {
            printf("Write i/o error in %s\n", sysl.nam);
            exit(8);
        }
        *(sysl.buf) = c;
        sysl.tek = 1;
    }
} /*sfwrc*/

static void ksmb(char b)
{
    stm.cc[1] = 0;
    stm.cc[0] = b;
    ksm.ww += stm.ww;
    ksm.cc[1] = '\0';
}

static void ksmn(char *b, int n)
{
    int i;
    stm.cc[1] = 0;
    for (i = 0; i < n; i++)
    {
        stm.cc[0] = *(b + i);
        ksm.ww += stm.ww;
    }
    ksm.cc[1] = '\0';
}

static void ksum()
{
    stm.cc[0] = d.b[0];
    ksm.ww += stm.ww;
    stm.cc[0] = d.b[1];
    ksm.ww += stm.ww;
    ksm.cc[1] = '\0';
}

static void zakr()
{
    d.w = 256 - ksm.ww;
    c = d.b[0];
    sfwrc();
    ksm.ww = 0;
}

static void imja(char *n, int l)
{
    int i;
    d.w = l;
    c = d.b[0];
    sfwrc();
    ksmb(c);
    if (l != 0)
    {
        sfwr(n, l, &sysl);
        ksmn(n, l);
    }
}

static void imj_a(char *n, int l)
{
    int i;
    d.w = l + 1;
    c = d.b[0];
    sfwrc();
    ksmb(c);
    c = '_';
    sfwrc();
    ksmb(c);
    if (l != 0)
    {
        sfwr(n, l, &sysl);
        ksmn(n, l);
    }
}

static void golowa(int len, int typ)
{
    d.w = typ;
    c = d.b[0];
    sfwrc();
    ksmb(c);
    d.w = len;
    sfwr((char *)&d, 2, &sysl);
    ksum();
}

extern void jendo()
{
    struct u *p, *pp;
    T_EXT *qxn, *qxk;
    int i, nomsim, smes;
    zakon();
    if (options.multmod == 1)
    {
        strcat(mod_i, ".obj");
        sfop_w(mod_i, &sysl);
    }
    stm.ww = 0;
    ksm.ww = 0;
    nomsim = 1;
    smes = 0;

    /* heading generating  */
    golowa(2 + strlen(parm_i), 0x80);
    imja(parm_i, strlen(parm_i));
    zakr();

    /* version number  */
    k = strlen(vers_i);
    golowa(3 + k, 0x88);
    i = 0;
    sfwr((char *)&i, 2, &sysl);
    sfwr(vers_i, k, &sysl);
    ksmn(vers_i, k);
    zakr();

    /* names */
    golowa(9 + lnmmod, 0x96);
    c = '\0';
    sfwrc();
    imja("CODE", 4);
    imj_a(mod_name, lnmmod);
    zakr();

    /* segment definition */
    golowa(7, 0x98);
    c = 0x48;
    sfwrc();
    ksmb(c);
    d.w = curr_addr;
    sfwr((char *)&d.w, 2, &sysl);
    ksum();
    c = 3;
    sfwrc();
    c = 2;
    sfwrc();
    c = 1;
    sfwrc();
    ksmb('\006');
    zakr();

    /* external names */
    qx = first_ext->next;
    while (qx != NULL)
    {
        qxn = qx;
        for (i = 0; (i < 1000) && (qx != NULL); qx = qx->next)
            i += (qx->le + 3);
        qxk = qx;
        golowa(i + 1, 0x8C);
        for (qx = qxn; qx != qxk; qx = qx->next)
        {
            imj_a(qx->e, qx->le);
            c = '\0';
            sfwrc();
            qx->noms = nomsim++;
        }
        zakr();
    }

    /* entry labels */
    q = first_ent->next;
    while (q != NULL)
    {
        golowa(8 + q->le, 0x90);
        c = 0;
        sfwrc();
        c = 1;
        sfwrc();
        imj_a(q->e, q->le);
        pp = q->p;
        while (((pp->mode) & '\300') == '\300')
            pp = pp->info.infop;
        d.w = pp->info.infon;
        sfwr((char *)&d, 2, &sysl);
        d.w++;
        ksum();
        c = 0;
        sfwrc();
        zakr();
        q = q->next;
    }

    /*  empty module test    */
    if (mod_length == 0)
        goto JTERM;

    /* text generation */
    sfop_r(&sysut1);
    sfop_r(&sysut2);

GEN_TXT:
    sfrd2();
    delta = rl.delta;
    while (delta > 1020)
    {
        delta -= 1020;
        golowa(1024, 0xA0);
        c = 1;
        sfwrc();
        d.w = smes;
        sfwr((char *)&d, 2, &sysl);
        d.w++;
        ksum();
        smes += 1020;
        sfobmen(1020);
        zakr();
    }
    p = rl.point;
    if (p != NULL)
        golowa(8 + delta, 0xA0);
    else
        golowa(4 + delta, 0xA0);
    c = 1;
    sfwrc();
    d.w = smes;
    sfwr((char *)&d, 2, &sysl);
    d.w++;
    ksum();
    smes += (delta + 4);
    sfobmen(delta);
    if (p != NULL)
    {
        c = 0;
        for (i = 0; i < 4; i++)
            sfwrc();
    }
    zakr();
    if (p != NULL)
    {
        while (((p->mode) & '\300') == '\300')
            p = p->info.infop;
        if (((p->mode) & '\300') != '\200')
        {
            /* nonexternal label */
            golowa(8, 0x9C);
            d.w = delta;
            c = d.b[1];
            d.b[1] = d.b[0];
            d.b[0] = c | '\314';
            sfwr((char *)&d, 2, &sysl);
            ksum();
            c = 0;
            sfwrc();
            c = 1;
            sfwrc();
            c = 1;
            sfwrc();
            d.w = p->info.infon;
            sfwr((char *)&d, 2, &sysl);
            d.w += 2;
            ksum();
        }
        else
        {
            /* external   label */
            qx = first_ext;
            for (i = 1; i < p->info.infon; i++)
                qx = qx->next;
            if (qx->noms < 128)
                golowa(6, 0x9C);
            else
                golowa(8, 0x9C);
            d.w = delta;
            c = d.b[1];
            d.b[1] = d.b[0];
            d.b[0] = c | '\314';
            sfwr((char *)&d, 2, &sysl);
            ksum();
            c = 0x26;
            sfwrc();
            c = qx->noms;
            if (qx->noms < 128)
            {
                sfwrc();
                sfwrc();
                d.w = (c * 2 + 0x26);
                ksum();
            }
            else
            {
                /* >127 */
                d.b[1] = c;
                d.b[0] = (qx->noms >> 8) | 0x80;
                sfwr((char *)&d, 2, &sysl);
                sfwr((char *)&d, 2, &sysl);
                ksum();
                ksum();
                ksmb(0x26);
            }
        }
        zakr();
        goto GEN_TXT;
    } /*if*/

    /* termination */

JTERM:
    golowa(2, 0x8A);
    c = 0;
    imja((char *)&c, 0);
    zakr();
    sfclr(&sysut1);
    sfclr(&sysut2);
    if (options.multmod == 1)
    {
        sfclose(&sysl);
        if (mod_length != 0)
        {
            if (nommod != 1)
                fputs("&\n", systxt);
            fputs("-+", systxt);
            fputs(mod_i, systxt);
            fputc(' ', systxt);
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
        printf("\nfree(cj) q=%lx", q);
#endif
        q = r;
    }
    qx = first_ext;
    while (qx != NULL)
    {
        rx = qx->next;
        free(qx);
#ifdef mdebug
        printf("\nfree(cj) qx=%lx", qx);
#endif
        qx = rx;
    }
}

/*-------------  end  of  file  cj_blf.c  --------------*/
