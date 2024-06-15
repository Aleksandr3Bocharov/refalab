/*-------------  file  --  CERR.C  ---------------*/
/*               Print error file                 */
/*        Last edition date : 14.06.24            */
/*------------------------------------------------*/
#include <stdio.h>
#include "refal.def"
#include "cerr.h"

#ifdef turbo
extern int ISTEK, IERRO;
extern char ESTSOO, ERRO[], BUFF[], MESS[];
#endif

extern void oshibka();
extern FILE *sysprint, *systerm;

void pchosh(char *s)
{
    char tmp[255];
    oshibka();
#ifdef turbo
    if (ESTSOO == 0)
    {
        sprintf(ERRO, "  %s", s);
        IERRO = ISTEK - 2;
        ESTSOO = 1;
    }
#endif
    sprintf(tmp, "***** %s\n", s);
    if (sysprint != NULL)
        fputs(tmp, sysprint);
    fputs(tmp, systerm);
}

void pchosi(char *s, char *t)
{
    char tmp[255];
    oshibka();
#ifdef turbo
    if (ESTSOO == 0)
    {
        sprintf(ERRO, "  %s %s", s, t);
        IERRO = ISTEK - 2;
        ESTSOO = 1;
    }
#endif
    sprintf(tmp, "***** %s %s\n", s, t);
    if (sysprint != NULL)
        fputs(tmp, sysprint);
    fputs(tmp, systerm);
}

void pchosj(char *s, char *sid, int lsid, char *s1)
{
    char tmp[255];
    char tmp1[255];
    int i;
    oshibka();
    for (i = 0; i < lsid; i++)
        tmp1[i] = *(sid + i);
    tmp1[lsid] = '\0';
#ifdef turbo
    if (ESTSOO == 0)
    {
        sprintf(ERRO, "  %s %s %s", s, tmp1, s1);
        IERRO = ISTEK - 2;
        ESTSOO = 1;
    }
#endif
    sprintf(tmp, "***** %s %s %s\n", s, tmp1, s1);
    if (sysprint != NULL)
        fputs(tmp, sysprint);
    fputs(tmp, systerm);
}

void pchosx(char *s, char *sid, int lsid, char *s1)
{
    char tmp[255];
    char tmp1[255];
    int i;
    oshibka();
    for (i = 0; i < lsid; i++)
        tmp1[i] = *(sid + i);
    tmp1[lsid] = '\0';
#ifdef turbo
    if (ESTSOO == 0)
    {
        sprintf(ERRO, "  %s %s %s", s, tmp1, s1);
        IERRO = ISTEK - 2;
        ESTSOO = 1;
    }
#endif
    sprintf(tmp, "***** %s %s %s\n", s, tmp1, s1);
    if (sysprint != NULL)
        fputs(tmp, sysprint);
    fputs(tmp, systerm);
}

void pchosa(char *s, char c)
{
    char tmp[255];
    oshibka();
#ifdef turbo
    if (ESTSOO == 0)
    {
        sprintf(ERRO, "  %s %c", s, c);
        IERRO = ISTEK - 2;
        ESTSOO = 1;
    }
#endif
    sprintf(tmp, "***** %s %c\n", s, c);
    if (sysprint != NULL)
        fputs(tmp, sysprint);
    fputs(tmp, systerm);
}

void pchose(char *s, char *t, int lt)
{
    char tmp[255];
    char tmp1[255];
    int i;
    oshibka();
    for (i = 0; i < lt; i++)
        tmp1[i] = *(t + i);
    tmp1[lt] = '\0';
#ifdef turbo
    if (ESTSOO == 0)
    {
        sprintf(ERRO, "  %s %s", s, tmp1);
        IERRO = ISTEK - 2;
        ESTSOO = 1;
    }
#endif
    sprintf(tmp, "***** %s %s\n", s, tmp1);
    if (sysprint != NULL)
        fputs(tmp, sysprint);
    fputs(tmp, systerm);
}
/*--------  end  of  file  CERR.C  ---------*/
