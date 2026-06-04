@echo off

rem Copyright (c) 2026 Aleksandr Bocharov
rem SPDX-License-Identifier: MIT
rem 2026-06-05
rem https://github.com/Aleksandr3Bocharov/refalab

rem *******************************************************************
rem refalabco.bat
rem compile from refal to object file 
rem arg: %1 - name of refal programm (without extension ref)
rem result: %1.o
rem *******************************************************************

bin\refalabc %1 -a,"-o %1.o" > %1.log
