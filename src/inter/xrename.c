// Copyright 2025 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2025-03-20
// https://github.com/Aleksandr3Bocharov/RefalAB

//---------- file -- XRENAME.C ------------
//           MO: file delete/rename
//-----------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "refalab.h"
#include "rfintf.h"

static void unlink_(void)
{
    const T_LINKCB *p = refal.preva->next;
    char namf[255];
    size_t i;
    for (i = 0; p != refal.nexta; i++)
    {
        if (p->tag != TAGO || i == 254)
        {
            printf("\nunlink: format error");
            refal.upshot = 2;
            return;
        }
        namf[i] = p->info.infoc;
        p = p->next;
    }
    namf[i] = '\0';
    if (unlink(namf) == -1)
        rfabe("unlink: error");
    return;
}
char unlink_0[] = {Z6 'U', 'N', 'L', 'I', 'N', 'K', (char)6};
G_L_B uint8_t refalab_unlink = '\122';
void (*unlink_1)(void) = unlink_;

static void rename_(void)
{
    const T_LINKCB *p = refal.preva->next;
    char namf[255];
    size_t i;
    bool heot = false;
    do
    {
        for (i = 0; p->tag != TAGO || p->info.infoc != '*'; i++)
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
        namf[i] = '\0';
        p = p->next;
        char namt[255]; // from => to
        for (i = 0; p != refal.nexta; i++)
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
        namt[i] = '\0';
        if (rename(namf, namt) == -1)
            rfabe("rename: error");
        return;
    } while (false);
    printf("\nrename: format error");
    refal.upshot = 2;
    return;
}
char rename_0[] = {Z6 'R', 'E', 'N', 'A', 'M', 'E', (char)6};
G_L_B uint8_t refalab_rename = '\122';
void (*rename_1)(void) = rename_;

//-------- end of file  XRENAME.C -------
