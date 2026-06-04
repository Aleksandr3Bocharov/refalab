@echo off

rem Copyright (c) 2026 Aleksandr Bocharov
rem SPDX-License-Identifier: MIT
rem 2026-06-05
rem https://github.com/Aleksandr3Bocharov/refalab

rem *******************************************************************
rem refalabdclg.bat
rem compile from refal to object file + debugger link + go
rem arg: %1 - name of refal programm (without ext ref)
rem result: running of %1.exe debugger 
rem *******************************************************************

bin\refalabc %1 -a,"-o %1.o" > %1.log
clang -o %1.exe %1.o lib\obj\refalab_debugger.o -Llib -lrefalab -pthread -static >> %1.log
%1.exe
