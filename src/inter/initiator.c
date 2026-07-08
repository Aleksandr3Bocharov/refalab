// Copyright (c) 2026 Aleksandr Bocharov
// SPDX-License-Identifier: MIT
// 2026-06-05
// https://github.com/Aleksandr3Bocharov/refalab

//----------  file initiator.c  ----------
//          Standard REFAL-initiator
//----------------------------------------

#include <stdio.h>
#include <stdint.h>
#include "interface.h"

// GO -> refalab_go
extern uint8_t refalab_go;

int main(int argc, char *argv[])
{
    refal_get_args(argc, argv);
    refal_execute(&refalab_go);
}

//----------  end of file initiator.c  ----------
