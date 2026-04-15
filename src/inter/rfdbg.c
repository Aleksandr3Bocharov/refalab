// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-04-14
// https://github.com/Aleksandr3Bocharov/refalab

//------------ file -- RFDBG.C -------------
//          Standard RefalAB debugger
//------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "refalab.h"
#include "d.h"
#include "rfintf.h"

static T_STATUS_TABLE s_st;
extern uint8_t refalab_go;

int main(int argc, char *argv[])
{
    refal_get_args(argc, argv);
    refal_initiate();
    do
    {
        if (!more_free_memory())
            break;
        if (!lcre(&s_st))
            break;
        if (!insert_view_k_function_d(&s_st, &refalab_go))
            break;
        rfdbg(&s_st); // there is exit
    } while (false);
    printf("\nRefalAB debugger: no memory for initialization\n");
    fclose(stdin);
    rftermm();
    exit(1);
}

//---------  end of file RFDBG.C -----------
