// Copyright 2024 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2024-09-27
// https://github.com/Aleksandr3Bocharov/RefalAB

//------------ file -- RFDBG.C -------------
//          Standard REFAL-Debugger
//------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "refal.def"
#include "d.h"
#include "rfintf.h"

static T_ST s_st;
extern void go(void);

int main(void)
{
    rfinit();
    do
    {
        if (!lincrm())
            break;
        if (!lcre(&s_st))
            break;
        if (!linskd(&s_st, go))
            break;
        rfdbg(&s_st); // there is exit
    } while (false);
    printf("\nREFAL-debugger: no memory for initialization");
    fclose(stdin);
    rftermm();
    exit(1);
}

//---------  end of file RFDBG.C -----------
