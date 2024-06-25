/*-------------  file  --  CERR.C  ---------------*/
/*               Print error file                 */
/*        Last edition date : 25.06.24            */
/*------------------------------------------------*/
#include <stdio.h>
#include "refal.def"
#include "cerr.h"
#include "refal.h"

extern FILE *sysprint, *systerm;

extern void pchosh(char *s)
{
    char tmp[255];
    oshibka();
    sprintf(tmp, "***** %s\n", s);
    if (sysprint != NULL)
        fputs(tmp, sysprint);
    fputs(tmp, systerm);
}

static void pchosi(char *s, char *t)
{
    char tmp[255];
    oshibka();
    sprintf(tmp, "***** %s %s\n", s, t);
    if (sysprint != NULL)
        fputs(tmp, sysprint);
    fputs(tmp, systerm);
}

extern void pchosj(char *s, char *sid, int lsid, char *s1)
{
    char tmp[255];
    char tmp1[255];
    int i;
    oshibka();
    for (i = 0; i < lsid; i++)
        tmp1[i] = *(sid + i);
    tmp1[lsid] = '\0';
    sprintf(tmp, "***** %s %s %s\n", s, tmp1, s1);
    if (sysprint != NULL)
        fputs(tmp, sysprint);
    fputs(tmp, systerm);
}

extern void pchosx(char *s, char *sid, int lsid, char *s1)
{
    char tmp[255];
    char tmp1[255];
    int i;
    oshibka();
    for (i = 0; i < lsid; i++)
        tmp1[i] = *(sid + i);
    tmp1[lsid] = '\0';
    sprintf(tmp, "***** %s %s %s\n", s, tmp1, s1);
    if (sysprint != NULL)
        fputs(tmp, sysprint);
    fputs(tmp, systerm);
}

extern void pchosa(char *s, char c)
{
    char tmp[255];
    oshibka();
    sprintf(tmp, "***** %s %c\n", s, c);
    if (sysprint != NULL)
        fputs(tmp, sysprint);
    fputs(tmp, systerm);
}

static void pchose(char *s, char *t, int lt)
{
    char tmp[255];
    char tmp1[255];
    int i;
    oshibka();
    for (i = 0; i < lt; i++)
        tmp1[i] = *(t + i);
    tmp1[lt] = '\0';
    sprintf(tmp, "***** %s %s\n", s, tmp1);
    if (sysprint != NULL)
        fputs(tmp, sysprint);
    fputs(tmp, systerm);
}
/*--------  end  of  file  CERR.C  ---------*/
