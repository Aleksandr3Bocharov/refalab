// Copyright 2025 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2025-01-03
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
    char namf[255];
    for (size_t i = 0; i < 255; i++)
        namf[i] = '\0';
    for (size_t i = 0; p != refal.nexta; i++)
    {
        if (p->tag != TAGO || i == 254)
        {
            printf("\nunlnk: format error");
            refal.upshot = 2;
            return;
        }
        namf[i] = p->info.infoc;
        p = p->next;
    }
    if (unlink(namf) == -1)
        rfabe("unlnk: error");
    return;
}
char unlnk_0[] = {Z5 'U', 'N', 'L', 'N', 'K', '\005'};
G_L_B char unlnk = '\122';
void (*unlnk_1)(void) = unlnk_;

static void renam_(void)
{
    const T_LINKCB *p = refal.preva->next;
    char namf[255];
    for (size_t i = 0; i < 255; i++)
        namf[i] = '\0';
    bool heot = false;
    do
    {
        for (size_t i = 0; p->tag != TAGO || p->info.infoc != '*'; i++)
        {
            if (p->tag != TAGO || i == 254)
            {
                heot = true;
                break;
            }
            namf[i] = p->info.infoc;
            p = p->next;
        }
        if (heot)
            break;
        p = p->next;
        char namt[255]; // from => to
        for (size_t i = 0; i < 255; i++)
            namt[i] = '\0';
        for (size_t i = 0; p != refal.nexta; i++)
        {
            if (p->tag != TAGO || i == 254)
            {
                heot = true;
                break;
            }
            namt[i] = p->info.infoc;
            p = p->next;
        }
        if (heot)
            break;
        if (rename(namf, namt) == -1)
            rfabe("renam: error");
        return;
    } while (false);
    printf("\nrenam: format error");
    refal.upshot = 2;
    return;
}
char renam_0[] = {Z5 'R', 'E', 'N', 'A', 'M', '\005'};
G_L_B char renam = '\122';
void (*renam_1)(void) = renam_;

//-------- end of file  XRENAME.C -------
