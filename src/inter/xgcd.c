/*-------------- file -- XGCD.C ------------*/
/*      MO: gcd - great common divider      */
/*      Last edition date : 19.06.24        */
/*------------------------------------------*/
#include <stdio.h>
#include "refal.def"

extern REFAL refal;

#define d24 16777216l
#define HMAX 4096L
#define MASKA 0xffffffL

#define l1 l[0]
#define l2 l[1]
#define h1 hd[0]
#define h2 hd[1]
#define t1 tl[0]
#define t2 tl[1]
#define p1 p[0]
#define p2 p[1]
#define x1 x[0]
#define x2 x[1]
#define y1 y[0]
#define y2 y[1]
#define s1 s[0]
#define s2 s[1]
#define r1 r[0]
#define r2 r[1]

static void norm(linkcb *X, int dl, int j) /*  normaliz. posledov. makrocifr */
{                                          /*  X - ukaz. na konec            */
    long a, g, m, peren;
    int i, ip;
    peren = 0l;
    ip = 24 - j;
    m = MASKA >> j; /* maska */
    for (i = 0; i < dl; i++)
    {
        g = gcoden(X);
        a = (g & m) << j;
        pcoden(X, (long)(a | peren));
        peren = g >> ip;
        X = X->prev;
    }
}

static void ymn(long *a, long *b)
{ /* rez.: a - star., b - mlad. */
    int a1, a2, b1, b2, rr1, rr2, rr3, rr4;
    long rr;
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
    a1 = (*a) >> 12;
    b1 = (*b) >> 12;
    a2 = (*a) & 0xFFF;
    b2 = (*b) & 0xFFF;
    rr = a2 * (long)b2;
    *b = rr & 0xFFF;
    rr3 = rr >> 12;
    rr = a1 * (long)b2;
    rr3 += rr & 0xFFF;
    rr2 = rr >> 12;
    rr = a2 * (long)b1;
    rr3 += rr & 0xFFF;
    rr2 += rr >> 12;
    rr = a1 * (long)b1;
    rr2 += rr & 0xFFF;
    rr1 = rr >> 12;
    rr4 = rr3 >> 12;
    *a = rr1 * HMAX + rr2 + rr4;
    *b += (rr3 & 0xFFF) * HMAX;
}

