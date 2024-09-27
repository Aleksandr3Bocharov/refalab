// Copyright 2024 Aleksandr Bocharov
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
// 2024-09-27
// https://github.com/Aleksandr3Bocharov/RefalAB

//----------- file -- MAINRF.C -------------------
//          Standard REFAL-initiator
//------------------------------------------------

#include <stdint.h>
#include "rfintf.h"

// BLF GO -> go
extern void go(void);

int main(void)
{
    rfexec(go);
}

//---------  end of file MAINRF.C  ---------
