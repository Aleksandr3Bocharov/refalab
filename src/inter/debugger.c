// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-06-05
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file debugger.c  -----------
//          Standard RefalAB debugger
//----------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "refalab.h"
#include "debugger_interface.h"
#include "rfintf.h"

static T_STATUS_TABLE status_table;
extern uint8_t refalab_go;

int main(int argc, char *argv[])
{
    refal_get_args(argc, argv);
    refal_initiate();
    do
    {
        if (!more_free_memory())
            break;
        if (!create_status_table(&status_table))
            break;
        if (!insert_view_k_function_dot(&status_table, &refalab_go))
            break;
        refal_debugger(&status_table); // there is exit
    } while (false);
    printf("\nRefalAB debugger: no memory for initialization\n");
    fclose(stdin);
    refal_terminate_memory();
    exit(1);
}

//----------  end of file debugger.c ----------
