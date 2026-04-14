// Copyright 2026 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2026-04-14
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
    rfexec(&refalab_go);
}

//---------  end of file MAINRF.C  ---------
