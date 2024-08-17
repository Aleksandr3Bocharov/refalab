@echo off
rem *******************************************************************
rem r2dclg_dbg.bat
rem	compile from refal to GNU-assembler + debugger link + go
rem 	arg: %1 - name of refal programm (without ext ref)
rem	result: running of %1.exe in debugging mode 
rem
rem Copyright(C) Aleksandr Bocharov
rem Author : Aleksandr Bocharov
rem Date : 05.08.2024
rem *******************************************************************

bin\refal2 %1 > %1.log
as -o %1.o %1.asm >> %1.log
gcc -o %1.exe lib\rfdbg.o -Wl,-L.\lib %1.o -Wl,-lr2_dbg >> %1.log
%1.exe
