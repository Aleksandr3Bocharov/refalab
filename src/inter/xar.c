//-------------- file -- xar_blf.c ---------
//              Multi-digits MO:
//       add, sub, mul, div, dr, nrel
//       addn, subn, muln, divn, drn,
//      Last edition date : 11.07.2024
//------------------------------------------
#include <stdio.h>
#include <stdbool.h>
#include "refal.def"
#include "rfintf.h"

#define Oadd 1
#define Osub 2
#define Omul 3
#define Odr 5

#define HMAX 4096L
#define MAX 16777216L
#define SMAX 24

static void oper(unsigned int o, unsigned int prn);

static void add_() { oper(Oadd, 0); }

// BLF
#ifdef UNIX
static char add_0[] = {Z3 'A', 'D', '_', '\003'};
G_L_B char ad_ = '\122';
static void (*add_1)() = add_;
#else
static char add_0[] = {Z3 'A', 'D', 'D', '\003'};
G_L_B char add = '\122';
static void (*add_1)() = add_;
#endif

static void sub_() { oper(Osub, 0); }

// BLF
#ifdef UNIX
static char sub_0[] = {Z3 'S', 'U', '_', '\003'};
G_L_B char su_ = '\122';
static void (*sub_1)() = sub_;
#else
static char sub_0[] = {Z3 'S', 'U', 'B', '\003'};
G_L_B char sub = '\122';
static void (*sub_1)() = sub_;
#endif

static void mul_() { oper(Omul, 0); }

// BLF
#ifdef UNIX
static char mul_0[] = {Z3 'M', 'U', '_', '\003'};
G_L_B char mu_ = '\122';
static void (*mul_1)() = mul_;
#else
static char mul_0[] = {Z3 'M', 'U', 'L', '\003'};
G_L_B char mul = '\122';
static void (*mul_1)() = mul_;
#endif

static void dr_() { oper(Odr, 0); }
static char dr_0[] = {Z2 'D', 'R', '\002'};
G_L_B char dr = '\122';
static void (*dr_1)() = dr_;

static void divv_() { oper(Odr, 2); }

// BLF
#ifdef UNIX
static char div_0[] = {Z3 'D', 'I', '_', '\003'};
G_L_B char div_ = '\122';
static void (*div_1)() = divv_;
#else
static char div_0[] = {Z3 'D', 'I', 'V', '\003'};
G_L_B char div_ = '\122';
static void (*div_1)() = divv_;
#endif

static void addn_() { oper(Oadd, 1); }
static char addn_0[] = {Z4 'A', 'D', 'D', 'N', '\004'};
G_L_B char addn = '\122';
static void (*addn_1)() = addn_;

static void subn_() { oper(Osub, 1); }
static char subn_0[] = {Z4 'S', 'U', 'B', 'N', '\004'};
G_L_B char subn = '\122';
static void (*subn_1)() = subn_;

static void muln_() { oper(Omul, 1); }
static char muln_0[] = {Z4 'M', 'U', 'L', 'N', '\004'};
G_L_B char muln = '\122';
static void (*muln_1)() = muln_;

static void drn_() { oper(Odr, 1); }
static char drn_0[] = {Z3 'D', 'R', 'N', '\003'};
G_L_B char drn = '\122';
static void (*drn_1)() = drn_;

static void divn_() { oper(Odr, 3); }
static char divn_0[] = {Z4 'D', 'I', 'V', 'N', '\004'};
G_L_B char divn = '\122';
static void (*divn_1)() = divn_;

static T_LINKCB *x, *y, *Xn, *Xk, *nach, *kon, *Yn, *Yk;
static unsigned int dl, Xdl, Ydl;
static char zn, Xzn, Yzn;

