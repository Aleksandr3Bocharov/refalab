@echo off

rem Copyright (c) 2026 Aleksandr Bocharov
rem SPDX-License-Identifier: MIT
rem 2026-06-09
rem https://github.com/Aleksandr3Bocharov/refalab

rem *******************************************************************
rem refalabco_dbg.bat
rem compile from refal to object file in debugging mode
rem arg: %1 - name of refal programm (without extension ref)
rem result: %1_dbg.o
rem *******************************************************************

bin\refalabc %1 -l,"-o %1.o -O0 -g -Wno-override-module" > %1_dbg.log
