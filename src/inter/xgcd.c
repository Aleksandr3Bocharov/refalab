//-------------- file -- XGCD.C ------------ 
//      MO: gcd - great common divider       
//      Last edition date : 11.07.24         
//------------------------------------------ 
#include <stdio.h>
#include "refal.def"
#include "rfintf.h"

#define d24 16777216l
#define HMAX 4096L
#define MASKA 0xffffffL

static void norm(T_LINKCB *X, unsigned int dl, unsigned int j) //  normaliz. posledov. makrocifr  
{                                                            //  X - ukaz. na konec             
    unsigned long int peren = 0l;
    const unsigned int ip = 24 - j;
    const unsigned long int m = MASKA >> j; // maska  
    for (unsigned int i = 0; i < dl; i++)
    {
        const unsigned long int g = gcoden(X);
        const unsigned long int a = (g & m) << j;
        pcoden(X, a | peren);
        peren = g >> ip;
        X = X->prev;
    }
}

static void ymn(long int *a, long int *b)
{ // rez.: a - star., b - mlad.  
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
    unsigned long int rr = a2 * (unsigned long int)b2;
    *b = rr & 0xFFF;
    unsigned int rr3 = rr >> 12;
    rr = a1 * (unsigned long int)b2;
    rr3 += rr & 0xFFF;
    unsigned int rr2 = rr >> 12;
    rr = a2 * (unsigned long int)b1;
    rr3 += rr & 0xFFF;
    rr2 += rr >> 12;
    rr = a1 * (unsigned long int)b1;
    rr2 += rr & 0xFFF;
    const unsigned int rr1 = rr >> 12;
    const unsigned int rr4 = rr3 >> 12;
    *a = rr1 * HMAX + rr2 + rr4;
    *b += (rr3 & 0xFFF) * HMAX;
}

