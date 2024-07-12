//------------ file -- RFDBG.C ------------- 
//          Standard REFAL-Debugger          
//        Last modification : 11.07.24       
//------------------------------------------ 
#include <stdio.h>
#include <stdlib.h>
#include "refal.def"
#include "d.h"
#include "rfintf.h"

static st s_st;
extern char go();

int main()
{
    rfinit();
    if (!lincrm())
        goto LACK;
    if (!lcre(&s_st))
        goto LACK;
    if (!linskd(&s_st, (unsigned char *)go))
        goto LACK;
    rfdbg(&s_st); // there is exit  
LACK:
    printf("\nREFAL-debugger: no memory for initialization");
    fclose(stdin);
    rftermm();
    exit(1);
}
//---------  end of file RFDBG.C ----------- 
