@echo off

rem Copyright 2025 Aleksandr Bocharov
rem Distributed under the Boost Software License, Version 1.0.
rem See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
rem 2025-10-05
rem https://github.com/Aleksandr3Bocharov/RefalAB

rem *******************************************************************
rem RefalABdclg.bat
rem compile from refal to GNU-assembler + debugger link + go
rem arg: %1 - name of refal programm (without ext ref)
rem result: running of %1.exe debugger 
rem *******************************************************************

bin\RefalAB %1 > %1.log
as -o %1.o %1.s >> %1.log
clang -o %1.exe %1.o lib\obj\rfdbg.o -Llib -lRefalAB >> %1.log
%1.exe
