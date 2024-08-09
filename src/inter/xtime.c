//-------------- file -- XTIME.C ------------
//        MO: rftime (date and time)
//            rftm (current process time)
//         Last edition date : 11.07.24
//-------------------------------------------
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <locale.h>
#include "refal.def"
#include "rfintf.h"

typedef struct timespec timespec;
static timespec t0, t1;

static void rftime_()
{
    setlocale(LC_TIME, setlocale(LC_TIME, ""));
    const time_t tim = time(NULL);
    char s[70];
    strftime(s, sizeof(s), "%c", localtime(&tim));
    T_LINKCB *p = refal.prevr;
    const char *c = s;
    while (*c >= ' ')
    {
        if (!slins(p, 1))
        {
            refal.upshot = 2;
            return;
        }
        p = p->next;
        p->tag = TAGO;
        p->info.codep = NULL;
        p->info.infoc = *c;
        c++;
    }
    return;
}
static char rftime_0[] = {Z6 'R', 'F', 'T', 'I', 'M', 'E', '\006'};
G_L_B char rftime = '\122';
static void (*rftime_1)() = rftime_;

static void rftm_()
{
    T_LINKCB *p = refal.preva->next;
    if (p != refal.nexta)
        if (p->tag == TAGO)
        {
            const char c = p->info.infoc;
            switch (c)
            {
            case 's':
            case 'S':
                timespec_get(&t0, TIME_UTC);
                return;
            case 'g':
            case 'G':
                timespec_get(&t1, TIME_UTC);
                int32_t in = t1.tv_nsec - t0.tv_nsec;
                uint32_t is = difftime(t1.tv_sec, t0.tv_sec);
                if (in < 0)
                {
                    in += 1000000000;
                    is--;
                }
                uint32_t im = is / 60;
                is %= 60;
                const uint32_t ih = im / 60;
                im %= 60;
                char s[25];
                sprintf(s, "%02u:%02u:%02u.%09d", ih, im, is, in);
                p = refal.prevr;
                if (!slins(p, strlen(s)))
                    return;
                for (size_t i = 0; s[i] != 0; i++)
                {
                    p = p->next;
                    p->tag = TAGO;
                    p->info.codep = NULL;
                    p->info.infoc = s[i];
                }
                return;
            default:;
            }
        }
    refal.upshot = 2;
    return;
}
static char rftm_0[] = {Z4 'R', 'F', 'T', 'M', '\004'};
G_L_B char rftm = '\122';
static void (*rftm_1)() = rftm_;

//------------------ end of file  XTIME.C ----------------
