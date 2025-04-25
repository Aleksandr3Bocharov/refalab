// Copyright 2025 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2025-04-25
// https://github.com/Aleksandr3Bocharov/RefalAB

//-------------- file -- XRAND.C ------------
//        MO: random,
//            random_number
//-------------------------------------------

//#include <stdio.h>
#include <stdint.h>
#include "refalab.h"
#include "rfintf.h"

static void random_(void)
{
    return;
}
char random_0[] = {Z6 'R', 'A', 'N', 'D', 'O', 'M', (char)6};
G_L_B uint8_t refalab_random = '\122';
void (*random_1)(void) = random_;

static void random_number_(void)
{
    return;
}
char random_number_0[] = {Z5 'R', 'A', 'N', 'D', 'O', 'M', '_', 'N', 'U', 'M', 'B', 'E', 'R', (char)13};
G_L_B uint8_t refalab_random_number = '\122';
void (*random_number_1)(void) = random_number_;

//--------------- end of file  XRAND.C -------------