static bool dajch()
{
    zn = '+';
    kon = y->prev;
    if (x == kon)
    { // pustoe chislo
        dl = 0;
        return true;
    }
    x = x->next;
    if ((x->tag == TAGO) &&
        ((x->info.infoc == '+') || (x->info.infoc == '-')))
    {
        zn = x->info.infoc;
        x = x->next;
        if (x == y)
            return false; //  w chisle - lish znak
    }
    for (; (x->tag == TAGN) && (gcoden(x) == 0l); x = x->next)
        ;
    if (x == y)
        dl = 0; //  wse cifry - nuli
    else
    {
        for (dl = 0, nach = x; x->tag == TAGN; x = x->next, dl++)
            ;
        if (x != y)
            return false; // ne makrocifra
    }
    return true;
}

static bool dajarg()
{
    x = refal.preva->next;
    if (x->tag != TAGLB)
        return false;
    y = x->info.codep;
    if (dajch())
    {
        Xn = nach;
        Xk = kon;
        Xzn = zn;
        Xdl = dl;
    }
    else
        return false;
    x = y;
    y = refal.nexta;
    if (dajch())
    {
        Yn = nach;
        Yk = kon;
        Yzn = zn;
        Ydl = dl;
        return true;
    }
    else
        return false;
}

static void obmen()
{
    T_LINKCB *p = Xn;
    Xn = Yn;
    Yn = p;
    p = Xk;
    Xk = Yk;
    Yk = p;
    const unsigned int i = Xdl;
    Xdl = Ydl;
    Ydl = i;
    const char c = Xzn;
    Xzn = Yzn;
    Yzn = c;
}

static unsigned int xmy()
{ //  if X < Y then true  ( po modulju)
    if (Xdl < Ydl)
        return 1;
    if (Xdl > Ydl)
        return 0;
    for (x = Xn, y = Yn; (x != Xk->next); x = x->next, y = y->next)
    {
        if (gcoden(x) < gcoden(y))
            return 1;
        if (gcoden(x) > gcoden(y))
            return 0;
    }
    return 2; // x=y
}

static void ymn(uint32_t *a, uint32_t *b)
{ // rez.: a - T_ST., b - ml
    if (*a == 0l)
    {
        *b = 0l;
        return;
    }
    if (*b == 0l)
    {
        *a = 0l;
        return;
    }
    const unsigned int a1 = (*a) >> 12;
    const unsigned int b1 = (*b) >> 12;
    const unsigned int a2 = (*a) & 0xFFF;
    const unsigned int b2 = (*b) & 0xFFF;
    uint32_t r = a2 * (uint32_t)b2;
    *b = r & 0xFFF;
    unsigned int r3 = r >> 12;
    r = a1 * (uint32_t)b2;
    r3 += r & 0xFFF;
    unsigned int r2 = r >> 12;
    r = a2 * (uint32_t)b1;
    r3 += r & 0xFFF;
    r2 += r >> 12;
    r = a1 * (uint32_t)b1;
    r2 += r & 0xFFF;
    const unsigned int r1 = r >> 12;
    const unsigned int r4 = r3 >> 12;
    *a = r1 * HMAX + r2 + r4;
    *b += (r3 & 0xFFF) * HMAX;
}

static void norm(T_LINKCB *X, unsigned int dl, unsigned int j) //  normaliz. posledov. makrocifr
{                                                              //  X - ukaz. na konec
    uint32_t peren = 0l;
    const unsigned int ip = 24 - j;
    const uint32_t m = 0xFFFFFFl >> j; // maska
    for (size_t i = 0; i < dl; i++)
    {
        const uint32_t g = gcoden(X);
        const uint32_t a = (g & m) << j;
        pcoden(X, a | peren);
        peren = g >> ip;
        X = X->prev;
    }
}

