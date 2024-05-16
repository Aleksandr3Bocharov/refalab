@echo off
rem *******************************************************************
rem r2clg.bat
rem	compile from refal to GNU-assembler + link + go
rem 	arg: %1 - name of refal programm (without ext ref)
rem	result: running of %1.exe 
rem
rem Copyright (C) Refal-2 Team
rem Author: Leonid Belous (aka BLF)
rem E-mail: belous@ilt.kharkov.ua
rem Date: 21.05.2005
rem *******************************************************************

%~dp0\bin\refal2 %1.ref > %1.log
F:\Programming\mingw32\bin\as -o %1.o %1.asm >> %1.log
F:\Programming\mingw32\bin\gcc -m32 -o %1.exe %~dp0\lib\r2objects\mainrf.o %1.o -Wl,-L%~dp0\lib -Wl,-lrefal2,-lr2user >> %1.log
%1.exe
