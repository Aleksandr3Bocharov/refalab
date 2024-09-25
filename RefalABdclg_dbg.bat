@echo off
rem *******************************************************************
rem RefalABdclg_dbg.bat
rem	compile from refal to GNU-assembler + debugger link + go
rem 	arg: %1 - name of refal programm (without ext ref)
rem	result: running of %1.exe in debugging mode 
rem *******************************************************************

bin\RefalAB %1 > %1.log
as -o %1.o %1.asm >> %1.log
gcc -o %1.exe lib\rfdbg.o -Wl,-L.\lib %1.o -Wl,-lRefalAB_dbg >> %1.log
%1.exe
