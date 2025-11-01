// Copyright 2025 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2025-11-01
// https://github.com/Aleksandr3Bocharov/refalab

//-------------- file -- XDELF.C ------------
//               MO: delf
//-------------------------------------------

#include <stddef.h>
#include <stdbool.h>
#include "refalab.h"
#include "rfintf.h"

static void delf_(void)
{
    if (refal.preva->next != refal.nexta)
    {
        refal.upshot = 2;
        return;
    } // FAIL
    const T_LINKCB *dot = refal.nexta;
    const T_LINKCB *dot1 = refal.nextr->info.codep;
    const T_LINKCB *sk;
    T_LINKCB *nd;
    while (true)
    {
        sk = dot->info.codep;
        dot = sk->info.codep;
        if (dot == NULL)
            rfabe("delf: sign '#' missing");
        nd = dot->next;
        if (nd->info.infoc != '#')
            continue;
        break;
    }
    while (dot1 != dot)
    {
        sk = dot1->info.codep;
        nd = sk->info.codep;
        rfdel(sk->prev, dot1->next);
        dot1 = nd;
    }
    sk = dot1->info.codep;
    nd = sk->info.codep;
    dot1 = dot1->next;
    rfdel(sk->prev, dot1->next);
    refal.nextr->info.codep = nd;
    return;
}
char delf_0[] = {Z4 'D', 'E', 'L', 'F', (char)4};
G_L_B uint8_t refalab_delf = '\122';
void (*delf_1)(void) = delf_;

//-------------------- end of file  XDELF.C ---------------
