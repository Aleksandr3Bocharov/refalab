// Copyright 2024 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2024-10-19
// https://github.com/Aleksandr3Bocharov/RefalAB

//---------------- file -- XAR.C -----------
//              Multi-digits MO:
//       add, sub, mul, dr, div
//       addn, subn, muln, drn, divn, nrel
//------------------------------------------

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "refal.def"
#include "rfintf.h"

#define Oadd 1
#define Osub 2
#define Omul 3
#define Odr 5

#define HMAX 4096
#define MAX 16777216
#define SMAX 24

static void oper(uint32_t o, uint32_t prn);

static void add_(void) { oper(Oadd, 0); }

#ifdef UNIX
void (*add_1)(void) = add_;
G_L_B char ad_ = '\122';
char add_0[] = {Z3 'A', 'D', '_', '\003'};
#else
void (*add_1)(void) = add_;
G_L_B char add = '\122';
char add_0[] = {Z3 'A', 'D', 'D', '\003'};
#endif

static void sub_(void) { oper(Osub, 0); }

#ifdef UNIX
void (*sub_1)(void) = sub_;
G_L_B char su_ = '\122';
char sub_0[] = {Z3 'S', 'U', '_', '\003'};
#else
void (*sub_1)(void) = sub_;
G_L_B char sub = '\122';
char sub_0[] = {Z3 'S', 'U', 'B', '\003'};
#endif

static void mul_(void) { oper(Omul, 0); }

#ifdef UNIX
void (*mul_1)(void) = mul_;
G_L_B char mu_ = '\122';
char mul_0[] = {Z3 'M', 'U', '_', '\003'};
#else
void (*mul_1)(void) = mul_;
G_L_B char mul = '\122';
char mul_0[] = {Z3 'M', 'U', 'L', '\003'};
#endif

static void dr_(void) { oper(Odr, 0); }
void (*dr_1)(void) = dr_;
G_L_B char dr = '\122';
char dr_0[] = {Z2 'D', 'R', '\002'};

static void div_(void) { oper(Odr, 2); }

#ifdef UNIX
void (*div_1)(void) = div_;
G_L_B char di_ = '\122';
char div_0[] = {Z3 'D', 'I', '_', '\003'};
#else
void (*div_1)(void) = div_;
G_L_B char div = '\122';
char div_0[] = {Z3 'D', 'I', 'V', '\003'};
#endif

static void addn_(void) { oper(Oadd, 1); }
void (*addn_1)(void) = addn_;
G_L_B char addn = '\122';
char addn_0[] = {Z4 'A', 'D', 'D', 'N', '\004'};

static void subn_(void) { oper(Osub, 1); }
void (*subn_1)(void) = subn_;
G_L_B char subn = '\122';
char subn_0[] = {Z4 'S', 'U', 'B', 'N', '\004'};

static void muln_(void) { oper(Omul, 1); }
void (*muln_1)(void) = muln_;
G_L_B char muln = '\122';
char muln_0[] = {Z4 'M', 'U', 'L', 'N', '\004'};

static void drn_(void) { oper(Odr, 1); }
void (*drn_1)(void) = drn_;
G_L_B char drn = '\122';
char drn_0[] = {Z3 'D', 'R', 'N', '\003'};

static void divn_(void) { oper(Odr, 3); }
void (*divn_1)(void) = divn_;
G_L_B char divn = '\122';
char divn_0[] = {Z4 'D', 'I', 'V', 'N', '\004'};

static T_LINKCB *x, *y, *Xn, *Xk, *nach, *kon, *Yn, *Yk;
static size_t dl, Xdl, Ydl;
static char zn, Xzn, Yzn;

