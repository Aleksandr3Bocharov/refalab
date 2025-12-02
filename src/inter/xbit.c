// Copyright 2025 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2025-12-01
// https://github.com/Aleksandr3Bocharov/refalab

//---------------- file -- XBIT.C -----------
//                Bitwise MO:
//      band, bor, bxor, bnot, shl, shr
//-------------------------------------------

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "refalab.h"
#include "rfintf.h"

#define Oand 1
#define Oor 2
#define Oxor 3
#define Onot 4
#define Oshl 1
#define Oshr 2

static void boper(uint32_t o);
static void shoper(uint32_t o);

static void band_(void) { boper(Oand); }
char band_0[] = {Z4 'B', 'A', 'N', 'D', (char)4};
G_L_B uint8_t refalab_band = '\122';
void (*band_1)(void) = band_;

static void bor_(void) { boper(Oor); }
char bor_0[] = {Z3 'B', 'O', 'R', (char)3};
G_L_B uint8_t refalab_bor = '\122';
void (*bor_1)(void) = bor_;

static void bxor_(void) { boper(Oxor); }
char bxor_0[] = {Z4 'B', 'X', 'O', 'R', (char)4};
G_L_B uint8_t refalab_bxor = '\122';
void (*bxor_1)(void) = bxor_;

static void bnot_(void) { boper(Onot); }
char bnot_0[] = {Z4 'B', 'N', 'O', 'T', (char)4};
G_L_B uint8_t refalab_bnot = '\122';
void (*bnot_1)(void) = bnot_;

static void shl_(void) { shoper(Oshl); }
char shl_0[] = {Z3 'S', 'H', 'L', (char)3};
G_L_B uint8_t refalab_shl = '\122';
void (*shl_1)(void) = shl_;

static void shr_(void) { shoper(Oshr); }
char shr_0[] = {Z3 'S', 'H', 'R', (char)3};
G_L_B uint8_t refalab_shr = '\122';
void (*shr_1)(void) = shr_;

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

static void boper(uint32_t o)
{
    do
    {
        if (o != Onot)
        {
            if (!dajarg())
                break;
        }
        else
        {
            x = refal.preva;
            y = refal.nexta;
            if (dajch())
            {
                Xn = nach;
                Xk = kon;
                Xzn = zn;
                Xdl = dl;
            }
            else
                break;
        }
        bool rez0 = true;
        switch (o)
        {
        case Oand:
            if (Ydl < Xdl)
                obmen();
            if (Xdl == 0)
                break;
            if (Xzn == '-' && Yzn == '+')
                Xzn = '+';
            for (dl = 0, y = Yn; dl < Ydl - Xdl; dl++, y = y->next)
                ;
            for (x = Xn; x != Xk->next; x = x->next, y = y->next)
            {
                pcoden(x, gcoden(x) & gcoden(y));
                if (rez0 && gcoden(x) != 0)
                    rez0 = false;
            }
            break;
        case Oor:
            if (Ydl > Xdl)
                obmen();
            if (Xdl == 0)
                break;
            rez0 = false;
            if (Ydl == 0)
                break;
            if (Xzn == '+' && Yzn == '-')
                Xzn = '-';
            for (dl = 0, x = Xn; dl < Xdl - Ydl; dl++, x = x->next)
                ;
            for (y = Yn; x != Xk->next; x = x->next, y = y->next)
                pcoden(x, gcoden(x) | gcoden(y));
            break;
        case Oxor:
            if (Ydl > Xdl)
                obmen();
            if (Xdl == 0)
                break;
            if (Xdl > Ydl)
                rez0 = false;
            if (Ydl == 0)
                break;
            if (Xzn == '+' && Yzn == '-')
                Xzn = '-';
            else if (Xzn == '-' && Yzn == '-')
                Xzn = '+';
            for (dl = 0, x = Xn; dl < Xdl - Ydl; dl++, x = x->next)
                ;
            for (y = Yn; x != Xk->next; x = x->next, y = y->next)
            {
                pcoden(x, gcoden(x) ^ gcoden(y));
                if (rez0 && gcoden(x) != 0)
                    rez0 = false;
            }
            break;
        case Onot:
            if (Xdl == 0)
            {
                if (refal.preva->next == refal.nexta)
                    if (!slins(refal.preva, 1))
                        return;
                rez0 = false;
                Xzn = '-';
                Xn = refal.preva->next;
                Xk = refal.preva->next;
                Xn->tag = TAGN;
                Xn->info.codep = NULL;
                pcoden(Xn, MAX_NUMBER);
                break;
            }
            if (Xzn == '+')
                Xzn = '-';
            else
                Xzn = '+';
            for (x = Xn; x != Xk->next; x = x->next)
            {
                pcoden(x, ~gcoden(x));
                if (rez0 && gcoden(x) != 0)
                    rez0 = false;
            }
        }
        if (rez0)
        {
            x = refal.preva->next;
            x->tag = TAGN;
            x->info.codep = NULL;
            rftpl(refal.prevr, x->prev, x->next);
            return;
        }
        //  wozwratim X
        // podawim wed. nuli
        for (x = Xn; gcoden(x) == 0; x = x->next)
            ;
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
    } while (false);
    refal.upshot = 2;
    return;
}

static void shoper(uint32_t o)
{
    do
    {
        x = refal.preva->next;
        if (x->tag != TAGLB)
            break;
        y = x->info.codep;
        if (dajch())
        {
            Xn = nach;
            Xk = kon;
            Xzn = zn;
            Xdl = dl;
        }
        else
            break;
        y = y->next;
        if (y->next != refal.nexta || y->tag != TAGN)
            break;
        uint32_t sh = gcoden(y);
        bool rez0 = true;
        switch (o)
        {
        case Oshl:
            if (Xdl == 0)
                break;
            rez0 = false;
            if (sh == 0)
                break;
            break;
        case Oshr:
            dl = sh / 32;
            if (dl >= Xdl)
                break;
            if (dl != 0)
            {
                for (x = Xk, Ydl = 0; Ydl < dl; x = x->prev, Ydl++)
                    ;
                for (y = Xk; x != Xn->prev; x = x->prev, y = y->prev)
                    pcoden(y, gcoden(x));
                Xn = y->next;
            }
            sh %= 32;
            if (sh == 0)
            {
                rez0 = false;
                break;
            }
            if (Xn == Xk)
            {
                pcoden(Xn, gcoden(Xn) >> sh);
                if (gcoden(Xn) != 0)
                    rez0 = false;
                break;
            }
            rez0 = false;
        }
        if (rez0)
        {
            x = refal.preva->next;
            x->tag = TAGN;
            x->info.codep = NULL;
            rftpl(refal.prevr, x->prev, x->next);
            return;
        }
        //  wozwratim X
        // podawim wed. nuli
        for (x = Xn; gcoden(x) == 0; x = x->next)
            ;
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
    } while (false);
    refal.upshot = 2;
    return;
}

//-------------------- end of file  XBIT.C ----------------