static void gcd_()
{
    int l[2], i, j, k, rez, la, lb, n;
    linkcb *hd[2], *tl[2], *pr, *p[2], *px, *py, *Xt, *Yt;
    long a, a1, b, b1, c, A, B, AL, AH, BL, BH, RL, RH, x[2], y[2], xn, yn, Q;
    long s[2], r0, r[2], v1, v2, q, peren, J;
    long vs1, vs2, vs3, vs4;

    /*   sint. control */
    pr = refal.preva->next;
    if (pr->tag != TAGLB)
        goto NEOT;
    p1 = pr;
    t1 = pr->info.codep;
    p2 = t1;
    t2 = refal.nexta;
    for (i = 0; i < 2; i++)
    {
        pr = p[i]->next;
        if ((pr->tag == TAGO) && (pr->info.infoc == '+' || pr->info.infoc == '-'))
            pr = pr->next;
        hd[i] = pr;
        l[i] = 0;
        while (pr != tl[i])
        {
            if (pr->tag != TAGN)
                goto NEOT;
            l[i]++;
            pr = pr->next;
        }
        tl[i] = pr->prev;
    }

OC: /********   ob. cikl  ************/
    /*   unicht. lewyh nulej */
    if (l1 != 0)
        while (gcoden(h1) == 0l && l1 > 0)
        {
            h1 = h1->next;
            l1--;
        }
    if (l2 != 0)
        while (gcoden(h2) == 0l && l2 > 0)
        {
            h2 = h2->next;
            l2--;
        }
    if (l1 == 0)
    {
        rez = 1;
        goto FIN1;
    }
    if (l2 == 0)
    {
        rez = 0;
        goto FIN1;
    }
    /*   delaem 1 > 2  */
    if (l1 == l2)
    {
        p1 = h1;
        p2 = h2;
        for (i = 0; i < l1; i++)
        {
            v1 = gcoden(p1);
            v2 = gcoden(p2);
            if (v1 < v2)
                goto M12;
            else if (v1 > v2)
                goto M21;
            p1 = p1->next;
            p2 = p2->next;
        }
        rez = 0;
        goto FIN1;
    }
    else if (l1 < l2)
    {
    M12:
        pr = h1;
        h1 = h2;
        h2 = pr;
        pr = t1;
        t1 = t2;
        t2 = pr;
        i = l1;
        l1 = l2;
        l2 = i;
    }
M21:
    /*   wybor metoda */
    A = 0l;
    pr = h1;
    for (k = 0; k < l1; k++)
    {
        if (A >= 128L)
            break;
        A = A << 24;
        A += gcoden(pr);
        pr = pr->next;
    }
    if ((l1 == 1) || (l1 == 2 && k == 2))
    {
        /* Evklid nad korotkimi */
        /* UTV: l1 >= l2 */
        B = 0l;
        pr = h2;
        for (k = 0; k < l2; k++)
        {
            B = B << 24;
            B += gcoden(pr);
            pr = pr->next;
        }
        while (B != 0l)
        {
            v1 = A / B;
            v2 = A - v1 * B;
            A = B;
            B = v2;
        }
        /* UTV: rez v A */
        pr = refal.preva->next;
        v1 = A >> 24;
        if (v1 != 0l)
        {
            pr->tag = TAGN;
            pcoden(pr, v1);
            pr = pr->next;
            A = A & MASKA;
        }
        pr->tag = TAGN;
        pcoden(pr, A);
        pr = pr->next;
        rftpl(refal.prevr, refal.preva, pr);
        return;
    }
    /*    A - pribligenie  */
    /*    k={ 1/2 }        */
    la = k;
    lb = l2 - (l1 - la);
    if (lb <= 0)
    {
        /*  shag delenija (normal)   */
        /*  A nabrano s nedostatkom  */
        /*  l1 > l2   l2 >0          */
        /*  B nabiraem s izbytkom    */
    SHD:
        /*  delenie mnogih  cifr  */
        h1 = h1->prev;
        h1->tag = TAGN;
        pcoden(h1, 0l);
        l1++;
        for (i = 0, px = h1; i < l2; i++, px = px->next)
            ;
        py = h2->prev;
        py->tag = TAGN;
        pcoden(py, 0l);
        if (l2 != 0)
        { /* wozmovna normalizacija */
            b = gcoden(h2);
            for (n = 0; b < 8388608l; n++, b += b)
                ;
            if (n != 0)
            {
                norm(t1, l1, n);
                norm(t2, l2, n);
            }
        }
        peren = 0l;
        do
        {
            a = gcoden(h1), a1 = gcoden(h1->next);
            b = gcoden(h2);
            if ((a == 0l) && (a1 < b))
                c = 0l;
            else
            {
                if ((a == 0l) && (a1 >= b))
                {
                    c = 1l; /*  t.k. b - normalizowano */
                    a = a1;
                }
                else
                { /* delim a,a1 na b */
                    a = (a * 128) + (a1 >> 17);
                    c = (a / b) << 17;
                    b1 = a1 >> 10;
                    a = ((a % b) * 128) + (b1 & 0x7F);
                    c = c + ((a / b) * 1024);
                    b1 = a1 >> 3;
                    a = ((a % b) * 128) + (b1 & 0x7F);
                    c = c + ((a / b) * 8);
                    a = ((a % b) * 8) + (a1 & 7);
                    c = c + a / b;
                }
                if ((l2 > 1) && ((b1 = gcoden(h2->next)) != 0l))
                {
                    x1 = b1;
                    x2 = c;
                    ymn(&x1, &x2);
                    y1 = a % b;
                    y2 = gcoden(h1->next->next);
                    i = 0;
                    while ((x1 > y1) || (x1 == y1 && x2 > y2))
                    {
                        c--;
                        i = 1;
                        x1 = b1;
                        x2 = c;
                        ymn(&x1, &x2);
                        y1 += b;
                    }
                    if (i == 1)
                        c++; /* na wcjakij sluchaj */
                }
            }
            /* umnovenie  delitelja  na 'c' i wychit. iz X */
            if (c != 0L)
            {
                Yt = t2;
                Xt = px;
                peren = 0L;
                for (; Yt != py->prev; Xt = Xt->prev, Yt = Yt->prev)
                {
                    b = gcoden(Yt);
                    a = c;
                    ymn(&a, &b);
                    b += peren;
                    peren = (b >> 24);
                    b = b & MASKA;
                    J = gcoden(Xt);
                    if (J < b)
                    {
                        J += d24;
                        peren += 1L;
                    }
                    pcoden(Xt, J - b);
                    peren += a;
                }
                if (peren != 0L)
                { /* cifra welika  */
                    do
                    {
                        c -= 1L;
                        Xt = px;
                        Yt = t2;
                        J = 0L;
                        for (; Yt != py->prev; Xt = Xt->prev, Yt = Yt->prev)
                        {
                            a = gcoden(Xt) + gcoden(Yt) + J;
                            J = 0L;
                            if (a >= d24)
                            {
                                a -= d24;
                                J = 1L;
                            }
                            pcoden(Xt, a);
                        }
                        peren -= J;
                    } while (peren != 0L);
                }
            }
            px = px->next;
            h1 = h1->next;
            l1--;
        } while (px != t1->next);
        h1 = h1->prev;
        l1++;
        if (n != 0)
        {
            peren = 0L;
            i = 24 - n;
            c = MASKA >> i;
            /* denormalizacija ostatka */
            for (px = h1; px != t1->next; px = px->next)
            {
                a = gcoden(px);
                b = (a >> n) | (peren << i);
                peren = a & c;
                pcoden(px, b);
            }
            /* denormalizacija delitelja */
            peren = 0L;
            for (px = h2; px != t2->next; px = px->next)
            {
                a = gcoden(px);
                b = (a >> n) | (peren << i);
                peren = a & c;
                pcoden(px, b);
            }
        }
        /*   printf("\n l1=%d l2=%d",l1,l2);
        for(i=0,px=h1; px != t1->next; px=px->next,i++)
           printf("\n A(%d)=%lx",i,gcoden(px));
        for(i=0,px=h2; px != t2->next; px=px->next,i++)
           printf("\n B(%d)=%lx",i,gcoden(px));  */
        goto OC;
    }
    /* UTV:  l2 > hvosta,  */
    /* UTV:  l1 = {1/2} */
    B = 0l;
    pr = h2;
    for (k = 0; k < lb; k++)
    {
        B = B << 24;
        B += gcoden(pr);
        pr = pr->next;
    }
    if ((A / (B + 1) != (A + 1) / B))
        goto SHD;
    /*  metod Lemera         */
    /*  A i B s nedostatkom  */
    /*printf("\nP.chast: A=%ld B=%ld l1=%d l2=%d la=%d lb=%d",A,B,l1,l2,la,lb);*/
    AL = A;
    AH = A + 1;
    BL = B;
    BH = B + 1;
    x1 = 1;
    x2 = 0;
    y1 = 0;
    y2 = 1;
    /*  vychisl koeff. X i Y  */
    while (BL != 0)
    {
        Q = AL / BH;
        /*  UTV:   Q>0  */
        if (Q != AH / BL)
            break;
        RL = AL - Q * BH;
        RH = AH - Q * BL;
        /*  UTV:   RL>=0  */
        /*  UTV:   RH>0   */
        AL = BL;
        AH = BH;
        BL = RL;
        BH = RH;
        xn = x1 - Q * x2;
        yn = y1 - Q * y2;
        x1 = x2;
        y1 = y2;
        x2 = xn;
        y2 = yn;
    }
    /*   vyravnivanie dlin  */
    if (l1 != l2)
    {
        h2 = h2->prev;
        h2->tag = TAGN;
        pcoden(h2, 0l);
        l2++;
    }
    p1 = t1;
    p2 = t2;
    r1 = 0;
    r2 = 0;
    for (k = 0; k < l1; k++)
    {
        s1 = gcoden(p1);
        s2 = gcoden(p2);
        for (i = 0; i < 2; i++)
        {
            vs1 = s1;
            vs2 = s2;
            if (x[i] < 0)
            {
                vs3 = 0 - x[i];
                vs4 = y[i];
            }
            else
            {
                vs3 = x[i];
                vs4 = 0 - y[i];
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
            r0 = r[i] + vs3 + vs4;
            if (r0 < 0)
            {
                vs3 = r0 / d24;
                r0 = r0 % d24;
            }
            else
            {
                vs3 = r0 >> 24;
                r0 = r0 & MASKA;
            }
            r[i] = vs1 + vs2 + vs3;
            if (r0 < 0)
            {
                r[i]--;
                r0 += d24;
            }
            pcoden(p[i], r0);
            p[i] = p[i]->prev;
        }
    }
    /*printf("\nFIN Posl chast: r1=%ld r2=%ld",r1,r2); */
    /*  UTV: R1 i R2 ===0 */
    goto OC;
FIN1: /* rez: odno iz chisel */
    if (l[rez] == 0)
        goto NEOT;
    rftpl(refal.prevr, hd[rez]->prev, tl[rez]->next);
    return;
NEOT:
    refal.upshot = 2;
    return;
NOMEM:
    refal.upshot = 3;
    return;
}
static char add_0[] = {Z3 'G', 'C', 'D', '\003'};
G_L_B char gcd = '\122';
static void (*gcd_1)() = gcd_;

/*------------------ end of file  XGCD.C ----------------*/
