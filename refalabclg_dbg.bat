@echo off

rem Copyright (c) 2026 Aleksandr Bocharov
rem SPDX-License-Identifier: MIT
rem 2026-06-09
rem https://github.com/Aleksandr3Bocharov/refalab

rem *******************************************************************
rem refalabclg_dbg.bat
rem compile from refal to object file + link + go
rem arg: %1 - name of refal programm (without ext ref)
rem result: running of %1_dbg.exe in debugging mode
rem *******************************************************************

bin\refalabc %1 -l,"-o %1_dbg.o -O0 -g -Wno-override-module" > %1_dbg.log
clang -o %1_dbg.exe %1_dbg.o lib\obj\refalab_initiator_dbg.o -Llib -lrefalab_dbg -pthread -static >> %1_dbg.log
%1_dbg.exe
