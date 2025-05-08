// Copyright 2025 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2025-05-08
// https://github.com/Aleksandr3Bocharov/RefalAB

//----------- file -- MAINRF.C -------------------
//          Standard REFAL-initiator
//------------------------------------------------

#include <stdio.h>
#include <stdint.h>
#include "rfintf.h"
#include "xsys.h"

// GO -> go
extern uint8_t refalab_go;

int main(int argc, char *argv[])
{
    rfgetargs(argc, argv);
    rfexec(&refalab_go);
}

//---------  end of file MAINRF.C  ---------
