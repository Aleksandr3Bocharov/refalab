/*----------- file -- MAINRF.C -------------------*/
/*          Standard REFAL-initiator              */
/*       Last modification : 11.07.2024           */
/*------------------------------------------------*/
#include "rfintf.h"

/* BLF GO -> go */
extern char go();

int main()
{
    rfexec((unsigned char *)go);
}

/*---------  end of file MAINRF.C  ---------*/
