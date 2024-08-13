//----------- file -- MAINRF.C ------------------- 
//          Standard REFAL-initiator               
//       Last modification : 11.07.2024            
//------------------------------------------------ 
#include <stdint.h>
#include "rfintf.h"

// BLF GO -> go  
extern void go();

int main()
{
    rfexec(go);
}

//---------  end of file MAINRF.C  --------- 
