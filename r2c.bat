@echo off
rem *******************************************************************
rem r2c.bat
rem	compile from refal to GNU-assembler 
rem 	arg: %1 - name of refal programm (without extension ref)
rem	result: %1.asm in GNU assembler language 
rem
rem Copyright (C) Refal-2 Team
rem Author: Leonid Belous (aka BLF)
rem E-mail: belous@ilt.kharkov.ua
rem Date: 28.05.2005
rem *******************************************************************

%~dp0\bin\refal2 %1.ref > %1.log
