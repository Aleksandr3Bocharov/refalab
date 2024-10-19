// Copyright 2024 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2024-10-19
// https://github.com/Aleksandr3Bocharov/RefalAB

//---------- file -- XRENAME.C ------------
//           MO: file delete/rename
//-----------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "refal.def"
#include "rfintf.h"

static void unlnk_(void)
{
    const T_LINKCB *p = refal.preva->next;
    size_t i;
    char namf[40];
    for (i = 0; p != refal.nexta; i++)
        if (p->tag != TAGO || i >= 40)
        {
            printf("\nUnlnk: format error");
            refal.upshot = 2;
            return;
        }
        else
        {
            namf[i] = p->info.infoc;
            p = p->next;
        }
    namf[i] = '\0';
    if (unlink(namf) == -1)
    {
        printf("\nUnlnk: error");
        exit(1);
    }
    return;
}
char unlnk_0[] = {Z5 'U', 'N', 'L', 'N', 'K', '\005'};
G_L_B char unlnk = '\122';
void (*unlnk_1)(void) = unlnk_;

static void renam_(void)
{
    const T_LINKCB *p = refal.preva->next;
    size_t i;
    char namf[40];
    bool heot = false;
    do
    {
        for (i = 0; p->tag != TAGO || p->info.infoc != ' '; i++)
        {
            if (p->tag != TAGO || i >= 40)
            {
                heot = true;
                break;
            }
            namf[i] = p->info.infoc;
            p = p->next;
        }
        if (heot)
            break;
        namf[i] = '\0';
        p = p->next;
        char namt[40]; // from => to
        for (i = 0; p != refal.nexta; i++)
        {
            if (p->tag != TAGO || i >= 40)
            {
                heot = true;
                break;
            }
            namt[i] = p->info.infoc;
            p = p->next;
        }
        if (heot)
            break;
        namt[i] = '\0';
        if (rename(namf, namt) == -1)
        {
            printf("\nRenam: error");
            exit(1);
        }
        return;
    } while (false);
    printf("\nRenam: format error");
    refal.upshot = 2;
    return;
}
char renam_0[] = {Z5 'R', 'E', 'N', 'A', 'M', '\005'};
G_L_B char renam = '\122';
void (*renam_1)(void) = renam_;

//-------- end of file  XRENAME.C -------
