// Copyright 2025 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2025-03-20
// https://github.com/Aleksandr3Bocharov/RefalAB

//-------------- file -- XGCD.C ------------
//      MO: gcd - great common divider
//------------------------------------------

#include <stddef.h>
#include <stdint.h>
#include "refalab.h"
#include "rfintf.h"

#define d24 16777216
#define HMAX 4096
#define MASKA 0xFFFFFF

static void norm(T_LINKCB *X, size_t dl, size_t j) //  normaliz. posledov. makrocifr
{                                                              //  X - ukaz. na konec
    uint32_t peren = 0;
    const size_t ip = 24 - j;
    const uint32_t m = MASKA >> j; // maska
    for (size_t i = 0; i < dl; i++)
    {
        const uint32_t g = gcoden(X);
        const uint32_t a = (g & m) << j;
        pcoden(X, a | peren);
        peren = g >> ip;
        X = X->prev;
    }
    return;
}

static void ymn(int32_t *a, int32_t *b)
{ // rez.: a - star., b - mlad.
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
    const uint32_t a1 = (uint32_t)*a >> 12;
    const uint32_t b1 = (uint32_t)*b >> 12;
    const uint32_t a2 = *a & 0xFFF;
    const uint32_t b2 = *b & 0xFFF;
    uint32_t rr = a2 * b2;
    *b = rr & 0xFFF;
    uint32_t rr3 = rr >> 12;
    rr = a1 * b2;
    rr3 += rr & 0xFFF;
    uint32_t rr2 = rr >> 12;
    rr = a2 * b1;
    rr3 += rr & 0xFFF;
    rr2 += rr >> 12;
    rr = a1 * b1;
    rr2 += rr & 0xFFF;
    const uint32_t rr1 = rr >> 12;
    const uint32_t rr4 = rr3 >> 12;
    *a = (int32_t)(rr1 * HMAX + rr2 + rr4);
    *b += ((int32_t)rr3 & 0xFFF) * HMAX;
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
    uint32_t A;
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
            uint32_t v1, v2;
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
                A = A << 24;
                A += gcoden(pr);
                pr = pr->next;
            }
            uint32_t B;
            if (l[0] == 1 || (l[0] == 2 && k == 2))
            {
                // Evklid nad korotkimi
                // UTV: l[0] >= l[1]
                B = 0;
                pr = hd[1];
                for (k = 0; k < l[1]; k++)
                {
                    B = B << 24;
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
                v1 = A >> 24;
                if (v1 != 0)
                {
                    pr->tag = TAGN;
                    pr->info.codep = NULL;
                    pcoden(pr, v1);
                    pr = pr->next;
                    A = A & MASKA;
                }
                pr->tag = TAGN;
                pr->info.codep = NULL;
                pcoden(pr, A);
                pr = pr->next;
                rftpl(refal.prevr, refal.preva, pr);
                return;
            }
            //    A - pribligenie
            //    k={ 1/2 }
            const size_t la = k;
            const int32_t lb = (int32_t)(l[1] - (l[0] - la));
            int32_t x[2];
            uint32_t y[2];
            if (lb <= 0)
            {
                gcd_state = SHD;
                break;
            }
            // UTV:  l[1] > hvosta,
            // UTV:  l[0] = {1/2}
            B = 0;
            pr = hd[1];
            for (k = 0; k < (uint32_t)lb; k++)
            {
                B = B << 24;
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
            uint32_t AL = A;
            uint32_t AH = A + 1;
            uint32_t BL = B;
            uint32_t BH = B + 1;
            x[0] = 1;
            x[1] = 0;
            y[0] = 0;
            y[1] = 1;
            //  vychisl koeff. X i Y
            while (BL != 0)
            {
                const uint32_t Q = AL / BH;
                //  UTV:   Q>0
                if (Q != AH / BL)
                    break;
                const uint32_t RL = AL - Q * BH;
                const uint32_t RH = AH - Q * BL;
                //  UTV:   RL>=0
                //  UTV:   RH>0
                AL = BL;
                AH = BH;
                BL = RL;
                BH = RH;
                const int32_t xn = x[0] - (int32_t)Q * x[1];
                const int32_t yn = (int32_t)(y[0] - Q * y[1]);
                x[0] = x[1];
                y[0] = y[1];
                x[1] = xn;
                y[1] = (uint32_t)yn;
            }
            //   vyravnivanie dlin
            if (l[0] != l[1])
            {
                hd[1] = hd[1]->prev;
                hd[1]->tag = TAGN;
                hd[1]->info.codep = NULL;
                pcoden(hd[1], 0);
                l[1]++;
            }
            p[0] = tl[0];
            p[1] = tl[1];
            int32_t r[] = {0, 0};
            for (k = 0; k < l[0]; k++)
            {
                const uint32_t s[] = {gcoden(p[0]), gcoden(p[1])};
                int32_t vs3, vs4;
                for (i = 0; i < 2; i++)
                {
                    int32_t vs1 = (int32_t)s[0];
                    int32_t vs2 = (int32_t)s[1];
                    if (x[i] < 0)
                    {
                        vs3 = 0 - x[i];
                        vs4 = (int32_t)y[i];
                    }
                    else
                    {
                        vs3 = x[i];
                        vs4 = 0 - (int32_t)y[i];
                    }
                    ymn(&vs1, &vs3);
                    ymn(&vs2, &vs4);
                    if (x[i] < 0)
                    {
                        vs1 = 0 - vs1;
                        vs3 = 0 - vs3;
                    }
                    else
                    {
                        vs2 = 0 - vs2;
                        vs4 = 0 - vs4;
                    }
                    int32_t r0 = r[i] + vs3 + vs4;
                    if (r0 < 0)
                    {
                        vs3 = r0 / d24;
                        r0 %= d24;
                    }
                    else
                    {
                        vs3 = r0 >> 24;
                        r0 &= MASKA;
                    }
                    r[i] = vs1 + vs2 + vs3;
                    if (r0 < 0)
                    {
                        r[i]--;
                        r0 += d24;
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
            pcoden(hd[0], 0);
            l[0]++;
            T_LINKCB *px;
            for (i = 0, px = hd[0]; i < l[1]; i++, px = px->next)
                ;
            T_LINKCB *py = hd[1]->prev;
            py->tag = TAGN;
            py->info.codep = NULL;
            pcoden(py, 0);
            size_t n = 0;
            int32_t b;
            if (l[1] != 0)
            { // wozmovna normalizacija
                b = (int32_t)gcoden(hd[1]);
                for (n = 0; b < 8388608; n++, b += b)
                    ;
                if (n != 0)
                {
                    norm(tl[0], l[0], n);
                    norm(tl[1], l[1], n);
                }
            }
            uint32_t peren = 0;
            int32_t a, c;
            do
            {
                a = (int32_t)gcoden(hd[0]);
                const uint32_t a1 = gcoden(hd[0]->next);
                b = (int32_t)gcoden(hd[1]);
                if (a == 0 && a1 < (uint32_t)b)
                    c = 0;
                else
                {
                    uint32_t b1;
                    if (a == 0 && a1 >= (uint32_t)b)
                    {
                        c = 1; //  t.k. b - normalizowano
                        a = (int32_t)a1;
                    }
                    else
                    { // delim a,a1 na b
                        a = a * 128 + (int32_t)(a1 >> 17);
                        c = a / b << 17;
                        b1 = a1 >> 10;
                        a = a % b * 128 + (int32_t)(b1 & 0x7F);
                        c += a / b * 1024;
                        b1 = a1 >> 3;
                        a = a % b * 128 + (int32_t)(b1 & 0x7F);
                        c += a / b * 8;
                        a = a % b * 8 + (int32_t)(a1 & 7);
                        c += a / b;
                    }
                    b1 = gcoden(hd[1]->next);
                    if (l[1] > 1 && b1 != 0)
                    {
                        x[0] = (int32_t)b1;
                        x[1] = c;
                        ymn(&x[0], &x[1]);
                        y[0] = (uint32_t)(a % b);
                        y[1] = gcoden(hd[0]->next->next);
                        i = 0;
                        while (x[0] > (int32_t)y[0] || (x[0] == (int32_t)y[0] && x[1] > (int32_t)y[1]))
                        {
                            c--;
                            i = 1;
                            x[0] = (int32_t)b1;
                            x[1] = c;
                            ymn(&x[0], &x[1]);
                            y[0] += (uint32_t)b;
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
                    uint32_t J;
                    for (; Yt != py->prev; Xt = Xt->prev, Yt = Yt->prev)
                    {
                        b = (int32_t)gcoden(Yt);
                        a = c;
                        ymn(&a, &b);
                        b += (int32_t)peren;
                        peren = (uint32_t)b >> 24;
                        b &= MASKA;
                        J = gcoden(Xt);
                        if (J < (uint32_t)b)
                        {
                            J += d24;
                            peren += 1;
                        }
                        pcoden(Xt, J - (uint32_t)b);
                        peren += (uint32_t)a;
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
                                a = (int32_t)(gcoden(Xt) + gcoden(Yt) + J);
                                J = 0;
                                if (a >= d24)
                                {
                                    a -= d24;
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
                i = 24 - n;
                c = MASKA >> i;
                // denormalizacija ostatka
                for (px = hd[0]; px != tl[0]->next; px = px->next)
                {
                    a = (int32_t)gcoden(px);
                    b = a >> n | (int32_t)peren << i;
                    peren = (uint32_t)(a & c);
                    pcoden(px, (uint32_t)b);
                }
                // denormalizacija delitelja
                peren = 0;
                for (px = hd[1]; px != tl[1]->next; px = px->next)
                {
                    a = (int32_t)gcoden(px);
                    b = a >> n | (int32_t)peren << i;
                    peren = (uint32_t)(a & c);
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

//------------------ end of file  XGCD.C ----------------
