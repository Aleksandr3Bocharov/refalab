/*-------------  file  --  CERR.C  ---------------*/
/*               Print error file                 */
/*        Last edition date : 25.06.24            */
/*------------------------------------------------*/
#include <stdio.h>
#include "refal.def"
#include "cerr.h"
#include "refal.h"

void pchosh(char *s)
{
    oshibka();
    char tmp[255];
    sprintf(tmp, "***** %s\n", s);
    if (sysprint != NULL)
        fputs(tmp, sysprint);
    fputs(tmp, systerm);
}

static void pchosi(char *s, char *t)
{
    oshibka();
    char tmp[255];
    sprintf(tmp, "***** %s %s\n", s, t);
    if (sysprint != NULL)
        fputs(tmp, sysprint);
    fputs(tmp, systerm);
}

void pchosj(char *s, char *sid, unsigned int lsid, char *s1)
{
    oshibka();
    char tmp1[255];
    for (unsigned int i = 0; i < lsid; i++)
        tmp1[i] = *(sid + i);
    tmp1[lsid] = '\0';
    char tmp[255];
    sprintf(tmp, "***** %s %s %s\n", s, tmp1, s1);
    if (sysprint != NULL)
        fputs(tmp, sysprint);
    fputs(tmp, systerm);
}

void pchosx(char *s, char *sid, unsigned int lsid, char *s1)
{
    oshibka();
    char tmp1[255];
    for (unsigned int i = 0; i < lsid; i++)
        tmp1[i] = *(sid + i);
    tmp1[lsid] = '\0';
    char tmp[255];
    sprintf(tmp, "***** %s %s %s\n", s, tmp1, s1);
    if (sysprint != NULL)
        fputs(tmp, sysprint);
    fputs(tmp, systerm);
}

void pchosa(char *s, char c)
{
    oshibka();
    char tmp[255];
    sprintf(tmp, "***** %s %c\n", s, c);
    if (sysprint != NULL)
        fputs(tmp, sysprint);
    fputs(tmp, systerm);
}

static void pchose(char *s, char *t, unsigned int lt)
{
    oshibka();
    char tmp1[255];
    for (unsigned int i = 0; i < lt; i++)
        tmp1[i] = *(t + i);
    tmp1[lt] = '\0';
    char tmp[255];
    sprintf(tmp, "***** %s %s\n", s, tmp1);
    if (sysprint != NULL)
        fputs(tmp, sysprint);
    fputs(tmp, systerm);
}
/*--------  end  of  file  CERR.C  ---------*/
