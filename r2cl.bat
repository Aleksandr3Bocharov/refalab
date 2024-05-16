@echo off
rem *******************************************************************
rem r2cl.bat
rem	compile from refal to GNU-assembler + assembling. 
rem 	arg: %1 - name of refal programm (without ext ref)
rem	result: %1.o add to user lib (libr2user.a) 
rem
rem Copyright (C) Refal-2 Team
rem Author: Leonid Belous (aka BLF)
rem E-mail: belous@ilt.kharkov.ua
rem Date: 28.05.2005
rem *******************************************************************

%~dp0\bin\refal2 %1.ref > %1.log
F:\Programming\mingw32\bin\as -o %1.o %1.asm >> %1.log
F:\Programming\mingw32\bin\ar -r %~dp0\lib\libr2user.a %1.o