// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-11-17
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
#include "interface.h"

// Status table of the debugged program.
static T_STATUS_TABLE status_table;
extern uint8_t refalab_go;

// The standard initiator with the debugger: it creates the status table with the view field
// <Go> and gives control to the debugger, which never returns (the program ends inside it).
// We get further only when the initialization fails for lack of memory.
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
        // Normal end: the debugger exits the program.
        refal_debugger(&status_table); // there is exit
    } while (false);
    printf("\nRefalAB debugger: no memory for initialization\n");
    fclose(stdin);
    refal_terminate_memory();
    exit(1);
}

//----------  end of file debugger.c ----------
