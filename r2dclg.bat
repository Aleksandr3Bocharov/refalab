@echo off
rem *******************************************************************
rem r2dclg.bat
rem	compile from refal to GNU-assembler + debugger link + go
rem 	arg: %1 - name of refal programm (without ext ref)
rem	result: running of %1.exe in debugging mode 
rem
rem Copyright (C) Refal-2 Team
rem Author: Leonid Belous (aka BLF)
rem E-mail: belous@ilt.kharkov.ua
rem Date: 28.05.2005
rem *******************************************************************

bin\refal2 %1 > %1.log
as -o %1.o %1.asm >> %1.log
gcc -o %1.exe lib\r2objects\rfdbg.o -Wl,-L./lib %1.o -Wl,-lrefal2 >> %1.log
%1.exe
