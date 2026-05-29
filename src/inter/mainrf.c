// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-05-29
// https://github.com/Aleksandr3Bocharov/refalab

//----------- file -- MAINRF.C -------------------
//          Standard REFAL-initiator
//------------------------------------------------

#include <stdio.h>
#include <stdint.h>
#include "rfintf.h"

// GO -> go
extern uint8_t refalab_go;

int main(int argc, char *argv[])
{
    refal_get_args(argc, argv);
    refal_execute(&refalab_go);
}

//---------  end of file MAINRF.C  ---------
