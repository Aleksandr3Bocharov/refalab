@echo off

rem Copyright 2024 Aleksandr Bocharov
rem Distributed under the Boost Software License, Version 1.0.
rem See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
rem 2024-10-22
rem https://github.com/Aleksandr3Bocharov/RefalAB

rem *******************************************************************
rem RefalABdclg_dbg.bat
rem	compile from refal to GNU-assembler + debugger link + go
rem 	arg: %1 - name of refal programm (without ext ref)
rem	result: running of %1.exe in debugging mode 
rem *******************************************************************

bin\RefalAB %1 > %1.log
as -o %1.o %1.asm >> %1.log
clang -o %1.exe lib\rfdbg.o -Llib %1.o -lRefalAB_dbg >> %1.log
%1.exe
