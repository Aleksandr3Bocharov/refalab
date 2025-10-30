// Copyright 2025 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2025-10-30
// https://github.com/Aleksandr3Bocharov/refalab

//---------------- file -- XAR.C -----------
//              Multi-digits MO:
//       add, sub, mul, dr, div,
//       addn, subn, muln, drn, divn,
//       gcd, p1, m1, nrel
//------------------------------------------

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "refalab.h"
#include "rfintf.h"

#define Oadd 1
#define Osub 2
#define Omul 3
#define Odr 5

#define HMAX 65536
#define SMAX 32

static void oper(uint32_t o, uint32_t prn);

static void add_(void) { oper(Oadd, 0); }
char add_0[] = {Z3 'A', 'D', 'D', (char)3};
G_L_B uint8_t refalab_add = '\122';
void (*add_1)(void) = add_;

static void sub_(void) { oper(Osub, 0); }
char sub_0[] = {Z3 'S', 'U', 'B', (char)3};
G_L_B uint8_t refalab_sub = '\122';
void (*sub_1)(void) = sub_;

static void mul_(void) { oper(Omul, 0); }
char mul_0[] = {Z3 'M', 'U', 'L', (char)3};
G_L_B uint8_t refalab_mul = '\122';
void (*mul_1)(void) = mul_;

static void dr_(void) { oper(Odr, 0); }
char dr_0[] = {Z2 'D', 'R', (char)2};
G_L_B uint8_t refalab_dr = '\122';
void (*dr_1)(void) = dr_;

static void div_(void) { oper(Odr, 2); }
char div_0[] = {Z3 'D', 'I', 'V', (char)3};
G_L_B uint8_t refalab_div = '\122';
void (*div_1)(void) = div_;

static void addn_(void) { oper(Oadd, 1); }
char addn_0[] = {Z4 'A', 'D', 'D', 'N', (char)4};
G_L_B uint8_t refalab_addn = '\122';
void (*addn_1)(void) = addn_;

static void subn_(void) { oper(Osub, 1); }
char subn_0[] = {Z4 'S', 'U', 'B', 'N', (char)4};
G_L_B uint8_t refalab_subn = '\122';
void (*subn_1)(void) = subn_;

static void muln_(void) { oper(Omul, 1); }
char muln_0[] = {Z4 'M', 'U', 'L', 'N', (char)4};
G_L_B uint8_t refalab_muln = '\122';
void (*muln_1)(void) = muln_;

static void drn_(void) { oper(Odr, 1); }
char drn_0[] = {Z3 'D', 'R', 'N', (char)3};
G_L_B uint8_t refalab_drn = '\122';
void (*drn_1)(void) = drn_;

static void divn_(void) { oper(Odr, 3); }
char divn_0[] = {Z4 'D', 'I', 'V', 'N', (char)4};
G_L_B uint8_t refalab_divn = '\122';
void (*divn_1)(void) = divn_;

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

static void ymn(int64_t *a, int64_t *b)
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
    const int64_t a1 = *a >> 16;
    const int64_t b1 = *b >> 16;
    const int64_t a2 = *a & 0xFFFF;
    const int64_t b2 = *b & 0xFFFF;
    int64_t r = a2 * b2;
    *b = r & 0xFFFF;
    int64_t r3 = r >> 16;
    r = a1 * b2;
    r3 += r & 0xFFFF;
    int64_t r2 = r >> 16;
    r = a2 * b1;
    r3 += r & 0xFFFF;
    r2 += r >> 16;
    r = a1 * b1;
    r2 += r & 0xFFFF;
    const int64_t r1 = r >> 16;
    const int64_t r4 = r3 >> 16;
    *a = r1 * HMAX + r2 + r4;
    *b += (r3 & 0xFFFF) * HMAX;
    return;
}