static bool dajch(void)
{
    zn = '+';
    kon = y->prev;
    if (x == kon)
    { // pustoe chislo
        dl = 0;
        return true;
    }
    x = x->next;
    if (x->tag == TAGO &&
        (x->info.infoc == '+' || x->info.infoc == '-'))
    {
        zn = x->info.infoc;
        x = x->next;
        if (x == y)
            return false; //  w chisle - lish znak
    }
    for (; x->tag == TAGN && gcoden(x) == 0; x = x->next)
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

static bool dajarg(void)
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

static void obmen(void)
{
    T_LINKCB *p = Xn;
    Xn = Yn;
    Yn = p;
    p = Xk;
    Xk = Yk;
    Yk = p;
    const size_t i = Xdl;
    Xdl = Ydl;
    Ydl = i;
    const char c = Xzn;
    Xzn = Yzn;
    Yzn = c;
    return;
}

static uint32_t xmy(void)
{ //  if X < Y then true  ( po modulju)
    if (Xdl < Ydl)
        return 1;
    if (Xdl > Ydl)
        return 0;
    for (x = Xn, y = Yn; x != Xk->next; x = x->next, y = y->next)
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
    if (*a == 0)
    {
        *b = 0;
        return;
    }
    if (*b == 0)
    {
        *a = 0;
        return;
    }
    const uint32_t a1 = *a >> 12;
    const uint32_t b1 = *b >> 12;
    const uint32_t a2 = *a & 0xFFF;
    const uint32_t b2 = *b & 0xFFF;
    uint32_t r = a2 * b2;
    *b = r & 0xFFF;
    uint32_t r3 = r >> 12;
    r = a1 * b2;
    r3 += r & 0xFFF;
    uint32_t r2 = r >> 12;
    r = a2 * b1;
    r3 += r & 0xFFF;
    r2 += r >> 12;
    r = a1 * b1;
    r2 += r & 0xFFF;
    const uint32_t r1 = r >> 12;
    const uint32_t r4 = r3 >> 12;
    *a = r1 * HMAX + r2 + r4;
    *b += (r3 & 0xFFF) * HMAX;
    return;
}

static void norm(T_LINKCB *X, size_t dls, size_t j) //  normaliz. posledov. makrocifr
{                                                  //  X - ukaz. na konec
    uint32_t peren = 0;
    const size_t ip = 24 - j;
    const uint32_t m = 0xFFFFFF >> j; // maska
    for (size_t i = 0; i < dls; i++)
    {
        const uint32_t g = gcoden(X);
        const uint32_t a = (g & m) << j;
        pcoden(X, a | peren);
        peren = g >> ip;
        X = X->prev;
    }
    return;
}

static void oper(uint32_t o, uint32_t prn)
{
    if (!dajarg())
    {
        refal.upshot = 2;
        return;
    }
    uint32_t a, b;
    int32_t j;
    uint32_t peren;
    bool rez0 = false;
    bool odnc = false;
    switch (o)
    {
    case Osub: // izmenim znak i skladywaem
        if (Yzn == '-')
            Yzn = '+';
        else
            Yzn = '-';
    case Oadd:
        if (Xdl == 0 && Ydl == 0)
        {
            rez0 = true;
            break;
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
                pcoden(Xn, 0);
                peren = 0;
                for (x = Xk, y = Yk; x != Xn->prev; x = x->prev)
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
                        peren = 1;
                    }
                    else
                        peren = 0;
                } // for
            }
            else
            { // wychitaem
                if (xmy() == 2)
                {
                    rez0 = true;
                    break;
                }
                if (xmy() == 1)
                    obmen();   //  menjaem x i y
                Xn = Xn->prev; //  pripisywaem 0
                Xn->tag = TAGN;
                pcoden(Xn, 0);
                peren = 0;
                for (x = Xk, y = Yk; x != Xn->prev; x = x->prev)
                {
                    j = (int32_t)gcoden(x);
                    if (y != Yn->prev)
                    {
                        j -= (int32_t)(gcoden(y) + peren);
                        y = y->prev;
                    }
                    else
                        j -= (int32_t)peren;
                    if (j < 0)
                    {
                        j += MAX;
                        peren = 1;
                    }
                    else
                        peren = 0;
                    pcoden(x, (uint32_t)j);
                } // for
            } // if
        }
        break;
    case Omul:
        if (Xdl == 0 || Ydl == 0)
        {
            rez0 = true;
            break;
        }
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
            pcoden(x, 0);
        } //  zanulen rezultat
        if (Xdl < Ydl)
            obmen();
        //  dobawim 0 k X dlja summir. s perenosom
        Xn = Xn->prev;
        Xn->tag = TAGN;
        pcoden(Xn, 0);
        T_LINKCB *f;
        uint32_t c;
        for (f = r, y = Yk; y != Yn->prev; y = y->prev, f = f->prev)
        {
            const uint32_t d = gcoden(y);
            if (d != 0)
            { // umn. na 1 cifru
                peren = 0;
                const uint32_t b11 = d >> 12;
                const uint32_t b22 = d & 0xFFF;
                for (x = Xk, p = f; x != Xn->prev; x = x->prev, p = p->prev)
                {
                    a = gcoden(x);
                    if (a == 0)
                        b = 0;
                    else
                    {
                        const uint32_t a11 = a >> 12;
                        const uint32_t a22 = a & 0xFFF;
                        c = a22 * b22;
                        b = c & 0xFFF;
                        uint32_t r3 = c >> 12;
                        c = a11 * b22;
                        r3 += c & 0xFFF;
                        uint32_t r2 = c >> 12;
                        c = a22 * b11;
                        r3 += c & 0xFFF;
                        r2 += c >> 12;
                        c = a11 * b11;
                        r2 += c & 0xFFF;
                        const uint32_t r1 = c >> 12;
                        const uint32_t r4 = r3 >> 12;
                        a = r1 * HMAX + r2 + r4;
                        b += (r3 & 0xFFF) * HMAX;
                    }
                    j = (int32_t)(gcoden(p) + b + peren);
                    peren = 0;
                    if (j >= MAX)
                    {
                        j -= MAX;
                        peren++;
                    }
                    peren += a;
                    pcoden(p, (uint32_t)j);
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
            a = 0;
            b = 0;
            Xzn = '+';
            Yzn = '+';
            odnc = true;
            break;
        }
        if (xmy() == 2)
        { //  rawny
            a = 0;
            b = 1;
            odnc = true;
            break;
        }
        if (xmy() == 1)
        { //  delimoe < delitelja
            if ((prn & 2) == 2)
            { // DIV, DIVN
                a = 0;
                b = 0;
                Xzn = '+';
                Yzn = '+';
                odnc = true;
                break;
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
                pcoden(Xn, 0);
            }
            rftpl(refal.prevr, Xn->prev, Xk->next);
            return;
        }
        //   delimoe > delitelja
        if (Xdl == 1)
        { //  oba po odnoj cifre
            a = gcoden(Xn) % gcoden(Yn);
            b = gcoden(Xn) / gcoden(Yn);
            odnc = true;
            break;
        }
        //  delenie mnogih  cifr
        if (!lrqlk(Xdl - Ydl + 2))
        {
            refal.upshot = 3;
            return;
        }
        // t.k. k chastnomu dob. 0 i zweno na znak
        p = refal.preva;
        lins(p, Xdl - Ydl + 2);
        p = p->next; //  dlja znaka
        r = p->next; //  dlja  perwoj  cifry
        nach = r;
        Xn = Xn->prev;
        Xn->tag = TAGN;
        pcoden(Xn, 0);
        Xdl++;
        size_t i;
        for (i = 0, x = Xn; i < Ydl; i++, x = x->next)
            ;
        y = Yn->prev;
        y->tag = TAGN;
        pcoden(y, 0);
        size_t n = 0;
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
            if (a == 0 && a1 < b)
                c = 0;
            else
            {
                uint32_t b1;
                if (a == 0 && a1 >= b)
                {
                    c = 1; //  t.k. b - normalizowano
                    a = a1;
                }
                else
                { // delim a,a1 na b
                    a = a * 128 + (a1 >> 17);
                    c = a / b << 17;
                    b1 = a1 >> 10;
                    a = (a % b * 128) + (b1 & 0x7F);
                    c += a / b * 1024;
                    b1 = a1 >> 3;
                    a = a % b * 128 + (b1 & 0x7F);
                    c += a / b * 8;
                    a = a % b * 8 + (a1 & 7);
                    c += a / b;
                }
                b1 = gcoden(Yn->next);
                if (Ydl > 1 && b1 != 0)
                {
                    uint32_t x1 = b1;
                    uint32_t x2 = c;
                    ymn(&x1, &x2);
                    uint32_t y1 = a % b;
                    const uint32_t y2 = gcoden(Xn->next->next);
                    i = 0;
                    while (x1 > y1 || (x1 == y1 && x2 > y2))
                    {
                        c--;
                        i = 1;
                        x1 = b1;
                        x2 = c;
                        ymn(&x1, &x2);
                        y1 += b;
                    }
                    if (i == 1)
                        c++; // na wcjakij sluchaj
                }
            }
            // umnovenie  delitelja  na 'c' i wychit. iz X
            if (c != 0)
            {
                const T_LINKCB *Yt = Yk;
                T_LINKCB *Xt = x;
                peren = 0;
                for (; Yt != y->prev; Xt = Xt->prev, Yt = Yt->prev)
                {
                    b = gcoden(Yt);
                    a = c;
                    ymn(&a, &b);
                    b += peren;
                    peren = b >> SMAX;
                    b &= MAX - 1;
                    j = (int32_t)gcoden(Xt);
                    if (j < (int32_t)b)
                    {
                        j += MAX;
                        peren += 1;
                    }
                    pcoden(Xt, (uint32_t)j - b);
                    peren += a;
                }
                if (peren != 0)
                    do
                    {
                        c -= 1;
                        Xt = x;
                        Yt = Yk;
                        j = 0;
                        for (; Yt != y->prev; Xt = Xt->prev, Yt = Yt->prev)
                        {
                            a = gcoden(Xt) + gcoden(Yt) + (uint32_t)j;
                            j = 0;
                            if (a >= MAX)
                            {
                                a -= MAX;
                                j = 1;
                            }
                            pcoden(Xt, a);
                        }
                        peren -= (uint32_t)j;
                    } while (peren != 0);
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
            peren = 0;
            i = 24 - n;
            c = 0xFFFFFF >> i;
            for (x = Xn; x != Xk->next; x = x->next)
            {
                a = gcoden(x);
                b = a >> n | peren << i;
                peren = a & c;
                pcoden(x, b);
            }
        }
        for (x = Xn; x != Xk->next && gcoden(x) == 0; x = x->next)
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
        for (x = nach; gcoden(x) == 0; x = x->next)
            ;
        if (Xzn == '-')
        {
            x = x->prev;
            x->tag = TAGO;
            x->info.codep = NULL;
            x->info.infoc = '-';
        }
        if ((prn & 1) == 0 || Xn != Xk || gcoden(Xn) != 0)
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
    if (rez0)
    {
        if (prn == 1)
            return; // dlja n-operacij
        x = refal.preva->next;
        x->tag = TAGN;
        pcoden(x, 0);
        rftpl(refal.prevr, x->prev, x->next);
        return;
    }
    if (!odnc)
    {
        //  wozwratim X
        // podawim wed. nuli
        for (x = Xn; gcoden(x) == 0; x = x->next)
            ;
        if (prn == 1 && x == Xk && gcoden(x) == 0)
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
    }
    // wywod rezultata delenija, kogda ostatok i chastnoe
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
    if (b != 0 || (prn & 1) == 0)
    { // div/dr
        x->tag = TAGN;
        pcoden(x, b);
        x = x->next;
    }
    y = x->next;
    if (a != 0)
        if (Xzn != '+')
        {
            y->tag = TAGO;
            y->info.codep = NULL;
            y->info.infoc = '-';
            y = y->next;
        }
    if (a != 0 || (prn & 1) == 0)
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

static void nrel_(void)
{
    if (!dajarg())
    {
        refal.upshot = 2;
        return;
    }
    char c;
    if (Xdl == 0 && Ydl == 0)
        c = '=';
    else
    {
        if (Xzn == Yzn && xmy() == 2)
            c = '=';
        else
        {
            if ((Xzn == '-' && Yzn == '+') ||
                (Xzn == '-' && Yzn == '-' && !xmy()) ||
                (Xzn == '+' && Yzn == '+' && xmy()))
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
void (*nrel_1)(void) = nrel_;
G_L_B char nrel = '\122';
char nrel_0[] = {Z4 'N', 'R', 'E', 'L', '\004'};

//-------------------- end of file  XAR.C ----------------