static void oper(unsigned int o, unsigned int prn)
{
    if (!dajarg())
    {
        refal.upshot = 2;
        return;
    }
    uint32_t a, b;
    switch (o)
    {
    case Osub: // izmenim znak i skladywaem
        if (Yzn == '-')
            Yzn = '+';
        else
            Yzn = '-';
    case Oadd:
        int32_t j;
        uint32_t peren;
        if ((Xdl == 0) && (Ydl == 0))
        {
        REZ0:
            if (prn == 1)
                return; // dlja n-operacij
            x = refal.preva->next;
            x->tag = TAGN;
            pcoden(x, 0l);
            rftpl(refal.prevr, x->prev, x->next);
            return;
        }
        if (Xdl == 0)
        { //  rezult - wtoroe chislo
            Xn = Yn;
            Xk = Yk;
            Xzn = Yzn;
        }
        else if (Ydl != 0)
        { // summiruem
            if (Xzn == Yzn)
            { //  skladywaem
                if (Xdl < Ydl)
                    obmen();
                //  X  dlinnee  Y  (ili =)
                Xn = Xn->prev; //  pripisywaem  0
                Xn->tag = TAGN;
                pcoden(Xn, 0l);
                peren = 0L;
                for (x = Xk, y = Yk; (x != Xn->prev); x = x->prev)
                {
                    if (y != Yn->prev)
                    {
                        pcoden(x, gcoden(x) + gcoden(y) + peren);
                        y = y->prev;
                    }
                    else
                        pcoden(x, gcoden(x) + peren);
                    if (gcoden(x) >= MAX)
                    {
                        pcoden(x, gcoden(x) - MAX);
                        peren = 1L;
                    }
                    else
                        peren = 0L;
                } // for
            }
            else
            { // wychitaem
                if (xmy() == 2)
                    goto REZ0;
                if (xmy())
                    obmen();   //  menjaem x i y
                Xn = Xn->prev; //  pripisywaem 0
                Xn->tag = TAGN;
                pcoden(Xn, 0l);
                peren = 0L;
                for (x = Xk, y = Yk; (x != Xn->prev); x = x->prev)
                {
                    j = gcoden(x);
                    if (y != Yn->prev)
                    {
                        j -= (gcoden(y) + peren);
                        y = y->prev;
                    }
                    else
                        j -= peren;
                    if (j < 0L)
                    {
                        j += MAX;
                        peren = 1;
                    }
                    else
                        peren = 0L;
                    pcoden(x, j);
                } // for
            } // if
        }
        break;
    case Omul:
        if ((Xdl == 0) || (Ydl == 0))
            goto REZ0;
        if (!lrqlk(Xdl + Ydl + 1))
        {
            refal.upshot = 3;
            return;
        }
        T_LINKCB *p = refal.preva;
        T_LINKCB *r = p->next;
        lins(p, Xdl + Ydl + 1); //  1 zweno dlja znaka
        p = p->next;
        r = r->prev;
        for (x = p; x != r->next; x = x->next)
        {
            x->tag = TAGN;
            pcoden(x, 0l);
        } //  zanulen rezultat
        if (Xdl < Ydl)
            obmen();
        //  dobawim 0 k X dlja summir. s perenosom
        Xn = Xn->prev;
        Xn->tag = TAGN;
        pcoden(Xn, 0l);
        T_LINKCB *f;
        uint32_t c;
        for (f = r, y = Yk; y != Yn->prev; y = y->prev, f = f->prev)
        {
            const uint32_t d = gcoden(y);
            if (d != 0l)
            { // umn. na 1 cifru
                peren = 0L;
                const unsigned int b11 = d >> 12;
                const unsigned int b22 = d & 0xFFF;
                for (x = Xk, p = f; x != Xn->prev; x = x->prev, p = p->prev)
                {
                    a = gcoden(x);
                    if (a == 0l)
                    {
                        b = 0l;
                        goto ret;
                    }
                    const unsigned int a11 = a >> 12;
                    const unsigned int a22 = a & 0xFFF;
                    c = a22 * (uint32_t)b22;
                    b = c & 0xFFF;
                    unsigned int r3 = c >> 12;
                    c = a11 * (uint32_t)b22;
                    r3 += c & 0xFFF;
                    unsigned int r2 = c >> 12;
                    c = a22 * (uint32_t)b11;
                    r3 += c & 0xFFF;
                    r2 += c >> 12;
                    c = a11 * (uint32_t)b11;
                    r2 += c & 0xFFF;
                    const unsigned int r1 = c >> 12;
                    const unsigned int r4 = r3 >> 12;
                    a = r1 * HMAX + r2 + r4;
                    b += (r3 & 0xFFF) * HMAX;
                    // ymn (&a,&b);   // rez:a-T_ST, b-ml
                ret:
                    j = gcoden(p) + b + peren;
                    peren = 0L;
                    if (j >= MAX)
                    {
                        j -= MAX;
                        peren++;
                    }
                    peren += a;
                    pcoden(p, j);
                } // for
            }
        }
        if (Xzn != Yzn)
            Xzn = '-';
        else
            Xzn = '+';
        Xn = p;
        Xk = r;
        break;
    case Odr:
        if (Ydl == 0)
        {
            refal.upshot = 2;
            return;
        }
        if (Xdl == 0)
        {
            a = 0L;
            b = 0L;
            Xzn = '+';
            Yzn = '+';
            goto odnc;
        }
        if (xmy() == 2)
        { //  rawny
            a = 0L;
            b = 1L;
            goto odnc;
        }
        if (xmy())
        { //  delimoe < delitelja
            if ((prn & 2) == 2)
            { // DIV, DIVN
                a = 0L;
                b = 0L;
                Xzn = '+';
                Yzn = '+';
                goto odnc;
            }
            if (Xzn == '-')
            {
                Xn = Xn->prev;
                Xn->tag = TAGO;
                Xn->info.codep = NULL;
                Xn->info.infoc = '-';
            }
            Xn = Xn->prev;
            Xk = Xk->next;
            Xn->tag = TAGLB;
            Xk->tag = TAGRB;
            Xn->info.codep = Xk;
            Xk->info.codep = Xn;
            if (prn == 0)
            {
                Xn = Xn->prev;
                Xn->tag = TAGN;
                pcoden(Xn, 0l);
            }
            rftpl(refal.prevr, Xn->prev, Xk->next);
            return;
        }
        //   delimoe > delitelja
        if (Xdl == 1)
        { //  oba po odnoj cifre
            a = (gcoden(Xn)) % (gcoden(Yn));
            b = (gcoden(Xn)) / (gcoden(Yn));
            goto odnc;
        }
        //  delenie mnogih  cifr
        if (!lrqlk((Xdl - Ydl) + 2))
        {
            refal.upshot = 3;
            return;
        }
        // t.k. k chastnomu dob. 0 i zweno na znak
        p = refal.preva;
        lins(p, (Xdl - Ydl) + 2);
        p = p->next; //  dlja znaka
        r = p->next; //  dlja  perwoj  cifry
        nach = r;
        Xn = Xn->prev;
        Xn->tag = TAGN;
        pcoden(Xn, 0l);
        Xdl++;
        unsigned int i;
        for (i = 0, x = Xn; i < Ydl; i++, x = x->next)
            ;
        y = Yn->prev;
        y->tag = TAGN;
        pcoden(y, 0l);
        unsigned int n = 0;
        if (Ydl != 0)
        { // wozmovna normalizacija
            b = gcoden(Yn);
            for (n = 0; b < 8388608; n++, b += b)
                ;
            if (n != 0)
            {
                norm(Xk, Xdl, n);
                norm(Yk, Ydl, n);
            }
        }
        do
        {
            a = gcoden(Xn);
            const uint32_t a1 = gcoden(Xn->next);
            b = gcoden(Yn);
            /*printf("\na=%ld_%ld b=%ld b1=%ld",a,a1,
                                b,(uint32_t)gcoden(Yn->next));*/
            if ((a == 0l) && (a1 < b))
                c = 0l;
            else
            {
                uint32_t b1;
                if ((a == 0l) && (a1 >= b))
                {
                    c = 1l; //  t.k. b - normalizowano
                    a = a1;
                }
                else
                { // delim a,a1 na b
                    a = (a * 128) + (a1 >> 17);
                    c = (a / b) << 17;
                    b1 = a1 >> 10;
                    a = ((a % b) * 128) + (/*(a1/1024)*/ b1 & 0x7F);
                    c = c + ((a / b) * 1024);
                    b1 = a1 >> 3;
                    a = ((a % b) * 128) + (/*(a1/8)*/ b1 & 0x7F);
                    c = c + ((a / b) * 8);
                    a = ((a % b) * 8) + (a1 & 7);
                    c = c + a / b;
                }
                // printf("\nc=%ld oct=%ld",c,(uint32_t)(a%b));
                if ((Ydl > 1) && ((b1 = gcoden(Yn->next)) != 0l))
                {
                    uint32_t x1 = b1;
                    uint32_t x2 = c;
                    ymn(&x1, &x2);
                    uint32_t y1 = a % b;
                    const uint32_t y2 = gcoden(Xn->next->next);
                    /*printf("\nBegin: c=%ld ",c);
                    printf(" x=%lx_%lx (b1*c)",x1,x2);
                    printf(" y=%lx_%lx (o..a2)",y1,y2);*/
                    i = 0;
                    while ((x1 > y1) || (x1 == y1 && x2 > y2))
                    {
                        c--;
                        i = 1;
                        x1 = b1;
                        x2 = c;
                        ymn(&x1, &x2);
                        y1 += b;
                        /*printf("\nc=%lx ",c);
                        printf(" x=%lx_%lx (b1*c)",x1,x2);
                        printf(" y=%lx_%lx (o..a2)",y1,y2);*/
                    }
                    if (i == 1)
                        c++; // na wcjakij sluchaj
                }
            }
            // umnovenie  delitelja  na 'c' i wychit. iz X
            if (c != 0L)
            {
                const T_LINKCB *Yt = Yk;
                T_LINKCB *Xt = x;
                peren = 0L;
                for (; Yt != y->prev; Xt = Xt->prev, Yt = Yt->prev)
                {
                    b = gcoden(Yt);
                    a = c;
                    ymn(&a, &b);
                    b += peren;
                    peren = (b >> SMAX);
                    b = b & (MAX - 1);
                    j = gcoden(Xt);
                    if (j < b)
                    {
                        j += MAX;
                        peren += 1L;
                    }
                    pcoden(Xt, j - b);
                    peren += a;
                }
                if (peren != 0L)
                //{                                // cifra welika
                    // uint32_t jj=0l;  // !!! wremenno !!!
                    do
                    {
                        // jj++;
                        c -= 1L;
                        Xt = x;
                        Yt = Yk;
                        j = 0L;
                        for (; Yt != y->prev; Xt = Xt->prev, Yt = Yt->prev)
                        {
                            a = gcoden(Xt) + gcoden(Yt) + j;
                            j = 0L;
                            if (a >= MAX)
                            {
                                a -= MAX;
                                j = 1L;
                            }
                            pcoden(Xt, a);
                        }
                        peren -= j;
                    } while (peren != 0L);
                // printf("\nc veliko jj=%ld",jj);
                //}
            }
            r->tag = TAGN;
            pcoden(r, c);
            r = r->next;
            x = x->next;
            Xn = Xn->next;
        } while (x != Xk->next);
        Xn = Xn->prev;
        r = r->prev;
        if (n != 0)
        { // denormalizacija ostatka
            peren = 0L;
            i = 24 - n;
            c = 0xFFFFFFl >> i;
            for (x = Xn; x != Xk->next; x = x->next)
            {
                a = gcoden(x);
                b = (a >> n) | (peren << i);
                peren = a & c;
                pcoden(x, b);
            }
        }
        /*for(i=0,x=Xn; x != Xk->next; x=x->next,i++)
           printf("\n ostat(%d)=%ld",i,gcoden(x));
        for(i=0,x=nach; x != r->next; x=x->next,i++)
           printf("\n chast(%d)=%ld",i,gcoden(x));
        */
        for (x = Xn; (x != Xk->next) && (gcoden(x) == 0l); x = x->next)
            ;
        x = x->prev;
        if (x != Xk)
        {
            if (Xzn != Yzn)
            {
                x->tag = TAGO;
                x->info.codep = NULL;
                x->info.infoc = '-';
            }
            else
                x = x->next;
        }
        Xn = x;
        for (x = nach; gcoden(x) == 0l; x = x->next)
            ;
        if (Xzn == '-')
        {
            x = x->prev;
            x->tag = TAGO;
            x->info.codep = NULL;
            x->info.infoc = '-';
        }
        if ((prn & 1) == 0 || Xn != Xk || gcoden(Xn) != 0l)
            Xn = Xn->prev;
        Xn->tag = TAGLB;
        Xn->info.codep = Xk->next;
        Xk = Xk->next;
        Xk->tag = TAGRB;
        Xk->info.codep = Xn;
        if (r->next != Xn)
            rftpl(r, Xn->prev, Xk->next);
        if ((prn & 2) == 0)
            rftpl(refal.prevr, x->prev, Xk->next);
        else
            rftpl(refal.prevr, x->prev, Xn);
        return;
    } // end case
    //  wozwratim X
    // podawim wed. nuli
    for (x = Xn; gcoden(x) == 0l; x = x->next)
        ;
    if (prn == 1 && x == Xk && gcoden(x) == 0l)
        return;
    if (Xzn == '-')
    {
        x = x->prev;
        x->tag = TAGO;
        x->info.codep = NULL;
        x->info.infoc = '-';
    }
    //  perenosim reultat
    rftpl(refal.prevr, x->prev, Xk->next);
    return;
odnc: // wywod rezultata delenija, kogda ostatok i chastnoe
    // rawno po odnoj makrocifre a - ost., b - chastnoe
    if (!slins(refal.preva, 2))
    {
        refal.upshot = 3;
        return;
    }
    // in bad case: /1/() - 3 zwena est uje + name
    x = refal.preva;
    if (Xzn != Yzn)
    {
        x->tag = TAGO;
        x->info.codep = NULL;
        x->info.infoc = '-';
        x = x->next;
    }
    if (b != 0l || ((prn & 1) == 0))
    { // div/dr
        x->tag = TAGN;
        pcoden(x, b);
        x = x->next;
    }
    y = x->next;
    if (a != 0L)
        if (Xzn != '+')
        {
            y->tag = TAGO;
            y->info.codep = NULL;
            y->info.infoc = '-';
            y = y->next;
        }
    if (a != 0l || ((prn & 1) == 0))
    { // div/dr
        y->tag = TAGN;
        pcoden(y, a);
        y = y->next;
    }
    x->tag = TAGLB;
    y->tag = TAGRB;
    x->info.codep = y;
    y->info.codep = x;
    if ((prn & 2) == 0)
    // dr/n
        rftpl(refal.prevr, refal.preva->prev, y->next);
    else
    // div/n
        rftpl(refal.prevr, refal.preva->prev, x);
    return;
}

static void nrel_()
{
    if (!dajarg())
    {
        refal.upshot = 2;
        return;
    }
    char c;
    if ((Xdl == 0) && (Ydl == 0))
        c = '=';
    else
    {
        if ((Xzn == Yzn) && (xmy() == 2))
            c = '=';
        else
        {
            if (((Xzn == '-') && (Yzn == '+')) ||
                ((Xzn == '-') && (Yzn == '-') && !xmy()) ||
                ((Xzn == '+') && (Yzn == '+') && xmy()))
                c = '<';
            else
                c = '>';
        }
    }
    refal.preva->tag = TAGO;
    refal.preva->info.codep = NULL;
    refal.preva->info.infoc = c;
    rftpl(refal.prevr, refal.preva->prev, refal.nexta);
    return;
}
static char nrel_0[] = {Z4 'N', 'R', 'E', 'L', '\004'};
G_L_B char nrel = '\122';
static void (*nrel_1)() = nrel_;

//-------------------- end of file  XAR.C ----------------
