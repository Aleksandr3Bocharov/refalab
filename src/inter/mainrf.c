//----------- file -- MAINRF.C ------------------- 
//          Standard REFAL-initiator               
//       Last modification : 11.07.2024            
//------------------------------------------------ 
#include <stdint.h>
#include "rfintf.h"

// BLF GO -> go  
extern char go();

int main()
{
    rfexec((uint8_t *)go);
}

//---------  end of file MAINRF.C  --------- 
