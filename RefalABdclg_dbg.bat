@echo off

rem Copyright 2025 Aleksandr Bocharov
rem Distributed under the Boost Software License, Version 1.0.
rem See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
rem 2025-10-14
rem https://github.com/Aleksandr3Bocharov/RefalAB

rem *******************************************************************
rem RefalABdclg_dbg.bat
rem compile from refal to object file + debugger link + go
rem arg: %1 - name of refal programm (without ext ref)
rem result: running of %1.exe debugger in debugging mode 
rem *******************************************************************

bin\RefalABc %1 -a,"-o %1.o" > %1.log
clang -o %1.exe %1.o lib\obj\rfdbg.o -Llib -lRefalAB_dbg >> %1.log
%1.exe