static void gcd_()
{
    //   sint. control  
    T_LINKCB *pr = refal.preva->next;
    if (pr->tag != TAGLB)
        goto NEOT;
    T_LINKCB *tl[] = {pr->info.codep, refal.nexta};
    T_LINKCB *p[] = {pr, tl[0]};
    T_LINKCB *hd[2];
    unsigned int l[2];
    unsigned int i;
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

OC: //*******   ob. cikl  *********** 
    //   unicht. lewyh nulej  
    if (l[0] != 0)
        while (gcoden(hd[0]) == 0l && l[0] > 0)
        {
            hd[0] = hd[0]->next;
            l[0]--;
        }
    if (l[1] != 0)
        while (gcoden(hd[1]) == 0l && l[1] > 0)
        {
            hd[1] = hd[1]->next;
            l[1]--;
        }
    unsigned int rez;
    if (l[0] == 0)
    {
        rez = 1;
        goto FIN1;
    }
    if (l[1] == 0)
    {
        rez = 0;
        goto FIN1;
    }
    //   delaem 1 > 2   
    unsigned long int v1, v2;
    if (l[0] == l[1])
    {
        p[0] = hd[0];
        p[1] = hd[1];
        for (i = 0; i < l[0]; i++)
        {
            v1 = gcoden(p[0]);
            v2 = gcoden(p[1]);
            if (v1 < v2)
                goto M12;
            else if (v1 > v2)
                goto M21;
            p[0] = p[0]->next;
            p[1] = p[1]->next;
        }
        rez = 0;
        goto FIN1;
    }
    else if (l[0] < l[1])
    {
    M12:
        pr = hd[0];
        hd[0] = hd[1];
        hd[1] = pr;
        pr = tl[0];
        tl[0] = tl[1];
        tl[1] = pr;
        i = l[0];
        l[0] = l[1];
        l[1] = i;
    }
M21:
    //   wybor metoda  
   unsigned long int A = 0l;
    pr = hd[0];
    unsigned int k;
    for (k = 0; k < l[0]; k++)
    {
        if (A >= 128L)
            break;
        A = A << 24;
        A += gcoden(pr);
        pr = pr->next;
    }
    unsigned long int B;
    if ((l[0] == 1) || (l[0] == 2 && k == 2))
    {
        // Evklid nad korotkimi  
        // UTV: l[0] >= l[1]  
        B = 0l;
        pr = hd[1];
        for (k = 0; k < l[1]; k++)
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
        // UTV: rez v A  
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
    //    A - pribligenie   
    //    k={ 1/2 }         
    const unsigned int la = k;
    const int lb = l[1] - (l[0] - la);
    long int x[2];
    unsigned long int y[2];
    if (lb <= 0)
    {
        //  shag delenija (normal)    
        //  A nabrano s nedostatkom   
        //  l[0] > l[1]   l[1] >0           
        //  B nabiraem s izbytkom     
    SHD:
        //  delenie mnogih  cifr   
        hd[0] = hd[0]->prev;
        hd[0]->tag = TAGN;
        pcoden(hd[0], 0l);
        l[0]++;
        T_LINKCB *px;
        for (i = 0, px = hd[0]; i < l[1]; i++, px = px->next)
            ;
        T_LINKCB *py = hd[1]->prev;
        py->tag = TAGN;
        pcoden(py, 0l);
        unsigned int n;
        unsigned long int b;
        if (l[1] != 0)
        { // wozmovna normalizacija  
            b = gcoden(hd[1]);
            for (n = 0; b < 8388608l; n++, b += b)
                ;
            if (n != 0)
            {
                norm(tl[0], l[0], n);
                norm(tl[1], l[1], n);
            }
        }
        unsigned long int peren = 0l;
        long int a, c;
        do
        {
            a = gcoden(hd[0]);
            const unsigned long int a1 = gcoden(hd[0]->next);
            b = gcoden(hd[1]);
            if ((a == 0l) && (a1 < b))
                c = 0l;
            else
            {
                unsigned long int b1;
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
                    a = ((a % b) * 128) + (b1 & 0x7F);
                    c = c + ((a / b) * 1024);
                    b1 = a1 >> 3;
                    a = ((a % b) * 128) + (b1 & 0x7F);
                    c = c + ((a / b) * 8);
                    a = ((a % b) * 8) + (a1 & 7);
                    c = c + a / b;
                }
                if ((l[1] > 1) && ((b1 = gcoden(hd[1]->next)) != 0l))
                {
                    x[0] = b1;
                    x[1] = c;
                    ymn(&x[0], &x[1]);
                    y[0] = a % b;
                    y[1] = gcoden(hd[0]->next->next);
                    i = 0;
                    while ((x[0] > y[0]) || (x[0] == y[0] && x[1] > y[1]))
                    {
                        c--;
                        i = 1;
                        x[0] = b1;
                        x[1] = c;
                        ymn(&x[0], &x[1]);
                        y[0] += b;
                    }
                    if (i == 1)
                        c++; // na wcjakij sluchaj  
                }
            }
            // umnovenie  delitelja  na 'c' i wychit. iz X  
            if (c != 0L)
            {
                const T_LINKCB *Yt = tl[1];
                T_LINKCB *Xt = px;
                peren = 0L;
                unsigned long int J;
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
                { // cifra welika   
                    do
                    {
                        c -= 1L;
                        Xt = px;
                        Yt = tl[1];
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
            hd[0] = hd[0]->next;
            l[0]--;
        } while (px != tl[0]->next);
        hd[0] = hd[0]->prev;
        l[0]++;
        if (n != 0)
        {
            peren = 0L;
            i = 24 - n;
            c = MASKA >> i;
            // denormalizacija ostatka  
            for (px = hd[0]; px != tl[0]->next; px = px->next)
            {
                a = gcoden(px);
                b = (a >> n) | (peren << i);
                peren = a & c;
                pcoden(px, b);
            }
            // denormalizacija delitelja  
            peren = 0L;
            for (px = hd[1]; px != tl[1]->next; px = px->next)
            {
                a = gcoden(px);
                b = (a >> n) | (peren << i);
                peren = a & c;
                pcoden(px, b);
            }
        }
        /*   printf("\n l[0]=%d l[1]=%d",l[0],l[1]);
        for(i=0,px=hd[0]; px != tl[0]->next; px=px->next,i++)
           printf("\n A(%d)=%lx",i,gcoden(px));
        for(i=0,px=hd[1]; px != tl[1]->next; px=px->next,i++)
           printf("\n B(%d)=%lx",i,gcoden(px));  */
        goto OC;
    }
    // UTV:  l[1] > hvosta,   
    // UTV:  l[0] = {1/2}  
    B = 0l;
    pr = hd[1];
    for (k = 0; k < lb; k++)
    {
        B = B << 24;
        B += gcoden(pr);
        pr = pr->next;
    }
    if ((A / (B + 1) != (A + 1) / B))
        goto SHD;
    //  metod Lemera          
    //  A i B s nedostatkom   
    //printf("\nP.chast: A=%ld B=%ld l[0]=%d l[1]=%d la=%d lb=%d",A,B,l[0],l[1],la,lb); 
    unsigned long int AL = A;
    unsigned long int AH = A + 1;
    unsigned long int BL = B;
    unsigned long int BH = B + 1;
    x[0] = 1;
    x[1] = 0;
    y[0] = 0;
    y[1] = 1;
    //  vychisl koeff. X i Y   
    while (BL != 0)
    {
        const unsigned long int Q = AL / BH;
        //  UTV:   Q>0   
        if (Q != AH / BL)
            break;
        const unsigned long int RL = AL - Q * BH;
        const unsigned long int RH = AH - Q * BL;
        //  UTV:   RL>=0   
        //  UTV:   RH>0    
        AL = BL;
        AH = BH;
        BL = RL;
        BH = RH;
        const long int xn = x[0] - Q * x[1];
        const long int yn = y[0] - Q * y[1];
        x[0] = x[1];
        y[0] = y[1];
        x[1] = xn;
        y[1] = yn;
    }
    //   vyravnivanie dlin   
    if (l[0] != l[1])
    {
        hd[1] = hd[1]->prev;
        hd[1]->tag = TAGN;
        pcoden(hd[1], 0l);
        l[1]++;
    }
    p[0] = tl[0];
    p[1] = tl[1];
    long int r[] = {0, 0};
    for (k = 0; k < l[0]; k++)
    {
        const unsigned long int s[] = {gcoden(p[0]), gcoden(p[1])};
        long int vs3, vs4;
        for (i = 0; i < 2; i++)
        {
            long int vs1 = s[0];
            long int vs2 = s[1];
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
            long int r0 = r[i] + vs3 + vs4;
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
    //printf("\nFIN Posl chast: r[0]=%ld r[1]=%ld",r[0],r[1]);  
    //  UTV: r[0] i r[1] ===0  
    goto OC;
FIN1: // rez: odno iz chisel  
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

//------------------ end of file  XGCD.C ---------------- 
