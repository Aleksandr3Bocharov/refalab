/*----------- file -- MAINRF.C -------------------*/
/*          Standard REFAL-initiator              */
/*       Last modification : 19.06.2024           */
/*------------------------------------------------*/
#include "rfintf.h"

/* BLF GO -> go */
extern char *go;

int main()
{
  rfexec(go);
}

/*---------  end of file MAINRF.C  ---------*/