static void norm(T_LINKCB *X, size_t dls, size_t j) //  normaliz. posledov. makrocifr
{                                                   //  X - ukaz. na konec
    int64_t peren = 0;
    const size_t ip = SMAX - j;
    const int64_t m = MAX_NUMBER >> j; // maska
    for (size_t i = 0; i < dls; i++)
    {
        const int64_t g = gcoden(X);
        const int64_t a = (g & m) << j;
        pcoden(X, (uint32_t)(a | peren));
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
    int64_t a, b, j, peren;
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
                Xn->info.codep = NULL;
                peren = 0;
                for (x = Xk, y = Yk; x != Xn->prev; x = x->prev)
                {
                    if (y != Yn->prev)
                    {
                        j = (int64_t)gcoden(x) + gcoden(y) + peren;
                        y = y->prev;
                    }
                    else
                        j = (int64_t)gcoden(x) + peren;
                    if (j >= MAX_NUMBER + 1)
                    {
                        j -= MAX_NUMBER + 1;
                        peren = 1;
                    }
                    else
                        peren = 0;
                    pcoden(x, (uint32_t)j);
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
                Xn->info.codep = NULL;
                peren = 0;
                for (x = Xk, y = Yk; x != Xn->prev; x = x->prev)
                {
                    j = gcoden(x);
                    if (y != Yn->prev)
                    {
                        j -= (int64_t)gcoden(y) + peren;
                        y = y->prev;
                    }
                    else
                        j -= peren;
                    if (j < 0)
                    {
                        j += MAX_NUMBER + 1;
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
            x->info.codep = NULL;
        } //  zanulen rezultat
        if (Xdl < Ydl)
            obmen();
        //  dobawim 0 k X dlja summir. s perenosom
        Xn = Xn->prev;
        Xn->tag = TAGN;
        Xn->info.codep = NULL;
        T_LINKCB *f;
        int64_t c;
        for (f = r, y = Yk; y != Yn->prev; y = y->prev, f = f->prev)
        {
            const uint32_t d = gcoden(y);
            if (d != 0)
            { // umn. na 1 cifru
                peren = 0;
                const int64_t b11 = d >> 16;
                const int64_t b22 = d & 0xFFFF;
                for (x = Xk, p = f; x != Xn->prev; x = x->prev, p = p->prev)
                {
                    a = gcoden(x);
                    if (a == 0)
                        b = 0;
                    else
                    {
                        const int64_t a11 = a >> 16;
                        const int64_t a22 = a & 0xFFFF;
                        c = a22 * b22;
                        b = c & 0xFFFF;
                        int64_t r3 = c >> 16;
                        c = a11 * b22;
                        r3 += c & 0xFFFF;
                        int64_t r2 = c >> 16;
                        c = a22 * b11;
                        r3 += c & 0xFFFF;
                        r2 += c >> 16;
                        c = a11 * b11;
                        r2 += c & 0xFFFF;
                        const int64_t r1 = c >> 16;
                        const int64_t r4 = r3 >> 16;
                        a = r1 * HMAX + r2 + r4;
                        b += (r3 & 0xFFFF) * HMAX;
                    }
                    j = (int64_t)gcoden(p) + b + peren;
                    peren = 0;
                    if (j >= MAX_NUMBER + 1)
                    {
                        j -= MAX_NUMBER + 1;
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
                Xn->info.codep = NULL;
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
        Xn->info.codep = NULL;
        Xdl++;
        size_t i;
        for (i = 0, x = Xn; i < Ydl; i++, x = x->next)
            ;
        y = Yn->prev;
        y->tag = TAGN;
        y->info.codep = NULL;
        size_t n = 0;
        if (Ydl != 0)
        { // wozmovna normalizacija
            b = gcoden(Yn);
            for (n = 0; b < 2147483648; n++, b += b)
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
            const int64_t a1 = gcoden(Xn->next);
            b = gcoden(Yn);
            if (a == 0 && a1 < b)
                c = 0;
            else
            {
                int64_t b1;
                if (a == 0 && a1 >= b)
                {
                    c = 1; //  t.k. b - normalizowano
                    a = a1;
                }
                else
                { // delim a,a1 na b
                    a = (a << 7) + (a1 >> 25);
                    c = a / b << 25;
                    b1 = a1 >> 18;
                    a = (a % b << 7) + (b1 & 0x7F);
                    c += a / b << 18;
                    b1 = a1 >> 11;
                    a = (a % b << 7) + (b1 & 0x7F);
                    c += a / b << 11;
                    b1 = a1 >> 4;
                    a = (a % b << 7) + (b1 & 0x7F);
                    c += a / b << 4;
                    a = (a % b << 4) + (a1 & 0xF);
                    c += a / b;
                }
                b1 = gcoden(Yn->next);
                if (Ydl > 1 && b1 != 0)
                {
                    int64_t x1 = b1;
                    int64_t x2 = c;
                    ymn(&x1, &x2);
                    int64_t y1 = a % b;
                    const int64_t y2 = gcoden(Xn->next->next);
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
                    b &= MAX_NUMBER;
                    j = gcoden(Xt);
                    if (j < b)
                    {
                        j += MAX_NUMBER + 1;
                        peren += 1;
                    }
                    pcoden(Xt, (uint32_t)(j - b));
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
                            a = (int64_t)gcoden(Xt) + gcoden(Yt) + j;
                            j = 0;
                            if (a >= MAX_NUMBER + 1)
                            {
                                a -= MAX_NUMBER + 1;
                                j = 1;
                            }
                            pcoden(Xt, (uint32_t)a);
                        }
                        peren -= j;
                    } while (peren != 0);
            }
            r->tag = TAGN;
            r->info.codep = NULL;
            pcoden(r, (uint32_t)c);
            r = r->next;
            x = x->next;
            Xn = Xn->next;
        } while (x != Xk->next);
        Xn = Xn->prev;
        r = r->prev;
        if (n != 0)
        { // denormalizacija ostatka
            peren = 0;
            i = SMAX - n;
            c = MAX_NUMBER >> i;
            for (x = Xn; x != Xk->next; x = x->next)
            {
                a = gcoden(x);
                b = a >> n | peren << i;
                peren = a & c;
                pcoden(x, (uint32_t)b);
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
        x->info.codep = NULL;
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
        x->info.codep = NULL;
        pcoden(x, (uint32_t)b);
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
        y->info.codep = NULL;
        pcoden(y, (uint32_t)a);
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

static void gcd_(void)
{
    //   sint. control
    T_LINKCB *pr = refal.preva->next;
    T_LINKCB *tl[2], *p[2], *hd[2];
    size_t l[2];
    size_t i;
    enum
    {
        OC,
        M12,
        M21,
        SHD,
        FIN1,
        NEOT
    } gcd_state = OC;
    if (pr->tag != TAGLB)
        gcd_state = NEOT;
    else
    {
        tl[0] = pr->info.codep;
        tl[1] = refal.nexta;
        p[0] = pr;
        p[1] = tl[0];
        for (i = 0; i < 2; i++)
        {
            pr = p[i]->next;
            if (pr->tag == TAGO && (pr->info.infoc == '+' || pr->info.infoc == '-'))
                pr = pr->next;
            hd[i] = pr;
            l[i] = 0;
            while (pr != tl[i])
            {
                if (pr->tag != TAGN)
                {
                    gcd_state = NEOT;
                    break;
                }
                l[i]++;
                pr = pr->next;
            }
            if (gcd_state == NEOT)
                break;
            tl[i] = pr->prev;
        }
    }
    int64_t A;
    size_t rez = 0;
    while (true)
        switch (gcd_state)
        {
        case OC:
            //*******   ob. cikl  ***********
            //   unicht. lewyh nulej
            if (l[0] != 0)
                while (gcoden(hd[0]) == 0 && l[0] > 0)
                {
                    hd[0] = hd[0]->next;
                    l[0]--;
                }
            if (l[1] != 0)
                while (gcoden(hd[1]) == 0 && l[1] > 0)
                {
                    hd[1] = hd[1]->next;
                    l[1]--;
                }
            if (l[0] == 0)
            {
                rez = 1;
                gcd_state = FIN1;
                break;
            }
            if (l[1] == 0)
            {
                rez = 0;
                gcd_state = FIN1;
                break;
            }
            //   delaem 1 > 2
            int64_t v1, v2;
            if (l[0] == l[1])
            {
                p[0] = hd[0];
                p[1] = hd[1];
                for (i = 0; i < l[0]; i++)
                {
                    v1 = gcoden(p[0]);
                    v2 = gcoden(p[1]);
                    if (v1 < v2)
                    {
                        gcd_state = M12;
                        break;
                    }
                    if (v1 > v2)
                    {
                        gcd_state = M21;
                        break;
                    }
                    p[0] = p[0]->next;
                    p[1] = p[1]->next;
                }
                if (gcd_state != OC)
                    break;
                rez = 0;
                gcd_state = FIN1;
                break;
            }
            if (l[0] < l[1])
            {
                gcd_state = M12;
                break;
            }
            gcd_state = M21;
            break;
        case M12:
            pr = hd[0];
            hd[0] = hd[1];
            hd[1] = pr;
            pr = tl[0];
            tl[0] = tl[1];
            tl[1] = pr;
            i = l[0];
            l[0] = l[1];
            l[1] = i;
            gcd_state = M21;
            break;
        case M21:
            //   wybor metoda
            A = 0;
            pr = hd[0];
            size_t k;
            for (k = 0; k < l[0]; k++)
            {
                if (A >= 128)
                    break;
                A <<= SMAX;
                A += gcoden(pr);
                pr = pr->next;
            }
            int64_t B;
            if (l[0] == 1 || (l[0] == 2 && k == 2))
            {
                // Evklid nad korotkimi
                // UTV: l[0] >= l[1]
                B = 0;
                pr = hd[1];
                for (k = 0; k < l[1]; k++)
                {
                    B <<= SMAX;
                    B += gcoden(pr);
                    pr = pr->next;
                }
                while (B != 0)
                {
                    v1 = A / B;
                    v2 = A - v1 * B;
                    A = B;
                    B = v2;
                }
                // UTV: rez v A
                pr = refal.preva->next;
                v1 = A >> SMAX;
                if (v1 != 0)
                {
                    pr->tag = TAGN;
                    pr->info.codep = NULL;
                    pcoden(pr, (uint32_t)v1);
                    pr = pr->next;
                    A &= MAX_NUMBER;
                }
                pr->tag = TAGN;
                pr->info.codep = NULL;
                pcoden(pr, (uint32_t)A);
                pr = pr->next;
                rftpl(refal.prevr, refal.preva, pr);
                return;
            }
            //    A - pribligenie
            //    k={ 1/2 }
            const size_t la = k;
            const int64_t lb = (int64_t)l[1] - (int64_t)l[0] + (int64_t)la;
            int64_t xi[2], yi[2];
            if (lb <= 0)
            {
                gcd_state = SHD;
                break;
            }
            // UTV:  l[1] > hvosta,
            // UTV:  l[0] = {1/2}
            B = 0;
            pr = hd[1];
            for (k = 0; k < (size_t)lb; k++)
            {
                B <<= SMAX;
                B += gcoden(pr);
                pr = pr->next;
            }
            if (A / (B + 1) != (A + 1) / B)
            {
                gcd_state = SHD;
                break;
            }
            //  metod Lemera
            //  A i B s nedostatkom
            int64_t AL = A;
            int64_t AH = A + 1;
            int64_t BL = B;
            int64_t BH = B + 1;
            xi[0] = 1;
            xi[1] = 0;
            yi[0] = 0;
            yi[1] = 1;
            //  vychisl koeff. X i Y
            while (BL != 0)
            {
                const int64_t Q = AL / BH;
                //  UTV:   Q>0
                if (Q != AH / BL)
                    break;
                const int64_t RL = AL - Q * BH;
                const int64_t RH = AH - Q * BL;
                //  UTV:   RL>=0
                //  UTV:   RH>0
                AL = BL;
                AH = BH;
                BL = RL;
                BH = RH;
                const int64_t xn = xi[0] - Q * xi[1];
                const int64_t yn = yi[0] - Q * yi[1];
                xi[0] = xi[1];
                yi[0] = yi[1];
                xi[1] = xn;
                yi[1] = yn;
            }
            //   vyravnivanie dlin
            if (l[0] != l[1])
            {
                hd[1] = hd[1]->prev;
                hd[1]->tag = TAGN;
                hd[1]->info.codep = NULL;
                l[1]++;
            }
            p[0] = tl[0];
            p[1] = tl[1];
            int64_t r[] = {0, 0};
            for (k = 0; k < l[0]; k++)
            {
                const int64_t s[] = {gcoden(p[0]), gcoden(p[1])};
                int64_t vs3, vs4;
                for (i = 0; i < 2; i++)
                {
                    int64_t vs1 = s[0];
                    int64_t vs2 = s[1];
                    if (xi[i] < 0)
                    {
                        vs3 = 0 - xi[i];
                        vs4 = yi[i];
                    }
                    else
                    {
                        vs3 = xi[i];
                        vs4 = 0 - yi[i];
                    }
                    ymn(&vs1, &vs3);
                    ymn(&vs2, &vs4);
                    if (xi[i] < 0)
                    {
                        vs1 = 0 - vs1;
                        vs3 = 0 - vs3;
                    }
                    else
                    {
                        vs2 = 0 - vs2;
                        vs4 = 0 - vs4;
                    }
                    int64_t r0 = r[i] + vs3 + vs4;
                    if (r0 < 0)
                    {
                        vs3 = r0 / (MAX_NUMBER + 1);
                        r0 %= MAX_NUMBER + 1;
                    }
                    else
                    {
                        vs3 = r0 >> SMAX;
                        r0 &= MAX_NUMBER;
                    }
                    r[i] = vs1 + vs2 + vs3;
                    if (r0 < 0)
                    {
                        r[i]--;
                        r0 += MAX_NUMBER + 1;
                    }
                    pcoden(p[i], (uint32_t)r0);
                    p[i] = p[i]->prev;
                }
            }
            //   UTV: r[0] i r[1] ===0
            gcd_state = OC;
            break;
            //  shag delenija (normal)
            //  A nabrano s nedostatkom
            //  l[0] > l[1]   l[1] >0
            //  B nabiraem s izbytkom
        case SHD:
            //  delenie mnogih  cifr
            hd[0] = hd[0]->prev;
            hd[0]->tag = TAGN;
            hd[0]->info.codep = NULL;
            l[0]++;
            T_LINKCB *px;
            for (i = 0, px = hd[0]; i < l[1]; i++, px = px->next)
                ;
            T_LINKCB *py = hd[1]->prev;
            py->tag = TAGN;
            py->info.codep = NULL;
            size_t n = 0;
            int64_t b;
            if (l[1] != 0)
            { // wozmovna normalizacija
                b = gcoden(hd[1]);
                for (n = 0; b < 2147483648; n++, b += b)
                    ;
                if (n != 0)
                {
                    norm(tl[0], l[0], n);
                    norm(tl[1], l[1], n);
                }
            }
            int64_t peren = 0;
            int64_t a, c;
            do
            {
                a = gcoden(hd[0]);
                const int64_t a1 = gcoden(hd[0]->next);
                b = gcoden(hd[1]);
                if (a == 0 && a1 < b)
                    c = 0;
                else
                {
                    int64_t b1;
                    if (a == 0 && a1 >= b)
                    {
                        c = 1; //  t.k. b - normalizowano
                        a = a1;
                    }
                    else
                    { // delim a,a1 na b
                        a = (a << 7) + (a1 >> 25);
                        c = a / b << 25;
                        b1 = a1 >> 18;
                        a = (a % b << 7) + (b1 & 0x7F);
                        c += a / b << 18;
                        b1 = a1 >> 11;
                        a = (a % b << 7) + (b1 & 0x7F);
                        c += a / b << 11;
                        b1 = a1 >> 4;
                        a = (a % b << 7) + (b1 & 0x7F);
                        c += a / b << 4;
                        a = (a % b << 4) + (a1 & 0xF);
                        c += a / b;
                    }
                    b1 = gcoden(hd[1]->next);
                    if (l[1] > 1 && b1 != 0)
                    {
                        xi[0] = b1;
                        xi[1] = c;
                        ymn(&xi[0], &xi[1]);
                        yi[0] = a % b;
                        yi[1] = gcoden(hd[0]->next->next);
                        i = 0;
                        while (xi[0] > yi[0] || (xi[0] == yi[0] && xi[1] > yi[1]))
                        {
                            c--;
                            i = 1;
                            xi[0] = b1;
                            xi[1] = c;
                            ymn(&xi[0], &xi[1]);
                            yi[0] += b;
                        }
                        if (i == 1)
                            c++; // na wcjakij sluchaj
                    }
                }
                // umnovenie  delitelja  na 'c' i wychit. iz X
                if (c != 0)
                {
                    const T_LINKCB *Yt = tl[1];
                    T_LINKCB *Xt = px;
                    peren = 0;
                    int64_t J;
                    for (; Yt != py->prev; Xt = Xt->prev, Yt = Yt->prev)
                    {
                        b = gcoden(Yt);
                        a = c;
                        ymn(&a, &b);
                        b += peren;
                        peren = b >> SMAX;
                        b &= MAX_NUMBER;
                        J = gcoden(Xt);
                        if (J < b)
                        {
                            J += MAX_NUMBER + 1;
                            peren += 1;
                        }
                        pcoden(Xt, (uint32_t)(J - b));
                        peren += a;
                    }
                    if (peren != 0)
                    { // cifra welika
                        do
                        {
                            c -= 1;
                            Xt = px;
                            Yt = tl[1];
                            J = 0;
                            for (; Yt != py->prev; Xt = Xt->prev, Yt = Yt->prev)
                            {
                                a = (int64_t)gcoden(Xt) + gcoden(Yt) + J;
                                J = 0;
                                if (a >= MAX_NUMBER + 1)
                                {
                                    a -= MAX_NUMBER + 1;
                                    J = 1;
                                }
                                pcoden(Xt, (uint32_t)a);
                            }
                            peren -= J;
                        } while (peren != 0);
                    }
                }
                px = px->next;
                hd[0] = hd[0]->next;
                l[0]--;
            } while (px != tl[0]->next);
            hd[0] = hd[0]->prev;
            l[0]++;
            if (n != 0)
            {
                peren = 0;
                i = SMAX - n;
                c = MAX_NUMBER >> i;
                // denormalizacija ostatka
                for (px = hd[0]; px != tl[0]->next; px = px->next)
                {
                    a = gcoden(px);
                    b = a >> n | peren << i;
                    peren = a & c;
                    pcoden(px, (uint32_t)b);
                }
                // denormalizacija delitelja
                peren = 0;
                for (px = hd[1]; px != tl[1]->next; px = px->next)
                {
                    a = gcoden(px);
                    b = a >> n | peren << i;
                    peren = a & c;
                    pcoden(px, (uint32_t)b);
                }
            }
            gcd_state = OC;
            break;
        case FIN1:
            // rez: odno iz chisel
            if (l[rez] == 0)
            {
                gcd_state = NEOT;
                break;
            }
            rftpl(refal.prevr, hd[rez]->prev, tl[rez]->next);
            return;
        case NEOT:
            refal.upshot = 2;
            return;
        }
}
char gcd_0[] = {Z3 'G', 'C', 'D', (char)3};
G_L_B uint8_t refalab_gcd = '\122';
void (*gcd_1)(void) = gcd_;

static void p1_(void)
{
    do
    {
        T_LINKCB *p = refal.preva->next;
        if (p->next != refal.nexta || p->tag != TAGN)
            break;
        uint32_t l = gcoden(p) + 1;
        if (l == 0)
            break;
        pcoden(p, l);
        rftpl(refal.prevr, refal.preva, refal.nexta);
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char p1_0[] = {Z2 'P', '1', (char)2};
G_L_B uint8_t refalab_p1 = '\122';
void (*p1_1)(void) = p1_;

static void m1_(void)
{
    do
    {
        T_LINKCB *p = refal.preva->next;
        if (p->next != refal.nexta || p->tag != TAGN)
            break;
        uint32_t l = gcoden(p) - 1;
        if (l == MAX_NUMBER)
            break;
        pcoden(p, l);
        rftpl(refal.prevr, refal.preva, refal.nexta);
        return;
    } while (false);
    refal.upshot = 2;
    return;
}
char m1_0[] = {Z2 'M', '1', (char)2};
G_L_B uint8_t refalab_m1 = '\122';
void (*m1_1)(void) = m1_;

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
                (Xzn == '-' && Yzn == '-' && xmy() == 0) ||
                (Xzn == '+' && Yzn == '+' && xmy() == 1))
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
char nrel_0[] = {Z4 'N', 'R', 'E', 'L', (char)4};
G_L_B uint8_t refalab_nrel = '\122';
void (*nrel_1)(void) = nrel_;

//-------------------- end of file  XAR.C ----------------
