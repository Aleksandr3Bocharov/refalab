// Copyright 2025 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2025-04-05
// https://github.com/Aleksandr3Bocharov/RefalAB

//---------- file -- XRENAME.C ------------
//           MO: file delete/rename/exist
//-----------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "refalab.h"
#include "rfintf.h"

static void remove_file_(void)
{
    const T_LINKCB *p = refal.preva->next;
    char namf[MAX_FILE_NAME + 1];
    size_t i;
    for (i = 0; p != refal.nexta; i++)
    {
        if (p->tag != TAGO || i == MAX_FILE_NAME)
        {
            refal.upshot = 2;
            return;
        }
        namf[i] = p->info.infoc;
        p = p->next;
    }
    namf[i] = '\0';
    if (unlink(namf) == -1)
        rfabe("remove_file: error");
    return;
}
char remove_file_0[] = {Z3 'R', 'E', 'M', 'O', 'V', 'E', '_', 'F', 'I', 'L', 'E', (char)11};
G_L_B uint8_t refalab_remove_file = '\122';
void (*remove_file_1)(void) = remove_file_;

static void rename_(void)
{
    const T_LINKCB *p = refal.preva->next;
    char namf[MAX_FILE_NAME + 1];
    size_t i;
    bool heot = false;
    do
    {
        for (i = 0; p->tag != TAGO || p->info.infoc != '*'; i++)
        {
            if (p->tag != TAGO || i == MAX_FILE_NAME)
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
        char namt[MAX_FILE_NAME + 1]; // from => to
        for (i = 0; p != refal.nexta; i++)
        {
            if (p->tag != TAGO || i == MAX_FILE_NAME)
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
    refal.upshot = 2;
    return;
}
char rename_0[] = {Z6 'R', 'E', 'N', 'A', 'M', 'E', (char)6};
G_L_B uint8_t refalab_rename = '\122';
void (*rename_1)(void) = rename_;

static void exist_file_(void)
{
    const T_LINKCB *p = refal.preva->next;
    char namf[MAX_FILE_NAME + 1];
    size_t i;
    for (i = 0; p != refal.nexta; i++)
    {
        if (p->tag != TAGO || i == MAX_FILE_NAME)
        {
            refal.upshot = 2;
            return;
        }
        namf[i] = p->info.infoc;
        p = p->next;
    }
    namf[i] = '\0';
    
    return;
}
char exist_file_0[] = {Z2 'E', 'X', 'I', 'S', 'T', '_', 'F', 'I', 'L', 'E', (char)10};
G_L_B uint8_t refalab_exist_file = '\122';
void (*exist_file_1)(void) = exist_file_;

//-------- end of file  XRENAME.C -------
