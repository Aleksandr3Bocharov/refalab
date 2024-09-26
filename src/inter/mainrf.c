
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
